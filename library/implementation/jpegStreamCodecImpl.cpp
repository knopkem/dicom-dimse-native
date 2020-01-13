/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file jpegStreamCodec.cpp
    \brief Implementation of the class jpegStreamCodec.

*/

#include "exceptionImpl.h"
#include "streamReaderImpl.h"
#include "streamWriterImpl.h"
#include "huffmanTableImpl.h"
#include "jpegStreamCodecImpl.h"
#include "jpegImageCodecImpl.h"
#include "dataSetImpl.h"
#include "codecFactoryImpl.h"
#include "memoryStreamImpl.h"
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

#define JPEG_DECOMPRESSION_BITS_PRECISION 14

imageCodec::~imageCodec()
{}

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

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
jpegStreamCodec::jpegStreamCodec()
{
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write a jpeg stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegStreamCodec::writeStream(std::shared_ptr<streamWriter> pStream, std::shared_ptr<dataSet> pDataSet) const
{
    IMEBRA_FUNCTION_START();

    // Retrieve the transfer syntax
    ///////////////////////////////////////////////////////////
    std::string transferSyntax = pDataSet->getString(0x0002, 0x0, 0x0010, 0, 0);

    // The buffer can be written as it is
    ///////////////////////////////////////////////////////////
    jpegImageCodec imageCodec;
    if(imageCodec.canHandleTransferSyntax(transferSyntax))
    {
        std::shared_ptr<data> imageData = pDataSet->getTag(0x7fe0, 0, 0x0010);

        std::uint32_t firstBufferId(0);
        std::uint32_t endBufferId(1);
        if(imageData->bufferExists(1))
        {
            pDataSet->getFrameBufferIds(0, &firstBufferId, &endBufferId);
        }
        for(std::uint32_t scanBuffers = firstBufferId; scanBuffers != endBufferId; ++scanBuffers)
        {
            std::shared_ptr<handlers::readingDataHandlerRaw> readHandler = imageData->getReadingDataHandlerRaw(scanBuffers);
            const std::uint8_t* readBuffer = readHandler->getMemoryBuffer();
            pStream->write(readBuffer, readHandler->getSize());
        }
        return;

    }

    // Get the image then write it
    ///////////////////////////////////////////////////////////
    std::shared_ptr<image> decodedImage = pDataSet->getImage(0);
    imageCodec.setImage(pStream, decodedImage, "1.2.840.10008.1.2.4.50", imageQuality_t::high, 8, true, true, false, false);

    IMEBRA_FUNCTION_END();

}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Build a DICOM dataset from a jpeg file
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void jpegStreamCodec::readStream(std::shared_ptr<streamReader> pStream, std::shared_ptr<dataSet> pDataSet, std::uint32_t /* maxSizeBufferLoad = 0xffffffff */) const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<memory> jpegData = std::make_shared<memory>();

    std::shared_ptr<memoryStreamOutput> streamOutput = std::make_shared<memoryStreamOutput>(jpegData);

    std::shared_ptr<streamWriter> memoryStreamWriter = std::make_shared<streamWriter>(streamOutput);

    forwardStream forward(pStream, memoryStreamWriter);

    try
    {
        // Read the Jpeg signature
        ///////////////////////////////////////////////////////////
        std::uint8_t jpegSignature[2];
        pStream->read(jpegSignature, 2);

        // If the jpeg signature is wrong, then return an error
        //  condition
        ///////////////////////////////////////////////////////////
        std::uint8_t checkSignature[2]={(std::uint8_t)0xff, (std::uint8_t)0xd8};
        if(::memcmp(jpegSignature, checkSignature, 2) != 0)
        {
            IMEBRA_THROW(CodecWrongFormatError, "detected a wrong format");
        }
    }
    catch(StreamEOFError&)
    {
        IMEBRA_THROW(CodecWrongFormatError, "detected a wrong format");
    }

    // Used to read discharged chars
    ///////////////////////////////////////////////////////////
    std::uint8_t entryByte;

    // Read all the tags in the stream
    ///////////////////////////////////////////////////////////
    jpeg::jpegInformation information;
    while(!information.m_bEndOfImage)
    {
        // If a tag has been found, then parse it
        ///////////////////////////////////////////////////////////
        pStream->read(&entryByte, 1);
        if(entryByte != 0xff)
        {
            continue;
        }
        while(entryByte == 0xff)
        {
            pStream->read(&entryByte, 1);
        }

        if(entryByte != 0)
        {
            std::shared_ptr<jpeg::tag> pTag;
            tTagsMap::const_iterator findTag = m_tagsMap.find(entryByte);
            if(findTag != m_tagsMap.end())
            {
                pTag = findTag->second;
            }
            else
            {
                pTag = m_tagsMap.find(0xff)->second;
            }

            // Parse the tag
            ///////////////////////////////////////////////////////////
            pTag->readTag(*pStream, &information, entryByte);
        }
    }

    //
    // Build the dataset
    //
    ///////////////////////////////////////////////////////////

    // Color space
    ///////////////////////////////////////////////////////////
    if(information.m_channelsMap.size() == 1)
        pDataSet->setString(0x0028, 0, 0x0004, 0, "MONOCHROME2");
    else
        pDataSet->setString(0x0028, 0, 0x0004, 0, "YBR_FULL");

    // Transfer syntax
    ///////////////////////////////////////////////////////////
    switch(information.m_process)
    {
    case 0x00:
        pDataSet->setString(0x0002, 0, 0x0010, 0, "1.2.840.10008.1.2.4.50");
        break;
    case 0x01:
        pDataSet->setString(0x0002, 0, 0x0010, 0, "1.2.840.10008.1.2.4.51");
        break;
    case 0x03:
        pDataSet->setString(0x0002, 0, 0x0010, 0, "1.2.840.10008.1.2.4.57");
        break;
    case 0x07:
        pDataSet->setString(0x0002, 0, 0x0010, 0, "1.2.840.10008.1.2.4.57");
        break;
    default:
        IMEBRA_THROW(JpegCodecCannotHandleSyntaxError, "Jpeg SOF not supported");
    }

    // Number of planes
    ///////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0, 0x0002, 0, (std::uint32_t)information.m_channelsMap.size());

    // Image's width
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0, 0x0011, 0, information.m_imageWidth);

    // Image's height
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0, 0x0010, 0, information.m_imageHeight);

    // Number of frames
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0, 0x0008, 0, 1);

    // Pixel representation
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0x0, 0x0103, 0, 0);

    // Allocated, stored bits and high bit
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0x0, 0x0100, 0, information.m_precision);
    pDataSet->setUnsignedLong(0x0028, 0x0, 0x0101, 0, information.m_precision);
    pDataSet->setUnsignedLong(0x0028, 0x0, 0x0102, 0, information.m_precision - 1);

    // Interleaved (more than 1 channel in the channels list)
    /////////////////////////////////////////////////////////////////
    if(information.m_channelsMap.size() != 1)
    {
        pDataSet->setUnsignedLong(0x0028, 0x0, 0x0006, 0, information.m_channelsList.size() > 1 ? 1 : 0);
    }

    // Insert the basic offset table
    ////////////////////////////////////////////////////////////////
    std::shared_ptr<handlers::writingDataHandlerRaw> offsetHandler=pDataSet->getWritingDataHandlerRaw(0x7fe0, 0, 0x0010, 0, tagVR_t::OB);
    offsetHandler->setSize(4);
    ::memset(offsetHandler->getMemoryBuffer(), 0, offsetHandler->getSize());

    std::shared_ptr<handlers::writingDataHandlerRaw> imageHandler = pDataSet->getWritingDataHandlerRaw(0x7fe0, 0, 0x0010, 1, tagVR_t::OB);
    memoryStreamWriter->flushDataBuffer();
    imageHandler->getMemory()->copyFrom(jpegData);

    IMEBRA_FUNCTION_END();
}


} // namespace codecs

} // namespace implementation

} // namespace imebra



