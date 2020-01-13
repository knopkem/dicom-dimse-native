/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dicomStreamCodec.cpp
    \brief Implementation of the class dicomStreamCodec.

*/

#include <list>
#include <vector>
#include <string.h>
#include "exceptionImpl.h"
#include "streamReaderImpl.h"
#include "streamWriterImpl.h"
#include "memoryImpl.h"
#include "dicomStreamCodecImpl.h"
#include "dataSetImpl.h"
#include "dicomDictImpl.h"
#include "bufferImpl.h"
#include "nullStreamImpl.h"
#include "../include/imebra/exceptions.h"
#include "../include/imebra/definitions.h"

namespace imebra
{

namespace implementation
{

namespace codecs
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// dicomStreamCodec
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write a Dicom stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomStreamCodec::writeStream(std::shared_ptr<streamWriter> pStream, std::shared_ptr<dataSet> pDataSet) const
{
    IMEBRA_FUNCTION_START();

    // Retrieve the transfer syntax
    ///////////////////////////////////////////////////////////
    std::string transferSyntax = pDataSet->getString(0x0002, 0, 0x0010, 0, 0, "1.2.840.10008.1.2");

    // Adjust the flags
    ///////////////////////////////////////////////////////////
    bool bExplicitDataType = (transferSyntax != "1.2.840.10008.1.2");        // Implicit VR little endian

    // Explicit VR big endian
    ///////////////////////////////////////////////////////////
    streamController::tByteOrdering endianType = (transferSyntax == "1.2.840.10008.1.2.2") ? streamController::highByteEndian : streamController::lowByteEndian;

    // Write the dicom header
    ///////////////////////////////////////////////////////////
    std::uint8_t zeroBuffer[128];
    ::memset(zeroBuffer, 0L, 128L);
    pStream->write(zeroBuffer, 128);

    // Write the DICM signature
    ///////////////////////////////////////////////////////////
    pStream->write((std::uint8_t*)"DICM", 4);

    // Build the stream
    ///////////////////////////////////////////////////////////
    buildStream(pStream, pDataSet, bExplicitDataType, endianType, streamType_t::mediaStorage);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Build a dicom stream, without header and DICM signature
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomStreamCodec::buildStream(std::shared_ptr<streamWriter> pStream, std::shared_ptr<const dataSet> pDataSet, bool bExplicitDataType, streamController::tByteOrdering endianType, streamType_t streamType)
{
    IMEBRA_FUNCTION_START();

    dataSet::tGroupsIds groups = pDataSet->getGroups();

    for(dataSet::tGroupsIds::const_iterator scanGroups(groups.begin()), endGroups(groups.end()); scanGroups != endGroups; ++scanGroups)
    {
        size_t numGroups = pDataSet->getGroupsNumber(*scanGroups);
        for(size_t scanGroupsNumber(0); scanGroupsNumber != numGroups; ++scanGroupsNumber)
        {
            const dataSet::tTags& tags(pDataSet->getGroupTags(*scanGroups, scanGroupsNumber));

            if(*scanGroups == 0x0002)
            {
                // When writing a media storage file, the tag 0002,0001 must be 0,1 (OB)
                ////////////////////////////////////////////////////////////////////////
                if(streamType == streamType_t::mediaStorage)
                {
                    dataSet::tTags temporaryTags(tags);
                    const std::shared_ptr<charsetsList_t> charsets(std::make_shared<charsetsList_t>());
                    std::shared_ptr<data> metaInformationTag(std::make_shared<data>(tagVR_t::OB, charsets));
                    {
                        std::shared_ptr<handlers::writingDataHandler> handler(metaInformationTag->getWritingDataHandler(0));
                        handler->setUnsignedLong(0, 0);
                        handler->setUnsignedLong(1, 1);
                    }
                    temporaryTags[1] = metaInformationTag;

                    // Set the implementation tags
                    std::shared_ptr<data> implementationClassUidTag(std::make_shared<data>(tagVR_t::UI, charsets));
                    {
                        std::shared_ptr<handlers::writingDataHandler> handler(implementationClassUidTag->getWritingDataHandler(0));
                        handler->setString(0, IMEBRA_IMPLEMENTATION_CLASS_UID);
                    }
                    temporaryTags[0x12] = implementationClassUidTag;

                    // Set the implementation name
                    std::shared_ptr<data> implementationNameTag(std::make_shared<data>(tagVR_t::SH, charsets));
                    {
                        std::shared_ptr<handlers::writingDataHandler> handler(implementationNameTag->getWritingDataHandler(0));
                        handler->setString(0, IMEBRA_IMPLEMENTATION_NAME);
                    }
                    temporaryTags[0x13] = implementationNameTag;

                    writeGroup(pStream, temporaryTags, *scanGroups, bExplicitDataType, endianType);
                }
            }
            else
            {
                writeGroup(pStream, tags, *scanGroups, bExplicitDataType, endianType);
            }
        }
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write a single data group
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomStreamCodec::writeGroup(std::shared_ptr<streamWriter> pDestStream, const dataSet::tTags& tags, std::uint16_t groupId, bool bExplicitDataType, streamController::tByteOrdering endianType)
{
    IMEBRA_FUNCTION_START();

    if(groupId == 2)
    {
        bExplicitDataType = true;
        endianType = streamController::lowByteEndian;
    }

    // Group id adjusted for byte endianess
    ///////////////////////////////////////////////////////////
    const std::uint16_t adjustedGroupId = pDestStream->adjustEndian(groupId, endianType);;

    // The group length is mandatory for groups 0 and 2
    ///////////////////////////////////////////////////////////
    if(groupId == 0 || groupId == 2)
    {
        // Calculate the group's length
        ///////////////////////////////////////////////////////////
        std::uint32_t groupLength = getGroupLength(tags, bExplicitDataType);

        // Write the group length VR
        ///////////////////////////////////////////////////////////
        const char lengthDataType[] = "UL";

        // Write the group length ID
        ///////////////////////////////////////////////////////////
        const std::uint16_t tagId = 0;
        pDestStream->write(reinterpret_cast<const std::uint8_t*>(&adjustedGroupId), 2u);
        pDestStream->write(reinterpret_cast<const std::uint8_t*>(&tagId), 2u);

        // Write the length of the group length tag
        ///////////////////////////////////////////////////////////
        if(bExplicitDataType)
        {
            pDestStream->write(reinterpret_cast<const std::uint8_t*>(&lengthDataType), 2u);
            const std::uint16_t tagLengthWord = pDestStream->adjustEndian(std::uint16_t(4u), endianType);;
            pDestStream->write(reinterpret_cast<const std::uint8_t*>(&tagLengthWord), 2u);
        }
        else
        {
            const std::uint32_t tagLengthDword = pDestStream->adjustEndian(std::uint32_t(4u), endianType);
            pDestStream->write(reinterpret_cast<const std::uint8_t*>(&tagLengthDword), 4u);
        }

        // Write the length
        ///////////////////////////////////////////////////////////
        std::uint32_t adjustedGroupLength = pDestStream->adjustEndian(groupLength, endianType);
        pDestStream->write(reinterpret_cast<std::uint8_t*>(&adjustedGroupLength), 4u);
    }

    // Write all the tags
    ///////////////////////////////////////////////////////////
    for(dataSet::tTags::const_iterator scanTags(tags.begin()), endTags(tags.end()); scanTags != endTags; ++scanTags)
    {
        std::uint16_t tagId = scanTags->first;
        if(tagId == 0)
        {
            continue;
        }
        pDestStream->write(reinterpret_cast<const std::uint8_t*>(&adjustedGroupId), 2u);
        writeTag(pDestStream, scanTags->second, tagId, bExplicitDataType, endianType);
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write a single tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomStreamCodec::writeTag(std::shared_ptr<streamWriter> pDestStream, std::shared_ptr<data> pData, std::uint16_t tagId, bool bExplicitDataType, streamController::tByteOrdering endianType)
{
    IMEBRA_FUNCTION_START();

    // Calculate the tag's length
    ///////////////////////////////////////////////////////////
    bool bSequence;
    std::uint32_t tagHeader;
    std::uint32_t tagLength = getTagLength(pData, bExplicitDataType, &tagHeader, &bSequence);

    // Prepare the identifiers for the sequence (adjust the
    //  endian)
    ///////////////////////////////////////////////////////////
    const std::uint16_t sequenceItemGroup = streamController::adjustEndian(std::uint16_t(0xfffe), endianType);
    const std::uint16_t sequenceItemDelimiter = streamController::adjustEndian(std::uint16_t(0xe000), endianType);
    const std::uint16_t sequenceTagEnd = streamController::adjustEndian(std::uint16_t(0xe0dd), endianType);
    const std::uint32_t zeroLength = 0;

    // Check the data type
    ///////////////////////////////////////////////////////////
    tagVR_t dataType = pData->getDataType();

    // Adjust the tag id endianess and write it
    ///////////////////////////////////////////////////////////
    std::uint16_t adjustedTagId = tagId;
    pDestStream->adjustEndian(reinterpret_cast<std::uint8_t*>(&adjustedTagId), 2, endianType);
    pDestStream->write(reinterpret_cast<const std::uint8_t*>(&adjustedTagId), 2);

    // Write the data type if it is explicit
    ///////////////////////////////////////////////////////////
    if(bExplicitDataType)
    {
        std::string dataTypeString(dicomDictionary::getDicomDictionary()->enumDataTypeToString(dataType));
        pDestStream->write(reinterpret_cast<const std::uint8_t*>(dataTypeString.c_str()), 2);

        std::uint16_t tagLengthWord = static_cast<std::uint16_t>(tagLength);

        if(dicomDictionary::getDicomDictionary()->getLongLength(dataType))
        {
            std::uint32_t tagLengthDWord = bSequence ? 0xffffffff : tagLength;
            tagLengthWord = 0;
            pDestStream->adjustEndian((std::uint8_t*)&tagLengthDWord, 4, endianType);
            pDestStream->write((std::uint8_t*)&tagLengthWord, 2);
            pDestStream->write((std::uint8_t*)&tagLengthDWord, 4);
        }
        else
        {
            if(bSequence)
            {
                IMEBRA_THROW(InvalidSequenceItemError, "Sequences cannot be used with dataType " << dataTypeString);
            }
            pDestStream->adjustEndian((std::uint8_t*)&tagLengthWord, 2, endianType);
            pDestStream->write((std::uint8_t*)&tagLengthWord, 2);
        }
    }
    else
    {
        std::uint32_t tagLengthDword = bSequence ? 0xffffffff : tagLength;
        pDestStream->adjustEndian((std::uint8_t*)&tagLengthDword, 4, endianType);
        pDestStream->write((std::uint8_t*)&tagLengthDword, 4);
    }

    // Write all the buffers or datasets
    ///////////////////////////////////////////////////////////
    for(std::uint32_t scanBuffers = 0; ; ++scanBuffers)
    {
        if(pData->bufferExists(scanBuffers))
        {
            std::shared_ptr<buffer> pBuffer = pData->getBuffer(scanBuffers);

            size_t bufferSize = pBuffer->getBufferSizeBytes();
            size_t writeSize(bufferSize);
            if((writeSize & 1u) == 1u)
            {
                ++writeSize;
            }

            // write the sequence item header
            ///////////////////////////////////////////////////////////
            if(bSequence)
            {
                pDestStream->write((std::uint8_t*)&sequenceItemGroup, 2);
                pDestStream->write((std::uint8_t*)&sequenceItemDelimiter, 2);
                std::uint32_t sequenceItemLength = (std::uint32_t)writeSize;
                pDestStream->adjustEndian((std::uint8_t*)&sequenceItemLength, 4, endianType);
                pDestStream->write((std::uint8_t*)&sequenceItemLength, 4);
            }

            if(writeSize == 0)
            {
                continue;
            }

            std::uint32_t wordSize = dicomDictionary::getDicomDictionary()->getWordSize(dataType);
            if(pBuffer->hasExternalStream() && (wordSize < 2u || endianType == pBuffer->getEndianType()))
            {
                // The buffer has a stream which is already with the
                // requested byte endianess
                ///////////////////////////////////////////////////////////
                std::shared_ptr<streamReader> pReader = pData->getStreamReader(scanBuffers);

                size_t writtenSize(0);
                std::uint8_t buffer[32768];

                try
                {
                    while(writtenSize != writeSize)
                    {
                        size_t readSize(std::min(sizeof(buffer), writeSize - writtenSize));
                        size_t readBytes = pReader->readSome(buffer, readSize);
                        if(readBytes == 0)
                        {
                            break;
                        }
                        pDestStream->write(buffer, readBytes);
                        writtenSize += readBytes;
                    }
                }
                catch(const StreamEOFError&)
                {
                    if(writtenSize != writeSize - 1)
                    {
                        IMEBRA_THROW(StreamEOFError, "An external stream containing a tag value cannot be read");
                    }
                    if(writeSize != bufferSize)
                    {
                        std::uint8_t paddingByte = pData->getPaddingByte();
                        pDestStream->write(&paddingByte, 1);
                    }
                }
            }
            else
            {
                // We need to get the raw memory (the stream is not in the
                // requested byte endianess or we have the raw memory)
                ///////////////////////////////////////////////////////////
                std::shared_ptr<handlers::readingDataHandlerRaw> pDataHandlerRaw = pData->getReadingDataHandlerRaw(scanBuffers);

                if(wordSize > 1)
                {
                    std::vector<std::uint8_t> tempBuffer(writeSize);
                    ::memcpy(tempBuffer.data(), pDataHandlerRaw->getMemoryBuffer(), pDataHandlerRaw->getSize());
                    if(writeSize != bufferSize)
                    {
                        tempBuffer[bufferSize] = pData->getPaddingByte();
                    }
                    if(pBuffer->getEndianType() != endianType)
                    {
                        streamController::reverseEndian(tempBuffer.data(), wordSize, writeSize / wordSize);
                    }
                    pDestStream->write(tempBuffer.data(), writeSize);

                }
                else
                {
                    pDestStream->write(reinterpret_cast<const std::uint8_t*>(pDataHandlerRaw->getMemoryBuffer()), bufferSize);
                    if(bufferSize != writeSize)
                    {
                        const std::uint8_t paddingByte(pData->getPaddingByte());
                        pDestStream->write(&paddingByte, 1);
                    }
                }

            }

            continue;

        }

        // Write a nested dataset
        ///////////////////////////////////////////////////////////
        if(!pData->dataSetExists(scanBuffers))
        {
            break;
        }
        std::shared_ptr<dataSet> pDataSet = pData->getSequenceItem(scanBuffers);

        // Don't write empty datasets
        ///////////////////////////////////////////////////////////
        if(pDataSet->getGroups().empty())
        {
            continue;
        }

        // Remember the position at which the item has been written
        ///////////////////////////////////////////////////////////
        pDataSet->setItemOffset(static_cast<std::uint32_t>(pDestStream->getControlledStreamPosition()));

        // write the sequence item header
        ///////////////////////////////////////////////////////////
        pDestStream->write(reinterpret_cast<const std::uint8_t*>(&sequenceItemGroup), 2);
        pDestStream->write(reinterpret_cast<const std::uint8_t*>(&sequenceItemDelimiter), 2);
        std::uint32_t sequenceItemLength = getDataSetLength(pDataSet, bExplicitDataType);
        pDestStream->adjustEndian(reinterpret_cast<std::uint8_t*>(&sequenceItemLength), 4, endianType);
        pDestStream->write(reinterpret_cast<const std::uint8_t*>(&sequenceItemLength), 4);

        // write the dataset
        ///////////////////////////////////////////////////////////
        buildStream(pDestStream, pDataSet, bExplicitDataType, endianType, streamType_t::normal);
    }

    // write the sequence item end marker
    ///////////////////////////////////////////////////////////
    if(bSequence)
    {
        pDestStream->write(reinterpret_cast<const std::uint8_t*>(&sequenceItemGroup), 2);
        pDestStream->write(reinterpret_cast<const std::uint8_t*>(&sequenceTagEnd), 2);
        pDestStream->write(reinterpret_cast<const std::uint8_t*>(&zeroLength), 4);
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Calculate the tag's length
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dicomStreamCodec::getTagLength(const std::shared_ptr<data>& pData, bool bExplicitDataType, std::uint32_t* pHeaderLength, bool *pbSequence)
{
    IMEBRA_FUNCTION_START();

    tagVR_t dataType = pData->getDataType();
    *pbSequence = (dataType == tagVR_t::SQ);
    std::uint32_t numberOfElements = 0;
    std::uint32_t totalLength = 0;
    for(std::uint32_t scanBuffers = 0; ; ++scanBuffers, ++numberOfElements)
    {
        if(pData->dataSetExists(scanBuffers))
        {
            std::shared_ptr<dataSet> pDataSet = pData->getSequenceItem(scanBuffers);
            totalLength += getDataSetLength(pDataSet, bExplicitDataType);
            totalLength += 8; // item tag and item length
            *pbSequence = true;
            continue;
        }
        if(!pData->bufferExists(scanBuffers))
        {
            break;
        }

        size_t bufferSize = static_cast<std::uint32_t>(pData->getBufferSize(scanBuffers));
        if((bufferSize & 1u) == 1u)
        {
            ++bufferSize;
        }
        totalLength += static_cast<std::uint32_t>(bufferSize);
    }

    (*pbSequence) |= (numberOfElements > 1);

    // Find the tag type
    bool bLongLength = dicomDictionary::getDicomDictionary()->getLongLength(dataType);

    *pHeaderLength = 8;
    if((bLongLength || (*pbSequence)) && bExplicitDataType)
    {
        (*pHeaderLength) +=4;
    }

    if(*pbSequence)
    {
        totalLength += (numberOfElements+1) * 8;
    }

    return totalLength;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Calculate the group's length
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dicomStreamCodec::getGroupLength(const dataSet::tTags& tags, bool bExplicitDataType)
{
    IMEBRA_FUNCTION_START();

    std::uint32_t totalLength(0);

    for(dataSet::tTags::const_iterator scanTags(tags.begin()), endTags(tags.end()); scanTags != endTags; ++scanTags)
    {
        if(scanTags->first == 0)
        {
            continue;
        }

        std::uint32_t tagHeaderLength;
        bool bSequence;
        totalLength += getTagLength(scanTags->second, bExplicitDataType, &tagHeaderLength, &bSequence);
        totalLength += tagHeaderLength;
    }

    return totalLength;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Calculate the dataset's length
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dicomStreamCodec::getDataSetLength(std::shared_ptr<dataSet> pDataSet, bool bExplicitDataType)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<nullStreamWriter> pNullStream(std::make_shared<nullStreamWriter>());

    std::shared_ptr<streamWriter> pWriter(std::make_shared<streamWriter>(pNullStream));

    buildStream(pWriter, pDataSet, bExplicitDataType, streamController::lowByteEndian, streamType_t::normal);

    return static_cast<std::uint32_t>(pWriter->position());

    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read a DICOM stream and fill the dataset with the
//  DICOM's content
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomStreamCodec::readStream(std::shared_ptr<streamReader> pStream, std::shared_ptr<dataSet> pDataSet, std::uint32_t maxSizeBufferLoad /* = 0xffffffff */) const
{
    IMEBRA_FUNCTION_START();

    // This flag signals a failure
    ///////////////////////////////////////////////////////////
    bool bFailed=false;

    // Read the old dicom signature (NEMA)
    ///////////////////////////////////////////////////////////
    std::uint8_t oldDicomSignature[8];

    try
    {
        pStream->read(oldDicomSignature, 8);
    }
    catch(StreamEOFError&)
    {
        IMEBRA_THROW(CodecWrongFormatError, "detected a wrong format");
    }

    // Skip the first 128 bytes (8 already skipped)
    ///////////////////////////////////////////////////////////
    pStream->seekForward(120u);

    // Read the DICOM signature (DICM)
    ///////////////////////////////////////////////////////////
    std::uint8_t dicomSignature[4];
    pStream->read(dicomSignature, 4);

    // Check the DICM signature
    ///////////////////////////////////////////////////////////
    const char* checkSignature="DICM";
    if(::memcmp(dicomSignature, checkSignature, 4) != 0)
    {
        bFailed=true;
    }

    bool bExplicitDataType = true;
    streamController::tByteOrdering endianType=streamController::lowByteEndian;
    if(bFailed)
    {
        // Tags 0x8 and 0x2 are accepted in the begin of the file
        ///////////////////////////////////////////////////////////
        if(
                (oldDicomSignature[0]!=0x8 && oldDicomSignature[0]!=0x2) ||
                oldDicomSignature[1]!=0x0 ||
                oldDicomSignature[3]!=0x0)
        {
            IMEBRA_THROW(CodecWrongFormatError, "Not a DICOM file");
        }

        // Set "explicit data type" to true if a valid data type
        //  is found
        ///////////////////////////////////////////////////////////
        std::string firstDataType;
        firstDataType.push_back((char)(oldDicomSignature[4]));
        firstDataType.push_back((char)(oldDicomSignature[5]));
        bExplicitDataType = dicomDictionary::getDicomDictionary()->isDataTypeValid(firstDataType);
    }

    // Signature OK. Now scan all the tags.
    ///////////////////////////////////////////////////////////
    parseStream(pStream, pDataSet, bExplicitDataType, endianType, maxSizeBufferLoad);

    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Parse a Dicom stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomStreamCodec::parseStream(std::shared_ptr<streamReader> pStream,
                             std::shared_ptr<dataSet> pDataSet,
                             bool bExplicitDataType,
                             streamController::tByteOrdering endianType,
                             std::uint32_t maxSizeBufferLoad /* = 0xffffffff */,
                             std::uint32_t subItemLength /* = 0xffffffff */,
                             std::uint32_t* pReadSubItemLength /* = 0 */,
                             std::uint32_t depth)
{
    IMEBRA_FUNCTION_START();

    if(depth > IMEBRA_DATASET_MAX_DEPTH)
    {
        IMEBRA_THROW(DicomCodecDepthLimitReachedError, "Depth for embedded dataset reached");
    }

    std::uint16_t tagId;
    std::uint16_t tagSubId;
    std::uint16_t tagLengthWord;
    std::uint32_t tagLengthDWord;

    // Used to calculate the group order
    ///////////////////////////////////////////////////////////
    std::uint16_t order = 0;
    std::uint16_t lastGroupId = 0;
    std::uint16_t lastTagId = 0;

    std::uint32_t tempReadSubItemLength = 0; // used when the last parameter is not defined
    bool       bFirstTag = (pReadSubItemLength == 0);
    bool       bCheckTransferSyntax = bFirstTag;
    size_t     wordSize;

    if(pReadSubItemLength == 0)
    {
        pReadSubItemLength = &tempReadSubItemLength;
    }
    *pReadSubItemLength = 0;

    ///////////////////////////////////////////////////////////
    //
    // Read all the tags
    //
    ///////////////////////////////////////////////////////////
    while(!pStream->endReached() && (*pReadSubItemLength < subItemLength))
    {
        // Get the tag's ID
        ///////////////////////////////////////////////////////////
        pStream->read((std::uint8_t*)&tagId, sizeof(tagId));
        pStream->adjustEndian((std::uint8_t*)&tagId, sizeof(tagId), endianType);
        (*pReadSubItemLength) += (std::uint32_t)sizeof(tagId);

        // Check for EOF
        ///////////////////////////////////////////////////////////
        if(pStream->endReached())
        {
            break;
        }

        // Check the byte order
        ///////////////////////////////////////////////////////////
        if(bFirstTag && tagId==0x0200)
        {
            // Reverse the last adjust
            pStream->adjustEndian((std::uint8_t*)&tagId, sizeof(tagId), endianType);

            // Fix the byte adjustment
            endianType=streamController::highByteEndian;

            // Redo the byte adjustment
            pStream->adjustEndian((std::uint8_t*)&tagId, sizeof(tagId), endianType);
        }

        // If this tag's id is not 0x0002, then load the
        //  transfer syntax and set the byte endian.
        ///////////////////////////////////////////////////////////
        if(tagId != 0x0002 && bCheckTransferSyntax)
        {
            // Reverse the last adjust
            pStream->adjustEndian((std::uint8_t*)&tagId, sizeof(tagId), endianType);

            std::string transferSyntax = pDataSet->getString(
                        0x0002,
                        0x0,
                        0x0010,
                        0,
                        0,
                        endianType == streamController::lowByteEndian ? "1.2.840.10008.1.2.1" : "1.2.840.10008.1.2.2");

            if(transferSyntax == "1.2.840.10008.1.2.2")
                endianType = streamController::highByteEndian;
            if(transferSyntax == "1.2.840.10008.1.2")
                bExplicitDataType=false;

            // Redo the byte adjustment
            pStream->adjustEndian((std::uint8_t*)&tagId, sizeof(tagId), endianType);

            bCheckTransferSyntax=false;
        }

        // The first tag's ID has been read
        ///////////////////////////////////////////////////////////
        bFirstTag = false;

        // Set the word's length to the default value
        ///////////////////////////////////////////////////////////
        wordSize = 1;

        // Get the tag's sub ID
        ///////////////////////////////////////////////////////////
        pStream->read((std::uint8_t*)&tagSubId, sizeof(tagSubId));
        pStream->adjustEndian((std::uint8_t*)&tagSubId, sizeof(tagSubId), endianType);
        (*pReadSubItemLength) += (std::uint32_t)sizeof(tagSubId);

        // Check for the end of the dataset
        ///////////////////////////////////////////////////////////
        if(tagId==0xfffe && tagSubId==0xe00d)
        {
            // skip the tag's length and exit
            std::uint32_t dummyDWord;
            pStream->read((std::uint8_t*)&dummyDWord, 4);
            (*pReadSubItemLength) += 4;
            break;
        }

        //
        // Explicit data type
        //
        ///////////////////////////////////////////////////////////
        tagVR_t tagType(tagVR_t::UN);

        if(bExplicitDataType && tagId!=0xfffe)
        {
            // Get the tag's type
            ///////////////////////////////////////////////////////////
            std::string tagTypeString((size_t)2, ' ');

            pStream->read((std::uint8_t*)&(tagTypeString[0]), 2);
            (*pReadSubItemLength) += 2;

            // Get the tag's length
            ///////////////////////////////////////////////////////////
            pStream->read((std::uint8_t*)&tagLengthWord, sizeof(tagLengthWord));
            pStream->adjustEndian((std::uint8_t*)&tagLengthWord, sizeof(tagLengthWord), endianType);
            (*pReadSubItemLength) += (std::uint32_t)sizeof(tagLengthWord);

            // The data type is valid
            ///////////////////////////////////////////////////////////
            try
            {
                tagType = dicomDictionary::getDicomDictionary()->stringDataTypeToEnum(tagTypeString);
                tagLengthDWord=(std::uint32_t)tagLengthWord;
                wordSize = dicomDictionary::getDicomDictionary()->getWordSize(tagType);
                if(dicomDictionary::getDicomDictionary()->getLongLength(tagType))
                {
                    pStream->read((std::uint8_t*)&tagLengthDWord, sizeof(tagLengthDWord));
                    pStream->adjustEndian((std::uint8_t*)&tagLengthDWord, sizeof(tagLengthDWord), endianType);
                    (*pReadSubItemLength) += (std::uint32_t)sizeof(tagLengthDWord);
                }
            }
            catch(const DictionaryUnknownDataTypeError&)
            {
                // The data type is not valid. Switch to implicit data type
                ///////////////////////////////////////////////////////////
                bExplicitDataType = false;
                if(endianType == streamController::lowByteEndian)
                    tagLengthDWord=(((std::uint32_t)tagLengthWord)<<16) | ((std::uint32_t)tagTypeString[0]) | (((std::uint32_t)tagTypeString[1])<<8);
                else
                    tagLengthDWord=(std::uint32_t)tagLengthWord | (((std::uint32_t)tagTypeString[0])<<24) | (((std::uint32_t)tagTypeString[1])<<16);
            }


        } // End of the explicit data type read block


        ///////////////////////////////////////////////////////////
        //
        // Implicit data type
        //
        ///////////////////////////////////////////////////////////
        else
        {
            // Get the tag's length
            ///////////////////////////////////////////////////////////
            pStream->read((std::uint8_t*)&tagLengthDWord, sizeof(tagLengthDWord));
            pStream->adjustEndian((std::uint8_t*)&tagLengthDWord, sizeof(tagLengthDWord), endianType);
            (*pReadSubItemLength) += (std::uint32_t)sizeof(tagLengthDWord);
        }

        ///////////////////////////////////////////////////////////
        //
        // Find the default data type and the tag's word's size
        //
        ///////////////////////////////////////////////////////////
        if((!bExplicitDataType || tagId==0xfffe))
        {
            // Group length. Data type is always UL
            ///////////////////////////////////////////////////////////
            if(tagSubId == 0)
            {
                tagType = tagVR_t::UL;
            }
            else
            {
                try
                {
                    tagType = dicomDictionary::getDicomDictionary()->getTagType(tagId, tagSubId);
                }
                catch(const DictionaryUnknownTagError&)
                {
                    tagType = tagVR_t::UN;
                }
                wordSize = dicomDictionary::getDicomDictionary()->getWordSize(tagType);
            }
        }

        // Check for the end of a sequence
        ///////////////////////////////////////////////////////////
        if(tagId==0xfffe && tagSubId==0xe0dd)
        {
            break;
        }

        ///////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////
        //
        //
        // Read the tag's buffer
        //
        //
        ///////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////
        //
        // Adjust the order when multiple groups with the same
        //  id are present.
        //
        ///////////////////////////////////////////////////////////
        if(tagId != 0 && tagId<=lastGroupId && tagSubId<=lastTagId)
        {
            ++order;
        }
        else
        {
            if(tagId>lastGroupId)
            {
                order = 0;
            }
        }
        lastGroupId=tagId;
        lastTagId=tagSubId;

        if(tagLengthDWord != 0xffffffff && tagType != tagVR_t::SQ)
        {
            (*pReadSubItemLength) += readTag(pStream, pDataSet, tagLengthDWord, tagId, order, tagSubId, tagType, endianType, wordSize, 0, maxSizeBufferLoad);

            // We found the charsets list
            if(tagId == 0x0008 && tagSubId == 0x0005)
            {
                std::shared_ptr<handlers::readingDataHandler> charsetsHandler(pDataSet->getReadingDataHandler(0x0008, 0, 0x0005, 0));
                charsetsList_t charsets;
                for(size_t componentId(0); componentId != charsetsHandler->getSize(); ++componentId)
                {
                    const std::string charset(charsetsHandler->getString(componentId));
                    if(!charset.empty())
                    {
                        charsets.push_back(charset);
                    }
                }
                pDataSet->setCharsetsList(charsets);
            }
            continue;
        }

        ///////////////////////////////////////////////////////////
        //
        // We are within an undefined-length tag or a sequence
        //
        ///////////////////////////////////////////////////////////

        // Parse all the sequence's items
        ///////////////////////////////////////////////////////////
        std::uint16_t subItemGroupId;
        std::uint16_t subItemTagId;
        std::uint32_t sequenceItemLength;
        std::uint32_t bufferId = 0;
        while(tagLengthDWord && !pStream->endReached())
        {
            // Add the tag to the dataset
            ///////////////////////////////////////////////////////////
            std::shared_ptr<data> sequenceTag = pDataSet->getTagCreate(tagId, 0x0, tagSubId, tagType);

            // Remember the item's position (used by DICOMDIR
            //  structures)
            ///////////////////////////////////////////////////////////
            std::uint32_t itemOffset((std::uint32_t)pStream->getControlledStreamPosition());

            // Read the sequence item's group
            ///////////////////////////////////////////////////////////
            pStream->read((std::uint8_t*)&subItemGroupId, sizeof(subItemGroupId));
            pStream->adjustEndian((std::uint8_t*)&subItemGroupId, sizeof(subItemGroupId), endianType);
            (*pReadSubItemLength) += (std::uint32_t)sizeof(subItemGroupId);

            // Read the sequence item's id
            ///////////////////////////////////////////////////////////
            pStream->read((std::uint8_t*)&subItemTagId, sizeof(subItemTagId));
            pStream->adjustEndian((std::uint8_t*)&subItemTagId, sizeof(subItemTagId), endianType);
            (*pReadSubItemLength) += (std::uint32_t)sizeof(subItemTagId);

            // Read the sequence item's length
            ///////////////////////////////////////////////////////////
            pStream->read((std::uint8_t*)&sequenceItemLength, sizeof(sequenceItemLength));
            pStream->adjustEndian((std::uint8_t*)&sequenceItemLength, sizeof(sequenceItemLength), endianType);
            (*pReadSubItemLength) += (std::uint32_t)sizeof(sequenceItemLength);

            if(tagLengthDWord!=0xffffffff)
            {
                tagLengthDWord-=8;
            }

            // check for the end of the undefined length sequence
            ///////////////////////////////////////////////////////////
            if(subItemGroupId==0xfffe && subItemTagId==0xe0dd)
            {
                break;
            }

            ///////////////////////////////////////////////////////////
            // Parse a sub element
            ///////////////////////////////////////////////////////////
            if((sequenceItemLength == 0xffffffff) || tagType == tagVR_t::SQ)
            {
                std::shared_ptr<dataSet> sequenceDataSet(sequenceTag->appendSequenceItem());
                sequenceDataSet->setItemOffset(itemOffset);
                std::uint32_t effectiveLength(0);
                parseStream(pStream, sequenceDataSet, bExplicitDataType, endianType, maxSizeBufferLoad, sequenceItemLength, &effectiveLength, depth + 1);
                (*pReadSubItemLength) += effectiveLength;
                if(tagLengthDWord!=0xffffffff)
                    tagLengthDWord-=effectiveLength;

                continue;
            }

            ///////////////////////////////////////////////////////////
            // Read a buffer's element
            ///////////////////////////////////////////////////////////
            sequenceItemLength = readTag(pStream, pDataSet, sequenceItemLength, tagId, order, tagSubId, tagType, endianType, wordSize, bufferId++, maxSizeBufferLoad);
            (*pReadSubItemLength) += sequenceItemLength;
            if(tagLengthDWord!=0xffffffff)
            {
                tagLengthDWord -= sequenceItemLength;
            }
        }

    } // End of the tags-read block

    IMEBRA_FUNCTION_END();

}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read a single tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dicomStreamCodec::readTag(
        std::shared_ptr<streamReader> pStream,
        std::shared_ptr<dataSet> pDataSet,
        std::uint32_t tagLengthDWord,
        std::uint16_t tagId,
        std::uint16_t order,
        std::uint16_t tagSubId,
        tagVR_t tagType,
        streamController::tByteOrdering endianType,
        size_t wordSize,
        std::uint32_t bufferId,
        std::uint32_t maxSizeBufferLoad /* = 0xffffffff */
        )
{
    IMEBRA_FUNCTION_START();

    // If the tag's size is bigger than the maximum loadable
    //  size then just specify in which file it resides
    ///////////////////////////////////////////////////////////
    if(tagLengthDWord > maxSizeBufferLoad)
    {
        size_t bufferPosition(pStream->position());
        size_t streamPosition(pStream->getControlledStreamPosition());
        pStream->seekForward(tagLengthDWord);
        size_t bufferLength(pStream->position() - bufferPosition);

        if(bufferLength != tagLengthDWord)
        {
            IMEBRA_THROW(CodecCorruptedFileError, "dicomCodec::readTag detected a corrupted tag");
        }

        std::shared_ptr<data> writeData (pDataSet->getTagCreate(tagId, order, tagSubId, tagType));
        writeData->getBufferCreate(bufferId,
                                   pStream->getControlledStream(),
                                   streamPosition,
                                   bufferLength,
                                   wordSize,
                                   endianType);

        return (std::uint32_t)bufferLength;
    }

    // Allocate the tag's buffer
    ///////////////////////////////////////////////////////////
    std::shared_ptr<handlers::writingDataHandlerRaw> handler(pDataSet->getWritingDataHandlerRaw(tagId, order, tagSubId, bufferId, tagType));

    // Do nothing if the tag's size is 0
    ///////////////////////////////////////////////////////////
    if(tagLengthDWord == 0)
    {
        return 0;
    }

    // In order to deal with damaged tags asking for an
    //  incredible amount of memory, this function reads the
    //  tag using a lot of small buffers (32768 bytes max)
    //  and then the tag's buffer is rebuilt at the end of the
    //  function.
    // This method saves a lot of time if a huge amount of
    //  memory is asked by a damaged tag, since only the amount
    //  of memory actually stored in the source file is
    //  allocated
    ///////////////////////////////////////////////////////////

    // If the buffer size is bigger than the following const
    //  variable, then read the buffer in small chunks
    ///////////////////////////////////////////////////////////
    const std::uint32_t smallBuffersSize(32768);

    if(tagLengthDWord <= smallBuffersSize) // Read in one go
    {
        handler->setSize(tagLengthDWord);
        pStream->read(handler->getMemoryBuffer(), tagLengthDWord);
    }
    else // Read in small chunks
    {
        std::list<std::vector<std::uint8_t> > buffers;

        // Used to keep track of the read bytes
        ///////////////////////////////////////////////////////////
        std::uint32_t remainingBytes(tagLengthDWord);

        // Fill all the small buffers
        ///////////////////////////////////////////////////////////
        while(remainingBytes != 0)
        {
            // Calculate the small buffer's size and allocate it
            ///////////////////////////////////////////////////////////
            std::uint32_t thisBufferSize( (remainingBytes > smallBuffersSize) ? smallBuffersSize : remainingBytes);
            buffers.push_back(std::vector<std::uint8_t>());
            buffers.back().resize(thisBufferSize);

            // Fill the buffer
            ///////////////////////////////////////////////////////////
            pStream->read(&buffers.back()[0], thisBufferSize);

            // Decrease the number of the remaining bytes
            ///////////////////////////////////////////////////////////
            remainingBytes -= thisBufferSize;
        }

        // Copy the small buffers into the tag object
        ///////////////////////////////////////////////////////////
        handler->setSize(tagLengthDWord);
        std::uint8_t* pHandlerBuffer(handler->getMemoryBuffer());

        // Scan all the small buffers and copy their content into
        //  the final buffer
        ///////////////////////////////////////////////////////////
        std::list<std::vector<std::uint8_t> >::iterator smallBuffersIterator;
        remainingBytes = tagLengthDWord;
        for(smallBuffersIterator=buffers.begin(); smallBuffersIterator != buffers.end(); ++smallBuffersIterator)
        {
            std::uint32_t copySize=(remainingBytes>smallBuffersSize) ? smallBuffersSize : remainingBytes;
            ::memcpy(pHandlerBuffer, &(*smallBuffersIterator)[0], copySize);
            pHandlerBuffer += copySize;
            remainingBytes -= copySize;
        }
    } // end of reading from stream

    // All the bytes have been read, now rebuild the tag's
    //  buffer. Don't rebuild the tag if it is 0xfffc,0xfffc
    //  (end of the stream)
    ///////////////////////////////////////////////////////////
    if(tagId == 0xfffc && tagSubId == 0xfffc)
    {
        return (std::uint32_t)tagLengthDWord;
    }

    // Adjust the buffer's byte endian
    ///////////////////////////////////////////////////////////
    if(wordSize != 0)
    {
        pStream->adjustEndian(handler->getMemoryBuffer(), wordSize, endianType, tagLengthDWord / wordSize);
    }

    // Return the tag's length in bytes
    ///////////////////////////////////////////////////////////
    return (std::uint32_t)tagLengthDWord;

    IMEBRA_FUNCTION_END();
}

} // namespace codecs

} // namespace implementation

} // namespace imebra

