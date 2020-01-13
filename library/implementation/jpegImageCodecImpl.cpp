/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file jpegImageCodec.cpp
    \brief Implementation of the class jpegImageCodec.

*/

#include "exceptionImpl.h"
#include "streamReaderImpl.h"
#include "streamWriterImpl.h"
#include "huffmanTableImpl.h"
#include "jpegImageCodecImpl.h"
#include "dataSetImpl.h"
#include "imageImpl.h"
#include "dataHandlerNumericImpl.h"
#include "codecFactoryImpl.h"
#include "../include/imebra/exceptions.h"
#include <vector>
#include <stdlib.h>
#include <string.h>

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
// Translate zig-zag order in 8x8 blocks to raw order
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegDeZigZagOrder[]=
{
    0, 1, 8, 16, 9, 2, 3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagDQT
//
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
namespace jpeg
{

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write the DQT tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDQT::writeTag(streamWriter* pStream, jpegInformation& information) const
{
    IMEBRA_FUNCTION_START();

    // Read the tag's length
    /////////////////////////////////////////////////////////////////
    std::uint16_t tagLength = 0;

    std::uint8_t  tablePrecision;
    std::uint8_t  tableValue8;
    std::uint16_t tableValue16;

    for(int phase = 0; phase < 2; ++phase)
    {
        if(phase != 0)
        {
            writeLength(pStream, tagLength);
        }
        for(std::uint8_t tableId = 0; tableId < 16; ++tableId)
        {
            // bAdd is true if the huffman table is used by a channel
            /////////////////////////////////////////////////////////////////
            bool bAdd=false;

            for(jpeg::jpegInformation::tChannelsMap::iterator channelsIterator = information.m_channelsMap.begin(); !bAdd && channelsIterator != information.m_channelsMap.end(); ++channelsIterator)
            {
                ptrChannel pChannel=channelsIterator->second;
                bAdd=pChannel->m_quantTable==tableId;
            }

            if(!bAdd)
            {
                continue;
            }
            // Calculate the table's precision
            bool b16Bits = information.m_precision > 8;
            for(int tableIndex = 0; !b16Bits && (tableIndex < 64); ++tableIndex)
            {
                if(information.m_quantizationTable[tableId][tableIndex] >= 256)
                {
                    b16Bits=true;
                }
            }

            if(phase == 0)
            {
                tagLength = (std::uint16_t)(tagLength + 1 + (b16Bits ? 128 : 64));
            }
            else
            {
                tablePrecision = (std::uint8_t)(tableId | (b16Bits ? 0x10 : 0));
                pStream->write(&tablePrecision, 1);
                if(b16Bits)
                {
                    for(int tableIndex = 0; tableIndex < 64; ++tableIndex)
                    {
                        tableValue16 = (std::uint16_t)information.m_quantizationTable[tableId][JpegDeZigZagOrder[tableIndex]];
                        pStream->adjustEndian((std::uint8_t*)&tableValue16, 2, streamController::highByteEndian);
                        pStream->write((std::uint8_t*)&tableValue16, 2);
                    }
                }
                else
                {
                    for(int tableIndex = 0; tableIndex < 64; ++tableIndex)
                    {
                        tableValue8=(std::uint8_t)information.m_quantizationTable[tableId][JpegDeZigZagOrder[tableIndex]];
                        pStream->write(&tableValue8, 1);
                    }
                }
            }

            information.recalculateQuantizationTables(tableId);
        }
    }

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the DQT tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDQT::readTag(streamReader& stream, jpegInformation* pInformation, std::uint8_t /* tagEntry */) const
{
    IMEBRA_FUNCTION_START();

    // tag dedicated stream (throws if we attempt to read past
    //  the tag bytes)
    //////////////////////////////////////////////////////////
    const std::uint32_t tagLength = readLength(stream);
    std::shared_ptr<streamReader> tagReader(stream.getReader(tagLength));

    std::uint8_t  tablePrecision;
    std::uint8_t  tableValue8;
    std::uint16_t tableValue16;
    while(!tagReader->endReached())
    {
        tagReader->read(&tablePrecision, 1);

        // Read a DQT table
        /////////////////////////////////////////////////////////////////
        for(int tableIndex = 0; tableIndex < 64; ++tableIndex)
        {
            // 16 bits precision
            /////////////////////////////////////////////////////////////////
            if((tablePrecision & 0xf0) != 0)
            {
                tagReader->read((std::uint8_t*)&tableValue16, 2);
                tagReader->adjustEndian((std::uint8_t*)&tableValue16, 2, streamController::highByteEndian);
                pInformation->m_quantizationTable[tablePrecision & 0x0f][JpegDeZigZagOrder[tableIndex]]=tableValue16;
            }

            // 8 bits precision
            /////////////////////////////////////////////////////////////////
            else
            {
                tagReader->read(&tableValue8, 1);
                pInformation->m_quantizationTable[tablePrecision & 0x0f][JpegDeZigZagOrder[tableIndex]]=tableValue8;
            }

        } // ----- End of table reading

        pInformation->recalculateQuantizationTables(tablePrecision & 0x0f);
    }

    IMEBRA_FUNCTION_END_MODIFY(StreamEOFError, CodecCorruptedFileError);

}

} // namespace jpeg


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodec
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Returns true if the codec can handle the specified transfer
//  syntax
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
bool jpegImageCodec::canHandleTransferSyntax(const std::string& transferSyntax) const
{
    return (
                transferSyntax == "1.2.840.10008.1.2.4.50" ||  // baseline (8 bits lossy)
                transferSyntax == "1.2.840.10008.1.2.4.51" ||  // extended (12 bits lossy)
                transferSyntax == "1.2.840.10008.1.2.4.57" ||  // lossless NH
                transferSyntax == "1.2.840.10008.1.2.4.70");   // lossless NH first order prediction
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Returns true if the transfer syntax has to be
//  encapsulated
//
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
bool jpegImageCodec::encapsulated(const std::string& transferSyntax) const
{
    IMEBRA_FUNCTION_START();

    if(!canHandleTransferSyntax(transferSyntax))
    {
        IMEBRA_THROW(CodecWrongTransferSyntaxError, "Cannot handle the transfer syntax");
    }
    return true;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the suggested allocated bits
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t jpegImageCodec::suggestAllocatedBits(const std::string& transferSyntax, std::uint32_t highBit) const
{
    if(transferSyntax == "1.2.840.10008.1.2.4.50")
    {
        return 8;
    }
    if(transferSyntax == "1.2.840.10008.1.2.4.51")
    {
        return 12;
    }
    return (highBit + 8) & 0xfffffff8;
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Get a jpeg image from a Dicom dataset
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
std::shared_ptr<image> jpegImageCodec::getImage(const std::string& transferSyntax,
                                                const std::string& colorSpace,
                                                std::uint32_t /* channelsNumber */,
                                                std::uint32_t /* imageWidth */,
                                                std::uint32_t /* imageHeight */,
                                                bool /* bSubSampledX */,
                                                bool /* bSubSampledY */,
                                                bool /* bInterleaved */,
                                                bool b2Complement,
                                                std::uint8_t /* allocatedBits */,
                                                std::uint8_t /* storedBits */,
                                                std::uint8_t /* highBit */,
                                                std::shared_ptr<streamReader> pSourceStream) const
{
    IMEBRA_FUNCTION_START();

    jpegStreamReader jpegStream(pSourceStream);

    // Read the Jpeg signature
    ///////////////////////////////////////////////////////////
    std::uint8_t jpegSignature[2];

    try
    {
        pSourceStream->read(jpegSignature, 2);
    }
    catch(StreamEOFError&)
    {
        IMEBRA_THROW(CodecWrongFormatError, "Jpeg signature not present");
    }

    // If the jpeg signature is wrong, then return an error
    //  condition
    ///////////////////////////////////////////////////////////
    const std::uint8_t checkSignature[2]={(std::uint8_t)0xff, (std::uint8_t)0xd8};
    if(::memcmp(jpegSignature, checkSignature, 2) != 0)
    {
        IMEBRA_THROW(CodecWrongFormatError, "Jpeg signature not valid");
    }

    // Read until the end of the image is reached
    ///////////////////////////////////////////////////////////
    jpeg::jpegInformation information;
    for(; !information.m_bEndOfImage; jpegStream.resetInBitsBuffer())
    {
        std::uint32_t nextMcuStop = information.m_mcuNumberTotal;
        if(information.m_mcuPerRestartInterval != 0)
        {
            nextMcuStop = information.m_mcuLastRestart + information.m_mcuPerRestartInterval;
            if(nextMcuStop > information.m_mcuNumberTotal)
            {
                nextMcuStop = information.m_mcuNumberTotal;
            }
        }

        if(nextMcuStop <= information.m_mcuProcessed)
        {
            // Look for a tag. Skip all the FF bytes
            std::uint8_t tagId(0xff);

            try
            {
                pSourceStream->read(&tagId, 1);
                if(tagId != 0xff)
                {
                    continue;
                }

                while(tagId == 0xff)
                {
                    pSourceStream->read(&tagId, 1);
                }


                // An entry has been found. Process it
                ///////////////////////////////////////////////////////////
                std::shared_ptr<jpeg::tag> pTag;
                tTagsMap::const_iterator findTag = m_tagsMap.find(tagId);
                if(findTag != m_tagsMap.end())
                    pTag = findTag->second;
                else
                    pTag = m_tagsMap.find(0xff)->second;

                pTag->readTag(*pSourceStream, &information, tagId);
            }
            catch(const StreamEOFError& e)
            {
                if(information.m_mcuProcessed == information.m_mcuNumberTotal && information.m_mcuNumberTotal != 0)
                {
                    information.m_bEndOfImage = true;
                }
                else
                {
                    IMEBRA_THROW(CodecCorruptedFileError, "End of the jpeg stream found while reading jpeg data");
                }
            }
            continue;

        }

        while(information.m_mcuProcessed < nextMcuStop && !pSourceStream->endReached())
        {
            // Read an MCU
            ///////////////////////////////////////////////////////////

            // Scan all components
            ///////////////////////////////////////////////////////////
            for(const std::shared_ptr<jpeg::jpegChannel>& pChannel: information.m_channelsList)
            {
                // Read a lossless pixel
                ///////////////////////////////////////////////////////////
                if(information.m_bLossless)
                {
                    for(std::uint32_t
                        scanBlock = 0;
                        scanBlock != pChannel->m_blockMcuXY;
                        ++scanBlock)
                    {
                        std::uint32_t amplitudeLength = pChannel->m_pActiveHuffmanTableDC->readHuffmanCode(jpegStream);
                        std::int32_t amplitude;
                        if(amplitudeLength == 16) // logically we should compare with information.m_precision, but DICOM says otherwise
                        {
                            amplitude = (std::int32_t)1 << 15;
                        }
                        else if(amplitudeLength != 0)
                        {
                            amplitude = (std::int32_t)jpegStream.readBits(amplitudeLength);
                            if(amplitude < ((std::int32_t)1<<(amplitudeLength-1)))
                            {
                                amplitude -= ((std::int32_t)1<<amplitudeLength)-1;
                            }
                        }
                        else
                        {
                            amplitude = 0;
                        }

                        pChannel->addUnprocessedAmplitude(amplitude, information.m_spectralIndexStart, information.m_mcuLastRestart == information.m_mcuProcessed && scanBlock == 0);
                    }

                    continue;
                }

                // Read a lossy MCU
                ///////////////////////////////////////////////////////////
                std::uint32_t bufferPointer = (information.m_mcuProcessedY * pChannel->m_blockMcuY * ((information.m_jpegImageWidth * pChannel->m_samplingFactorX / information.m_maxSamplingFactorX) >> 3) + information.m_mcuProcessedX * pChannel->m_blockMcuX) * 64;
                for(std::uint32_t scanBlockY = pChannel->m_blockMcuY; (scanBlockY != 0); --scanBlockY)
                {
                    for(std::uint32_t scanBlockX = pChannel->m_blockMcuX; scanBlockX != 0; --scanBlockX)
                    {
                        readBlock(jpegStream, information, &(pChannel->m_pBuffer[bufferPointer]), pChannel);

                        if(information.m_spectralIndexEnd >= 63)
                        {
                            IDCT(
                                        &(pChannel->m_pBuffer[bufferPointer]),
                                        information.m_decompressionQuantizationTable[pChannel->m_quantTable]
                                    );
                        }
                        bufferPointer += 64;
                    }
                    bufferPointer += (information.m_mcuNumberX -1) * pChannel->m_blockMcuX * 64;
                }
            }

            ++information.m_mcuProcessed;
            if(++information.m_mcuProcessedX == information.m_mcuNumberX)
            {
                information.m_mcuProcessedX = 0;
                ++information.m_mcuProcessedY;
            }
        }
    }

    // Process unprocessed lossless amplitudes
    ///////////////////////////////////////////////////////////
    for(jpeg::jpegInformation::tChannelsMap::iterator processLosslessIterator = information.m_channelsMap.begin();
        processLosslessIterator != information.m_channelsMap.end();
        ++processLosslessIterator)
    {
        processLosslessIterator->second->processUnprocessedAmplitudes();
    }


    // If the compression is jpeg baseline or jpeg extended
    //  then the color space cannot be "RGB"
    ///////////////////////////////////////////////////////////
    if(colorSpace == "RGB" && (transferSyntax == "1.2.840.10008.1.2.4.50" ||  // baseline (8 bits lossy)
                transferSyntax == "1.2.840.10008.1.2.4.51"))    // extended (12 bits lossy)
    {
        return copyJpegChannelsToImage(information, b2Complement, "YBR_FULL");
    }

    return copyJpegChannelsToImage(information, b2Complement, colorSpace);

    IMEBRA_FUNCTION_END_MODIFY(StreamEOFError, CodecCorruptedFileError);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the default planar configuration
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool jpegImageCodec::defaultInterleaved() const
{
    return true;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Copy the loaded image into a class image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<image> jpegImageCodec::copyJpegChannelsToImage(
        jpeg::jpegInformation& information,
        bool b2complement,
        const std::string& colorSpace) const
{
    IMEBRA_FUNCTION_START();

    bitDepth_t depth;
    if(b2complement)
        depth = (information.m_precision==8) ? bitDepth_t::depthS8 : bitDepth_t::depthS16;
    else
        depth = (information.m_precision==8) ? bitDepth_t::depthU8 : bitDepth_t::depthU16;

    std::shared_ptr<image> destImage(std::make_shared<image>(information.m_imageWidth, information.m_imageHeight, depth, colorSpace, (std::uint8_t)(information.m_precision-1)));

    std::shared_ptr<handlers::writingDataHandlerNumericBase> handler = destImage->getWritingDataHandler();

    std::int32_t offsetValue=(std::int32_t)1 << (information.m_precision-1);
    std::int32_t maxClipValue=((std::int32_t)1 << information.m_precision)-1;
    std::int32_t minClipValue = 0;
    if(b2complement)
    {
        maxClipValue-=offsetValue;
        minClipValue-=offsetValue;
    }

    // Copy the jpeg channels into the new image
    ///////////////////////////////////////////////////////////
    std::uint32_t destChannelNumber = 0;
    for(jpeg::jpegInformation::tChannelsMap::const_iterator copyChannelsIterator = information.m_channelsMap.begin();
        copyChannelsIterator != information.m_channelsMap.end();
        ++copyChannelsIterator)
    {
        std::shared_ptr<jpeg::jpegChannel> pChannel = copyChannelsIterator->second;

        // Adjust 2complement
        ///////////////////////////////////////////////////////////
        std::int32_t* pChannelBuffer = pChannel->m_pBuffer;
        if(!information.m_bLossless && !b2complement)
        {
            for(std::uint32_t adjust2complement = pChannel->m_bufferSize; adjust2complement != 0; --adjust2complement, ++pChannelBuffer)
            {
                *pChannelBuffer += offsetValue;
                if(*pChannelBuffer < minClipValue)
                {
                    *pChannelBuffer = minClipValue;
                }
                else if(*pChannelBuffer > maxClipValue)
                {
                    *pChannelBuffer = maxClipValue;
                }
            }
        }
        else if(information.m_bLossless && b2complement)
        {
            for(std::uint32_t adjust2complement = pChannel->m_bufferSize; adjust2complement; --adjust2complement)
            {
                if(*pChannelBuffer & offsetValue)
                {
                    *pChannelBuffer |= (std::int32_t)((std::int32_t)-1 * ((std::int32_t)1 << information.m_precision));
                }
                if(*pChannelBuffer < minClipValue)
                {
                    *pChannelBuffer = minClipValue;
                }
                else if(*pChannelBuffer > maxClipValue)
                {
                    *pChannelBuffer = maxClipValue;
                }
                ++pChannelBuffer;
            }
        }

        // If only one channel is present, then use the fast copy
        ///////////////////////////////////////////////////////////
        if(information.m_bLossless && information.m_channelsMap.size() == 1)
        {
            handler->copyFrom(pChannel->m_pBuffer, pChannel->m_bufferSize);
            return destImage;
        }

        // Lossless interleaved
        ///////////////////////////////////////////////////////////
        std::uint32_t runX = information.m_maxSamplingFactorX / pChannel->m_samplingFactorX;
        std::uint32_t runY = information.m_maxSamplingFactorY / pChannel->m_samplingFactorY;
        if(information.m_bLossless)
        {
            handler->copyFromInt32Interleaved(
                        pChannel->m_pBuffer,
                        runX, runY,
                        0, 0, pChannel->m_width * runX, pChannel->m_height * runY,
                        destChannelNumber++,
                        information.m_imageWidth, information.m_imageHeight,
                        (std::uint32_t)information.m_channelsMap.size());

            continue;
        }

        // Lossy interleaved
        ///////////////////////////////////////////////////////////
        std::uint32_t totalBlocksY(pChannel->m_height >> 3);
        std::uint32_t totalBlocksX(pChannel->m_width >> 3);

        std::int32_t* pSourceBuffer(pChannel->m_pBuffer);

        std::uint32_t startRow(0);
        for(std::uint32_t scanBlockY = 0; scanBlockY < totalBlocksY; ++scanBlockY)
        {
            std::uint32_t startCol(0);
            std::uint32_t endRow(startRow + (runY << 3));

            for(std::uint32_t scanBlockX = 0; scanBlockX < totalBlocksX; ++scanBlockX)
            {
                std::uint32_t endCol = startCol + (runX << 3);
                handler->copyFromInt32Interleaved(
                            pSourceBuffer,
                            runX, runY,
                            startCol,
                            startRow,
                            endCol,
                            endRow,
                            destChannelNumber,
                            information.m_imageWidth, information.m_imageHeight,
                            (std::uint32_t)information.m_channelsMap.size());

                pSourceBuffer += 64;
                startCol = endCol;
            }
            startRow = endRow;
        }
        ++destChannelNumber;
    }

    return destImage;

    IMEBRA_FUNCTION_END();
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Copy an image into the internal channels
//
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
void jpegImageCodec::copyImageToJpegChannels(
        jpeg::jpegInformation& information,
        std::shared_ptr<const image> sourceImage,
        bool b2complement,
        std::uint32_t allocatedBits,
        bool bSubSampledX,
        bool bSubSampledY) const
{
    IMEBRA_FUNCTION_START();

    std::string colorSpace = sourceImage->getColorSpace();
    sourceImage->getSize(&information.m_imageWidth, &information.m_imageHeight);
    information.m_precision = allocatedBits;

    // Create the channels
    ////////////////////////////////////////////////////////////////
    std::uint32_t channelsNumber(sourceImage->getChannelsNumber());
    std::shared_ptr<handlers::readingDataHandlerNumericBase>imageDataHandler = sourceImage->getReadingDataHandler();

    for(std::uint8_t channelId = 0; channelId < (std::uint8_t)channelsNumber; ++channelId)
    {
        std::shared_ptr<jpeg::jpegChannel> pChannel(std::make_shared<jpeg::jpegChannel>());
        information.m_channelsMap[channelId] = pChannel;

        pChannel->m_huffmanTableAC = 0;
        pChannel->m_pActiveHuffmanTableAC = information.m_pHuffmanTableAC[0].get();
        pChannel->m_huffmanTableDC = 0;
        pChannel->m_pActiveHuffmanTableDC = information.m_pHuffmanTableDC[0].get();

        if(channelId == 0)
        {
            if(bSubSampledX)
            {
                ++(pChannel->m_samplingFactorX);
            }
            if(bSubSampledY)
            {
                ++(pChannel->m_samplingFactorY);
            }
            continue;
        }
        if(colorSpace != "YBR_FULL" && colorSpace != "YBR_PARTIAL")
        {
            continue;
        }
        pChannel->m_quantTable = 1;
        pChannel->m_huffmanTableDC = 1;
        pChannel->m_huffmanTableAC = 1;
        pChannel->m_pActiveHuffmanTableAC = information.m_pHuffmanTableAC[1].get();
        pChannel->m_pActiveHuffmanTableDC = information.m_pHuffmanTableDC[1].get();
    }
    information.allocChannels();

    std::int32_t offsetValue=(std::int32_t)1 << (information.m_precision-1);
    std::int32_t maxClipValue=((std::int32_t)1 << information.m_precision)-1;
    std::int32_t minClipValue = 0;
    if(b2complement)
    {
        maxClipValue-=offsetValue;
        minClipValue-=offsetValue;
    }

    // Copy the image into the jpeg channels
    ///////////////////////////////////////////////////////////
    std::uint32_t sourceChannelNumber = 0;
    for(jpeg::jpegInformation::tChannelsMap::iterator copyChannelsIterator = information.m_channelsMap.begin();
        copyChannelsIterator != information.m_channelsMap.end();
        ++copyChannelsIterator)
    {
        std::shared_ptr<jpeg::jpegChannel> pChannel = copyChannelsIterator->second;

        // If only one channel is present, then use the fast copy
        ///////////////////////////////////////////////////////////
        if(information.m_bLossless && information.m_channelsMap.size() == 1)
        {
            imageDataHandler->copyTo(pChannel->m_pBuffer, pChannel->m_bufferSize);
            continue;
        }

        // Lossless interleaved
        ///////////////////////////////////////////////////////////
        std::uint32_t runX = information.m_maxSamplingFactorX / pChannel->m_samplingFactorX;
        std::uint32_t runY = information.m_maxSamplingFactorY / pChannel->m_samplingFactorY;
        if(information.m_bLossless)
        {
            imageDataHandler->copyToInt32Interleaved(
                        pChannel->m_pBuffer,
                        runX, runY,
                        0, 0, pChannel->m_width * runX, pChannel->m_height * runY,
                        sourceChannelNumber++,
                        information.m_imageWidth, information.m_imageHeight,
                        (std::uint32_t)information.m_channelsMap.size());

            continue;
        }

        // Lossy interleaved
        ///////////////////////////////////////////////////////////
        std::uint32_t totalBlocksY = (pChannel->m_height >> 3);
        std::uint32_t totalBlocksX = (pChannel->m_width >> 3);

        std::int32_t* pDestBuffer = pChannel->m_pBuffer;

        std::uint32_t startRow = 0;
        for(std::uint32_t scanBlockY = 0; scanBlockY < totalBlocksY; ++scanBlockY)
        {
            std::uint32_t startCol = 0;
            std::uint32_t endRow = startRow + (runY << 3);

            for(std::uint32_t scanBlockX = 0; scanBlockX < totalBlocksX; ++scanBlockX)
            {
                std::uint32_t endCol = startCol + (runX << 3);
                imageDataHandler->copyToInt32Interleaved(
                            pDestBuffer,
                            runX, runY,
                            startCol,
                            startRow,
                            endCol,
                            endRow,
                            sourceChannelNumber,
                            information.m_imageWidth, information.m_imageHeight,
                            (std::uint32_t)information.m_channelsMap.size());

                pDestBuffer += 64;
                startCol = endCol;
            }
            startRow = endRow;
        }
        ++sourceChannelNumber;
    }


    for(jpeg::jpegInformation::tChannelsMap::iterator clipChannelsIterator = information.m_channelsMap.begin();
        clipChannelsIterator != information.m_channelsMap.end();
        ++clipChannelsIterator)
    {
        std::shared_ptr<jpeg::jpegChannel> pChannel = clipChannelsIterator->second;

        // Clip the values
        ///////////////////////////////////////////////////////////
        std::int32_t* pChannelBuffer = pChannel->m_pBuffer;
        for(std::uint32_t clipValues = pChannel->m_bufferSize; clipValues; --clipValues)
        {
            if(*pChannelBuffer < minClipValue)
            {
                *pChannelBuffer = minClipValue;
            }
            if(*pChannelBuffer > maxClipValue)
            {
                *pChannelBuffer = maxClipValue;
            }
            ++pChannelBuffer;
        }

        // Adjust 2complement
        ///////////////////////////////////////////////////////////
        if(!information.m_bLossless && !b2complement)
        {
            pChannelBuffer = pChannel->m_pBuffer;
            for(std::uint32_t adjust2complement = pChannel->m_bufferSize; adjust2complement; --adjust2complement)
            {
                *(pChannelBuffer++) -= offsetValue;
            }
        }

        pChannelBuffer = pChannel->m_pBuffer;
        std::int32_t orValue   = (std::int32_t)((std::int32_t)-1 * ((std::int32_t)1 << information.m_precision));
        for(std::uint32_t adjustHighBits = pChannel->m_bufferSize; adjustHighBits != 0; --adjustHighBits)
        {
            if((*pChannelBuffer & offsetValue) != 0)
            {
                *pChannelBuffer |= orValue;
            }
            ++pChannelBuffer;
        }
    }

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write an image into the dataset
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void jpegImageCodec::setImage(
        std::shared_ptr<streamWriter> pDestStream,
        std::shared_ptr<const image> pImage,
        const std::string& transferSyntax,
        imageQuality_t imageQuality,
        std::uint32_t allocatedBits,
        bool bSubSampledX,
        bool bSubSampledY,
        bool bInterleaved,
        bool b2Complement) const
{
    IMEBRA_FUNCTION_START();

    streamWriter* pDestinationStream = pDestStream.get();

    // Activate the tags in the stream
    ///////////////////////////////////////////////////////////
    pDestinationStream->m_bJpegTags = true;

    // Reset the internal variables
    ////////////////////////////////////////////////////////////////
    jpeg::jpegInformation information;
    information.reset(imageQuality);

    information.m_bLossless = transferSyntax == "1.2.840.10008.1.2.4.57" ||  // lossless NH
            transferSyntax == "1.2.840.10008.1.2.4.70";    // lossless NH first order prediction

    copyImageToJpegChannels(information, pImage, b2Complement, allocatedBits, bSubSampledX, bSubSampledY);

    // Now write the jpeg stream
    ////////////////////////////////////////////////////////////////
    const std::uint8_t checkSignature[2]={(std::uint8_t)0xff, (std::uint8_t)0xd8};
    pDestinationStream->write(checkSignature, 2);

    // Write the SOF tag
    ////////////////////////////////////////////////////////////////
    writeTag(pDestinationStream, information.m_bLossless ? sof3 : (information.m_precision <= 8 ? sof0 : sof1), information);

    // Write the quantization tables
    ////////////////////////////////////////////////////////////////
    writeTag(pDestinationStream, dqt, information);

    for(int phase = 0; phase < 2; ++phase)
    {
        if(phase == 1)
        {
            // Write the huffman tables
            ////////////////////////////////////////////////////////////////
            writeTag(pDestinationStream, dht, information);
        }

        // Write the scans
        ////////////////////////////////////////////////////////////////
        information.m_channelsList.clear();

        if(bInterleaved)
        {
            for(jpeg::jpegInformation::tChannelsMap::iterator channelsIterator = information.m_channelsMap.begin();
                channelsIterator != information.m_channelsMap.end();
                ++channelsIterator)
            {
                information.m_channelsList.push_back(channelsIterator->second);
            }
            writeScan(pDestinationStream, information, phase == 0);
        }
        else
        {
            for(jpeg::jpegInformation::tChannelsMap::iterator channelsIterator = information.m_channelsMap.begin();
                channelsIterator != information.m_channelsMap.end();
                ++channelsIterator)
            {
                information.m_channelsList.clear();
                information.m_channelsList.push_back(channelsIterator->second);
                writeScan(pDestinationStream, information, phase == 0);
            }
        }
    }

    writeTag(pDestinationStream, eoi, information);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write a single scan (SOS tag + channels)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegImageCodec::writeScan(streamWriter* pDestinationStream, jpeg::jpegInformation& information, bool bCalcHuffman) const
{
    IMEBRA_FUNCTION_START();

    information.findMcuSize();

    if(information.m_bLossless)
    {
        information.m_spectralIndexStart = 1;
        information.m_spectralIndexEnd = 0;
    }
    if(!bCalcHuffman)
    {
        writeTag(pDestinationStream, sos, information);
    }

    while(information.m_mcuProcessed < information.m_mcuNumberTotal)
    {
        // Write an MCU
        ///////////////////////////////////////////////////////////

        // Scan all components
        ///////////////////////////////////////////////////////////
        for(const std::shared_ptr<jpeg::jpegChannel>& pChannel: information.m_channelsList)
        {
            // Write a lossless pixel
            ///////////////////////////////////////////////////////////
            if(information.m_bLossless)
            {
                std::int32_t lastValue = pChannel->m_lastDCValue;
                std::int32_t* pBuffer = pChannel->m_pBuffer + pChannel->m_losslessPositionY * pChannel->m_width + pChannel->m_losslessPositionX;

                for(std::uint32_t scanBlock = pChannel->m_blockMcuXY; scanBlock != 0; --scanBlock)
                {
                    std::int32_t value(*pBuffer);
                    if(pChannel->m_losslessPositionX == 0 && pChannel->m_losslessPositionY != 0)
                    {
                        lastValue = *(pBuffer - pChannel->m_width);
                    }
                    ++pBuffer;
                    std::int32_t diff = value - lastValue;
                    std::int32_t diff1 = value + ((std::int32_t)1 << information.m_precision) - lastValue;
                    std::int32_t diff2 = value - ((std::int32_t)1 << information.m_precision) - lastValue;
                    if(labs(diff1) < labs(diff))
                    {
                        diff = diff1;
                    }
                    if(labs(diff2) < labs(diff))
                    {
                        diff = diff2;
                    }

                    // Calculate amplitude and build the huffman table
                    std::uint32_t amplitudeLength = 0;
                    std::uint32_t amplitude = 0;
                    if(diff != 0)
                    {
                        amplitude = (diff > 0) ? (std::uint32_t)(diff) : (std::uint32_t)(-diff);
                        for(amplitudeLength = 32; (amplitude & ((std::uint32_t)1 << (amplitudeLength -1))) == 0; --amplitudeLength){};

                        if(diff < 0)
                        {
                            amplitude = (std::uint32_t) (((std::int32_t)1 << amplitudeLength) + diff - 1);
                        }

                    }

                    pChannel->m_lastDCValue = value;
                    if(++(pChannel->m_losslessPositionX) == pChannel->m_width)
                    {
                        ++(pChannel->m_losslessPositionY);
                        pChannel->m_losslessPositionX = 0;
                    }

                    if(bCalcHuffman)
                    {
                        pChannel->m_pActiveHuffmanTableDC->incValueFreq(amplitudeLength);
                        continue;
                    }
                    pChannel->m_pActiveHuffmanTableDC->writeHuffmanCode(amplitudeLength, pDestinationStream);
                    if(amplitudeLength != 16)
                    {
                        pDestinationStream->writeBits(amplitude, amplitudeLength);
                    }
                }

                continue;
            }

            // write a lossy MCU
            ///////////////////////////////////////////////////////////
            std::uint32_t bufferPointer =
                    (information.m_mcuProcessedY * pChannel->m_blockMcuY *
                     ((information.m_jpegImageWidth * pChannel->m_samplingFactorX / information.m_maxSamplingFactorX) >> 3) +
                     information.m_mcuProcessedX * pChannel->m_blockMcuX) * 64;

            for(std::uint32_t scanBlockY = 0; scanBlockY != pChannel->m_blockMcuY; ++scanBlockY)
            {
                for(std::uint32_t scanBlockX = 0; scanBlockX != pChannel->m_blockMcuX; ++scanBlockX)
                {
                    writeBlock(pDestinationStream, information, &(pChannel->m_pBuffer[bufferPointer]), pChannel, bCalcHuffman);
                    bufferPointer += 64;
                }
                bufferPointer += (information.m_mcuNumberX -1) * pChannel->m_blockMcuX * 64;
            }
        }

        ++information.m_mcuProcessed;
        if(++information.m_mcuProcessedX == information.m_mcuNumberX)
        {
            information.m_mcuProcessedX = 0;
            ++information.m_mcuProcessedY;
        }
    }

    if(!bCalcHuffman)
    {
        pDestinationStream->resetOutBitsBuffer();
    }

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read a single MCU's block.
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
inline void jpegImageCodec::readBlock(jpegStreamReader& stream, jpeg::jpegInformation& information, std::int32_t* pBuffer, const std::shared_ptr<jpeg::jpegChannel>& pChannel) const
{
    IMEBRA_FUNCTION_START();

    // Scan all the requested spectral values
    /////////////////////////////////////////////////////////////////
    std::uint32_t spectralIndex(information.m_spectralIndexStart);

    // If an eob-run is defined, then don't read the DC value
    /////////////////////////////////////////////////////////////////
    if(information.m_eobRun && spectralIndex == 0)
    {
        ++spectralIndex;
    }

    std::int32_t value = 0;
    std::int32_t oldValue;

    // Scan the specified spectral values
    /////////////////////////////////////////////////////////////////
    for(; spectralIndex <= information.m_spectralIndexEnd; ++spectralIndex)
    {
        // Read AC progressive bits for non-zero coefficients
        /////////////////////////////////////////////////////////////////
        if(information.m_eobRun != 0)
        {
            break;
        }

        //
        // AC/DC pass
        //
        /////////////////////////////////////////////////////////////////
        std::uint32_t hufCode;
        if(spectralIndex != 0)
        {
            hufCode = pChannel->m_pActiveHuffmanTableAC->readHuffmanCode(stream);

            // End of block reached
            /////////////////////////////////////////////////////////////////
            if(hufCode == 0)
            {
                ++information.m_eobRun;
                --spectralIndex;
                continue;
            }
        }
        else
        {
            hufCode = pChannel->m_pActiveHuffmanTableDC->readHuffmanCode(stream);
        }


        //
        // Get AC or DC amplitude or zero run
        //
        /////////////////////////////////////////////////////////////////

        // Find bit coded coeff. amplitude
        /////////////////////////////////////////////////////////////////
        std::uint8_t amplitudeLength = (std::uint8_t)(hufCode & 0xf);

        // Find zero run length
        /////////////////////////////////////////////////////////////////
        std::uint32_t runLength = (std::uint32_t)(hufCode>>4);

        // First DC or AC pass but not EOB run
        /////////////////////////////////////////////////////////////////
        if(spectralIndex == 0 || amplitudeLength != 0 || runLength == 0xf)
        {
            // Read coeff
            /////////////////////////////////////////////////////////////////
            if(amplitudeLength != 0)
            {
                value = (std::int32_t)(stream.readBits(amplitudeLength));
                if(value < ((std::int32_t)1 << (amplitudeLength-1)) )
                {
                    value -= ((std::int32_t)1 << amplitudeLength) - 1;
                }
            }
            else
            {
                value = 0;
            }

            spectralIndex += runLength;

            // Store coeff.
            /////////////////////////////////////////////////////////////////
            if(spectralIndex <= information.m_spectralIndexEnd)
            {
                oldValue = value;

                // DC coeff added to the previous value.
                /////////////////////////////////////////////////////////////////
                if(spectralIndex == 0)
                {
                    oldValue += pChannel->m_lastDCValue;
                    pChannel->m_lastDCValue=oldValue;
                }
                pBuffer[JpegDeZigZagOrder[spectralIndex]]=oldValue;
            }
        } // ----- End of first DC or AC pass or refine AC pass but not EOB run

        // EOB run found
        /////////////////////////////////////////////////////////////////
        else
        {
            information.m_eobRun += (std::uint32_t)1 << runLength;
            information.m_eobRun += stream.readBits(runLength);
            --spectralIndex;
        }
    }

    //
    // EOB run processor
    //
    /////////////////////////////////////////////////////////////////
    if(information.m_eobRun != 0)
        information.m_eobRun--;

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write a single MCU's block.
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
inline void jpegImageCodec::writeBlock(streamWriter* pStream, jpeg::jpegInformation& information, std::int32_t* pBuffer, const std::shared_ptr<jpeg::jpegChannel>& pChannel, bool bCalcHuffman) const
{
    IMEBRA_FUNCTION_START();

    if(bCalcHuffman)
    {
        FDCT(pBuffer, information.m_compressionQuantizationTable[pChannel->m_quantTable]);
    }

    // Scan the specified spectral values
    /////////////////////////////////////////////////////////////////
    std::uint32_t zeroRun = 0;
    std::int32_t value;
    const std::uint32_t* pJpegDeZigZagOrder(&(JpegDeZigZagOrder[information.m_spectralIndexStart]));
    huffmanTable* pActiveHuffmanTable;

    for(std::uint32_t spectralIndex = information.m_spectralIndexStart; spectralIndex <= information.m_spectralIndexEnd; ++spectralIndex)
    {
        value = pBuffer[*(pJpegDeZigZagOrder++)];

        if(value > 32767)
        {
            value = 32767;
        }
        else if(value < -32767)
        {
            value = -32767;
        }
        if(spectralIndex == 0)
        {
            value -= pChannel->m_lastDCValue;
            pChannel->m_lastDCValue += value;
            pActiveHuffmanTable = pChannel->m_pActiveHuffmanTableDC;
        }
        else
        {
            pActiveHuffmanTable = pChannel->m_pActiveHuffmanTableAC;
            if(value == 0)
            {
                ++zeroRun;
                continue;
            }
        }

        //Write out the zero runs
        /////////////////////////////////////////////////////////////////
        while(zeroRun >= 16)
        {
            zeroRun -= 16;
            const std::uint32_t zeroRunCode = 0xf0;
            if(bCalcHuffman)
            {
                pActiveHuffmanTable->incValueFreq(zeroRunCode);
                continue;
            }
            pActiveHuffmanTable->writeHuffmanCode(zeroRunCode, pStream);
        }

        std::uint32_t hufCode = (zeroRun << 4);
        zeroRun = 0;

        // Write out the value
        /////////////////////////////////////////////////////////////////
        std::uint32_t amplitudeLength = 0;
        std::uint32_t amplitude = 0;
        if(value != 0)
        {
            amplitude = (value > 0) ? (std::uint32_t)value : (std::uint32_t)(-value);
            for(amplitudeLength = 15; (amplitude & ((std::uint32_t)1 << (amplitudeLength -1))) == 0; --amplitudeLength){};

            if(value < 0)
            {
                amplitude = (std::uint32_t) (((std::int32_t)1 << amplitudeLength) + value -1);
            }
            hufCode |= amplitudeLength;
        }

        if(bCalcHuffman)
        {
            pActiveHuffmanTable->incValueFreq(hufCode);
            continue;
        }
        pActiveHuffmanTable->writeHuffmanCode(hufCode, pStream);
        if(amplitudeLength != 0)
        {
            pStream->writeBits(amplitude, amplitudeLength);
        }
    }

    if(zeroRun == 0)
    {
        return;
    }

    const std::uint32_t zero(0);
    if(bCalcHuffman)
    {
        pChannel->m_pActiveHuffmanTableAC->incValueFreq(zero);
        return;
    }
    pChannel->m_pActiveHuffmanTableAC->writeHuffmanCode(zero, pStream);

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Calc FDCT on MCU
// This routine comes from the IJG software version 6b
//
// Values must be Zero centered (-x...0...+x)
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void jpegImageCodec::FDCT(std::int32_t* pIOMatrix, float* pDescaleFactors) const
{
    // Temporary values
    /////////////////////////////////////////////////////////////////
    float tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    float tmp10, tmp11, tmp12, tmp13;
    float z1, z2, z3, z4, z5, z11, z13;

    float fdctTempMatrix[64];


    // Rows FDCT
    /////////////////////////////////////////////////////////////////
    std::int32_t *pointerOperator0, *pointerOperator1;
    for(int scanBlockY=0; scanBlockY<64; scanBlockY+=8)
    {
        pointerOperator0 = &(pIOMatrix[scanBlockY]);
        pointerOperator1 = &(pIOMatrix[scanBlockY + 7]);
        tmp0 = (float)(*pointerOperator0 + *pointerOperator1);
        tmp7 = (float)(*pointerOperator0 - *pointerOperator1);

        tmp1 = (float)(*(++pointerOperator0) + *(--pointerOperator1));
        tmp6 = (float)(*pointerOperator0 - *pointerOperator1);

        tmp2 = (float)(*(++pointerOperator0) + *(--pointerOperator1));
        tmp5 = (float)(*pointerOperator0 - *pointerOperator1);

        tmp3 = (float)(*(++pointerOperator0) + *(--pointerOperator1));
        tmp4 = (float)(*pointerOperator0 - *pointerOperator1);

        // Phase 2
        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        // Phase 3
        fdctTempMatrix[scanBlockY]   = tmp10 + tmp11;
        fdctTempMatrix[scanBlockY+4] = tmp10 - tmp11;

        z1 = (tmp12 + tmp13)*0.707106781f;     // c4

        // Phase 5
        fdctTempMatrix[scanBlockY+2] = tmp13 + z1;
        fdctTempMatrix[scanBlockY+6] = tmp13 - z1;

        // Odd part
        // Phase 2
        tmp10 = tmp4 + tmp5;
        tmp11 = tmp5 + tmp6;
        tmp12 = tmp6 + tmp7;

        // The rotator is modified from fig 4-8 to avoid extra negations.
        z5 =  (tmp10 - tmp12)*0.382683433f;    // c6
        z2 =  tmp10*0.541196100f + z5;         // c2-c6
        z4 =  tmp12*1.306562965f + z5;         // c2+c6
        z3 =  tmp11*0.707106781f;              // c4

        // Phase 5
        z11 = tmp7 + z3;
        z13 = tmp7 - z3;

        // Phase 6
        fdctTempMatrix[scanBlockY+5] = z13 + z2;
        fdctTempMatrix[scanBlockY+3] = z13 - z2;
        fdctTempMatrix[scanBlockY+1] = z11 + z4;
        fdctTempMatrix[scanBlockY+7] = z11 - z4;
    }

    // Columns FDCT
    /////////////////////////////////////////////////////////////////
    float *pointerOperatorFloat0, *pointerOperatorFloat1;
    for(int scanBlockX = 0; scanBlockX < 8; ++scanBlockX)
    {
        pointerOperatorFloat0 = &(fdctTempMatrix[scanBlockX]);
        pointerOperatorFloat1 = &(fdctTempMatrix[scanBlockX + 56]);

        tmp0 = *pointerOperatorFloat0 + *pointerOperatorFloat1;
        tmp7 = *pointerOperatorFloat0 - *pointerOperatorFloat1;

        pointerOperatorFloat0 += 8;
        pointerOperatorFloat1 -= 8;
        tmp1 = *pointerOperatorFloat0 + *pointerOperatorFloat1;
        tmp6 = *pointerOperatorFloat0 - *pointerOperatorFloat1;

        pointerOperatorFloat0 += 8;
        pointerOperatorFloat1 -= 8;
        tmp2 = *pointerOperatorFloat0 + *pointerOperatorFloat1;
        tmp5 = *pointerOperatorFloat0 - *pointerOperatorFloat1;

        pointerOperatorFloat0 += 8;
        pointerOperatorFloat1 -= 8;
        tmp3 = *pointerOperatorFloat0 + *pointerOperatorFloat1;
        tmp4 = *pointerOperatorFloat0 - *pointerOperatorFloat1;

        // Even part
        // Phase 2
        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        // Phase 3
        fdctTempMatrix[scanBlockX   ] = tmp10 + tmp11;
        fdctTempMatrix[scanBlockX+32] = tmp10 - tmp11;

        z1 = (tmp12 + tmp13)*0.707106781f;     // c4

        // Phase 5
        fdctTempMatrix[scanBlockX+16] = (tmp13 + z1);
        fdctTempMatrix[scanBlockX+48] = (tmp13 - z1);

        // Odd part
        // Phase 2
        tmp10 = tmp4 + tmp5;
        tmp11 = tmp5 + tmp6;
        tmp12 = tmp6 + tmp7;

        // Avoid extra negations.
        z5 =  (tmp10 - tmp12)*0.382683433f;    // c6
        z2 =  tmp10*0.541196100f + z5;         // c2-c6
        z4 =  tmp12*1.306562965f + z5;         // c2+c6
        z3 =  tmp11*0.707106781f;              // c4

        // Phase 5
        z11 = tmp7 + z3;
        z13 = tmp7 - z3;

        // Phase 6
        fdctTempMatrix[scanBlockX+40] = (z13 + z2);
        fdctTempMatrix[scanBlockX+24] = (z13 - z2);
        fdctTempMatrix[scanBlockX+ 8] = (z11 + z4);
        fdctTempMatrix[scanBlockX+56] = (z11 - z4);
    }

    // Descale FDCT results
    /////////////////////////////////////////////////////////////////
    for(int descale = 0; descale < 64; ++descale)
        pIOMatrix[descale]=(std::int32_t)(fdctTempMatrix[descale]*pDescaleFactors[descale]+.5f);
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Calc IDCT on MCU
// This routine comes from the IJG software version 6b
//
// Values must be Zero centered (-x...0...+x)
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void jpegImageCodec::IDCT(std::int32_t* pIOMatrix, long long* pScaleFactors) const
{
    const double multiplier((float)((long long)1 << JPEG_DECOMPRESSION_BITS_PRECISION));
    const long long multiplier_1_414213562f((long long)(multiplier * 1.414213562f + .5f));
    const long long multiplier_1_847759065f((long long)(multiplier * 1.847759065f + .5f));
    const long long multiplier_1_0823922f((long long)(multiplier * 1.0823922f + .5f));
    const long long multiplier_2_61312593f((long long)(multiplier * 2.61312593f + .5f));
    const long long zero_point_five((long long)1 << (JPEG_DECOMPRESSION_BITS_PRECISION - 1));
    const long long zero_point_five_by_8((std::int32_t)zero_point_five << 3);


    // Temporary values
    /////////////////////////////////////////////////////////////////
    long long tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    long long tmp10, tmp11, tmp12, tmp13;
    long long z5, z10, z11, z12, z13;

    //
    // Rows IDCT
    //
    /////////////////////////////////////////////////////////////////
    std::int32_t* pMatrix(pIOMatrix);
    std::int32_t* pCheckMatrix(pIOMatrix);

    // temporary matrix used by IDCT
    ///////////////////////////////////////////////////////////
    long long idctTempMatrix[64];

    std::int32_t checkZero;
    long long* pTempMatrix(idctTempMatrix);

    for(int scanBlockY(8); scanBlockY != 0; --scanBlockY)
    {
        checkZero = *(++pCheckMatrix);
        checkZero |= *(++pCheckMatrix);
        checkZero |= *(++pCheckMatrix);
        checkZero |= *(++pCheckMatrix);
        checkZero |= *(++pCheckMatrix);
        checkZero |= *(++pCheckMatrix);
        checkZero |= *(++pCheckMatrix);
        ++pCheckMatrix; // Point pCheckMatrix to the next row

        // Check for AC coefficients value.
        // If they are all NULL, then apply the DC value to all
        /////////////////////////////////////////////////////////////////
        if(checkZero == 0)
        {
            tmp0 = (long long)(*pMatrix) * (*pScaleFactors);
            *(pTempMatrix++) = tmp0;
            *(pTempMatrix++) = tmp0;
            *(pTempMatrix++) = tmp0;
            *(pTempMatrix++) = tmp0;
            *(pTempMatrix++) = tmp0;
            *(pTempMatrix++) = tmp0;
            *(pTempMatrix++) = tmp0;
            *(pTempMatrix++) = tmp0;
            pMatrix = pCheckMatrix;
            pScaleFactors += 8;
            continue;
        }

        tmp0 = (long long)*pMatrix++ * (*pScaleFactors++);
        tmp4 = (long long)*pMatrix++ * (*pScaleFactors++);
        tmp1 = (long long)*pMatrix++ * (*pScaleFactors++);
        tmp5 = (long long)*pMatrix++ * (*pScaleFactors++);
        tmp2 = (long long)*pMatrix++ * (*pScaleFactors++);
        tmp6 = (long long)*pMatrix++ * (*pScaleFactors++);
        tmp3 = (long long)*pMatrix++ * (*pScaleFactors++);
        tmp7 = (long long)*pMatrix++ * (*pScaleFactors++);

        // Phase 3
        tmp10 = tmp0 + tmp2;
        tmp11 = tmp0 - tmp2;

        // Phases 5-3
        tmp13 = tmp1 + tmp3;
        tmp12 = (((tmp1 - tmp3) * multiplier_1_414213562f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION) - tmp13; // 2*c4

        // Phase 2
        tmp0 = tmp10 + tmp13;
        tmp3 = tmp10 - tmp13;
        tmp1 = tmp11 + tmp12;
        tmp2 = tmp11 - tmp12;

        // Phase 6
        z13 = tmp6 + tmp5;
        z10 = tmp6 - tmp5;
        z11 = tmp4 + tmp7;
        z12 = tmp4 - tmp7;

        // Phase 5
        tmp7 = z11 + z13;
        z5 = ((z10 + z12) * multiplier_1_847759065f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION;    // 2*c2

        // Phase 2
        tmp6 = z5 - ((z10 *multiplier_2_61312593f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION) - tmp7;
        tmp5 = (((z11 - z13) * multiplier_1_414213562f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION) - tmp6;
        tmp4 = ((z12 * multiplier_1_0823922f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION) - z5 + tmp5;

        *(pTempMatrix++) = tmp0 + tmp7;
        *(pTempMatrix++) = tmp1 + tmp6;
        *(pTempMatrix++) = tmp2 + tmp5;
        *(pTempMatrix++) = tmp3 - tmp4;
        *(pTempMatrix++) = tmp3 + tmp4;
        *(pTempMatrix++) = tmp2 - tmp5;
        *(pTempMatrix++) = tmp1 - tmp6;
        *(pTempMatrix++) = tmp0 - tmp7;
    }

    //
    // Columns IDCT
    //
    /////////////////////////////////////////////////////////////////
    pMatrix = pIOMatrix;
    pTempMatrix = idctTempMatrix;
    for(int scanBlockX(8); scanBlockX != 0; --scanBlockX)
    {
        tmp0 = *pTempMatrix;
        pTempMatrix += 8;
        tmp4 = *pTempMatrix;
        pTempMatrix += 8;
        tmp1 = *pTempMatrix;
        pTempMatrix += 8;
        tmp5 = *pTempMatrix;
        pTempMatrix += 8;
        tmp2 = *pTempMatrix;
        pTempMatrix += 8;
        tmp6 = *pTempMatrix;
        pTempMatrix += 8;
        tmp3 = *pTempMatrix;
        pTempMatrix += 8;
        tmp7 = *pTempMatrix;
        pTempMatrix -= 55;

        // Phase 3
        tmp10 = tmp0 + tmp2;
        tmp11 = tmp0 - tmp2;

        // Phases 5-3
        tmp13 = tmp1 + tmp3;
        tmp12 = (((tmp1 - tmp3) * multiplier_1_414213562f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION) - tmp13; // 2*c4

        // Phase 2
        tmp0 = tmp10 + tmp13;
        tmp3 = tmp10 - tmp13;
        tmp1 = tmp11 + tmp12;
        tmp2 = tmp11 - tmp12;

        // Phase 6
        z13 = tmp6 + tmp5;
        z10 = tmp6 - tmp5;
        z11 = tmp4 + tmp7;
        z12 = tmp4 - tmp7;

        // Phase 5
        tmp7 = z11 + z13;
        tmp11 = ((z11 - z13) * multiplier_1_414213562f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION; // 2*c4

        z5 = ((z10 + z12) * multiplier_1_847759065f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION;    // 2*c2
        tmp10 = ((z12 * multiplier_1_0823922f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION) - z5;    // 2*(c2-c6)
        tmp12 = z5 - ((z10 *multiplier_2_61312593f + zero_point_five) >> JPEG_DECOMPRESSION_BITS_PRECISION);      // -2*(c2+c6)

        // Phase 2
        tmp6 = tmp12 - tmp7;
        tmp5 = tmp11 - tmp6;
        tmp4 = tmp10 + tmp5;

        // Final output stage: scale down by a factor of 8 (+JPEG_DECOMPRESSION_BITS_PRECISION bits)
        *pMatrix = (std::int32_t)((tmp0 + tmp7 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix += 8;
        *pMatrix = (std::int32_t)((tmp1 + tmp6 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix += 8;
        *pMatrix = (std::int32_t)((tmp2 + tmp5 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix += 8;
        *pMatrix = (std::int32_t)((tmp3 - tmp4 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix += 8;
        *pMatrix = (std::int32_t)((tmp3 + tmp4 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix += 8;
        *pMatrix = (std::int32_t)((tmp2 - tmp5 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix += 8;
        *pMatrix = (std::int32_t)((tmp1 - tmp6 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix += 8;
        *pMatrix = (std::int32_t)((tmp0 - tmp7 + zero_point_five_by_8)>>(JPEG_DECOMPRESSION_BITS_PRECISION + 3));
        pMatrix -= 55;
    }
}

} // namespace codecs

} // namespace implementation

} // namespace imebra



