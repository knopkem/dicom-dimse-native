/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dicomCodec.cpp
    \brief Implementation of the class dicomImageCodec.

*/

#include <list>
#include <vector>
#include <string.h>
#include "exceptionImpl.h"
#include "streamReaderImpl.h"
#include "streamWriterImpl.h"
#include "memoryImpl.h"
#include "dicomNativeImageCodecImpl.h"
#include "dataSetImpl.h"
#include "dicomDictImpl.h"
#include "imageImpl.h"
#include "colorTransformsFactoryImpl.h"
#include "codecFactoryImpl.h"
#include "bufferImpl.h"
#include "../include/imebra/exceptions.h"

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
// dicomCodec
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
// Get a DICOM raw or RLE image from a dicom structure
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
std::shared_ptr<image> dicomNativeImageCodec::getImage(const std::string& /* transferSyntax */,
                                                       const std::string& colorSpace,
                                                       std::uint32_t channelsNumber,
                                                       std::uint32_t imageWidth,
                                                       std::uint32_t imageHeight,
                                                       bool bSubSampledX,
                                                       bool bSubSampledY,
                                                       bool bInterleaved,
                                                       bool b2Complement,
                                                       std::uint8_t allocatedBits,
                                                       std::uint8_t /* storedBits */,
                                                       std::uint8_t highBit,
                                                       std::shared_ptr<streamReader> pSourceStream) const
{
    IMEBRA_FUNCTION_START();

    // Create an image
    ///////////////////////////////////////////////////////////
    bitDepth_t depth;
    if(b2Complement)
    {
        if(highBit >= 16)
        {
            depth = bitDepth_t::depthS32;
        }
        else if(highBit >= 8)
        {
            depth = bitDepth_t::depthS16;
        }
        else
        {
            depth = bitDepth_t::depthS8;
        }
    }
    else
    {
        if(highBit >= 16)
        {
            depth = bitDepth_t::depthU32;
        }
        else if(highBit >= 8)
        {
            depth = bitDepth_t::depthU16;
        }
        else
        {
            depth = bitDepth_t::depthU8;
        }
    }

    std::shared_ptr<image> pImage(std::make_shared<image>(imageWidth, imageHeight, depth, colorSpace, highBit));
    std::uint32_t tempChannelsNumber = pImage->getChannelsNumber();

    if(tempChannelsNumber != channelsNumber)
    {
        IMEBRA_THROW(CodecCorruptedFileError, "Wrong number of channels");
    }

    size_t nativeImageSizeBits = getNativeImageSizeBits(allocatedBits, imageWidth, imageHeight, channelsNumber, bSubSampledX, bSubSampledY);
    {
        std::shared_ptr<handlers::writingDataHandlerNumericBase> imageHandler = pImage->getWritingDataHandler();
        if(!bSubSampledX && !bSubSampledY)
        {
            size_t imageSizeBytes = (nativeImageSizeBits + 7) / 8;
            std::shared_ptr<memory> pMemory = std::make_shared<memory>(imageSizeBytes);
            pSourceStream->read(pMemory->data(), pMemory->size());

            if(allocatedBits != 1)
            {
                if(bInterleaved || channelsNumber == 1)
                {
                    readInterleavedNotSubsampled(
                                imageHandler->getMemoryBuffer(),
                                allocatedBits,
                                pImage->getDepth(),
                                pMemory->data(),
                                imageWidth * imageHeight,
                                channelsNumber
                                );
                }
                else
                {
                    readNotInterleavedNotSubsampled(
                                imageHandler->getMemoryBuffer(),
                                allocatedBits,
                                pImage->getDepth(),
                                pMemory->data(),
                                imageWidth * imageHeight,
                                channelsNumber
                                );
                }
            }
            else
            {
                if(bInterleaved || channelsNumber == 1)
                {
                    read1bitInterleaved(
                                imageHandler->getMemoryBuffer(),
                                pImage->getDepth(),
                                pMemory->data(),
                                imageWidth * imageHeight,
                                channelsNumber
                                );
                }
                else
                {
                    read1bitNotInterleaved(
                                imageHandler->getMemoryBuffer(),
                                pImage->getDepth(),
                                pMemory->data(),
                                imageWidth * imageHeight,
                                channelsNumber
                                );
                }
            }

            // Adjust b2complement buffers
            ///////////////////////////////////////////////////////////
            if(b2Complement)
            {
                adjustB2Complement(imageHandler->getMemoryBuffer(), highBit, depth, imageHandler->getSize());
            }

        }
        else
        {
            std::vector<std::shared_ptr<channel>> subsampledChannels = allocChannels(3, imageWidth, imageHeight, bSubSampledX, bSubSampledY);

            if(allocatedBits != 1)
            {
                size_t imageSizeBytes = nativeImageSizeBits / 8;
                std::shared_ptr<memory> pStreamMemory = std::make_shared<memory>(imageSizeBytes);
                pSourceStream->read(pStreamMemory->data(), pStreamMemory->size());
                readInterleavedSubsampled<std::int32_t>(subsampledChannels, allocatedBits, pStreamMemory->data());

                std::uint32_t maxSamplingFactorX = bSubSampledX ? 2u : 1u;
                std::uint32_t maxSamplingFactorY = bSubSampledY ? 2u : 1u;

                for(std::uint32_t copyChannels = 0; copyChannels < channelsNumber; ++copyChannels)
                {
                    std::shared_ptr<channel> pChannel = subsampledChannels[copyChannels];

                    // Adjust b2complement buffers
                    ///////////////////////////////////////////////////////////
                    if(b2Complement)
                    {
                        adjustB2Complement(pChannel->m_memory->data(), highBit, bitDepth_t::depthS32, pChannel->m_memory->size() / sizeof(std::uint32_t));
                    }

                    imageHandler->copyFromInt32Interleaved(
                                pChannel->m_pBuffer,
                                maxSamplingFactorX / pChannel->m_samplingFactorX,
                                maxSamplingFactorY / pChannel->m_samplingFactorY,
                                0, 0,
                                pChannel->m_width * (maxSamplingFactorX /pChannel->m_samplingFactorX),
                                pChannel->m_height * (maxSamplingFactorY /pChannel->m_samplingFactorY),
                                copyChannels,
                                imageWidth,
                                imageHeight,
                                channelsNumber);
                }
            }
            else
            {
                IMEBRA_THROW(std::logic_error, "Cannot load subsampled 1 bpp images");
            }
        }
    }

    // Return OK
    ///////////////////////////////////////////////////////////
    return pImage;

    IMEBRA_FUNCTION_END();

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
bool dicomNativeImageCodec::defaultInterleaved() const
{
    return true;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Insert an image into a Dicom structure
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomNativeImageCodec::setImage(
        std::shared_ptr<streamWriter> pDestStream,
        std::shared_ptr<const image> pImage,
        const std::string& /* transferSyntax */,
        imageQuality_t /*imageQuality*/,
        std::uint32_t allocatedBits,
        bool bSubSampledX,
        bool bSubSampledY,
        bool bInterleaved,
        bool /*b2Complement*/) const
{
    IMEBRA_FUNCTION_START();

    // First calculate the attributes we want to use.
    // Return an exception if they are different from the
    //  old ones and bDontChangeAttributes is true
    ///////////////////////////////////////////////////////////
    std::uint32_t imageWidth, imageHeight;
    pImage->getSize(&imageWidth, &imageHeight);

    std::string colorSpace = pImage->getColorSpace();

    std::shared_ptr<handlers::readingDataHandlerNumericBase> imageHandler = pImage->getReadingDataHandler();
    std::uint32_t channelsNumber = pImage->getChannelsNumber();

    size_t nativeImageSizeBits = getNativeImageSizeBits(allocatedBits, imageWidth, imageHeight, channelsNumber, bSubSampledX, bSubSampledY);

    if(!bSubSampledX && !bSubSampledY)
    {
        size_t imageSizeBytes = (nativeImageSizeBits + 7) / 8;
        std::shared_ptr<memory> pStreamMemory = std::make_shared<memory>(imageSizeBytes);
        if(allocatedBits != 1)
        {
            if(bInterleaved || channelsNumber == 1)
            {
                writeInterleavedNotSubsampled(
                            imageHandler->getMemoryBuffer(),
                            allocatedBits,
                            pImage->getDepth(),
                            pStreamMemory->data(),
                            imageWidth * imageHeight,
                            channelsNumber
                            );
            }
            else
            {
                writeNotInterleavedNotSubsampled(
                            imageHandler->getMemoryBuffer(),
                            allocatedBits,
                            pImage->getDepth(),
                            pStreamMemory->data(),
                            imageWidth * imageHeight,
                            channelsNumber
                            );
            }
        }
        else
        {
            if(bSubSampledX || bSubSampledY)
            {
                IMEBRA_THROW(std::logic_error, "Cannot save subsampled 1 bpp images");
            }
            if(bInterleaved || channelsNumber == 1)
            {
                write1bitInterleaved(
                            imageHandler->getMemoryBuffer(),
                            pImage->getDepth(),
                            pStreamMemory->data(),
                            imageWidth * imageHeight,
                            channelsNumber
                            );
            }
            else
            {
                write1bitNotInterleaved(
                            imageHandler->getMemoryBuffer(),
                            pImage->getDepth(),
                            pStreamMemory->data(),
                            imageWidth * imageHeight,
                            channelsNumber
                            );
            }
        }

        pDestStream->write(pStreamMemory->data(), imageSizeBytes);

    }
    else
    {
        std::vector<std::shared_ptr<channel>> subsampledChannels = splitImageIntoChannels(pImage, bSubSampledX, bSubSampledY);

        if(allocatedBits != 1)
        {
            size_t imageSizeBytes = nativeImageSizeBits / 8;
            std::shared_ptr<memory> pStreamMemory = std::make_shared<memory>(imageSizeBytes);
            writeInterleavedSubsampled<std::int32_t>(subsampledChannels, allocatedBits, pStreamMemory->data());

            pDestStream->write(pStreamMemory->data(), imageSizeBytes);
        }
        else
        {
            IMEBRA_THROW(std::logic_error, "Subsampled images with 1 bit per channel are not supported")
        }
    }


    IMEBRA_FUNCTION_END();
}


void dicomNativeImageCodec::writeInterleavedNotSubsampled(
        const std::uint8_t* pImageSamples,
        std::uint32_t allocatedBits,
        bitDepth_t samplesDepth,
        std::uint8_t* pLittleEndianTagData,
        size_t numPixels,
        size_t numChannels)
{
    switch(samplesDepth)
    {
    case bitDepth_t::depthU8:
        writeInterleavedNotSubsampled(pImageSamples, allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS8:
        writeInterleavedNotSubsampled(reinterpret_cast<const std::int8_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU16:
        writeInterleavedNotSubsampled(reinterpret_cast<const std::uint16_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS16:
        writeInterleavedNotSubsampled(reinterpret_cast<const std::int16_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU32:
        writeInterleavedNotSubsampled(reinterpret_cast<const std::uint32_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS32:
        writeInterleavedNotSubsampled(reinterpret_cast<const std::int32_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    }
}


void dicomNativeImageCodec::readInterleavedNotSubsampled(
        std::uint8_t* pImageSamples,
        std::uint32_t allocatedBits,
        bitDepth_t samplesDepth,
        const std::uint8_t* pLittleEndianTagData,
        size_t numPixels,
        size_t numChannels)
{
    switch(samplesDepth)
    {
    case bitDepth_t::depthU8:
        readInterleavedNotSubsampled(pImageSamples, allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS8:
        readInterleavedNotSubsampled(reinterpret_cast<std::int8_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU16:
        readInterleavedNotSubsampled(reinterpret_cast<std::uint16_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS16:
        readInterleavedNotSubsampled(reinterpret_cast<std::int16_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU32:
        readInterleavedNotSubsampled(reinterpret_cast<std::uint32_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS32:
        readInterleavedNotSubsampled(reinterpret_cast<std::int32_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    }
}


void dicomNativeImageCodec::writeNotInterleavedNotSubsampled(
        const std::uint8_t* pImageSamples,
        std::uint32_t allocatedBits,
        bitDepth_t samplesDepth,
        std::uint8_t* pLittleEndianTagData,
        size_t numPixels,
        size_t numChannels)
{
    switch(samplesDepth)
    {
    case bitDepth_t::depthU8:
        writeNotInterleavedNotSubsampled(pImageSamples, allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS8:
        writeNotInterleavedNotSubsampled(reinterpret_cast<const std::int8_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU16:
        writeNotInterleavedNotSubsampled(reinterpret_cast<const std::uint16_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS16:
        writeNotInterleavedNotSubsampled(reinterpret_cast<const std::int16_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU32:
        writeNotInterleavedNotSubsampled(reinterpret_cast<const std::uint32_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS32:
        writeNotInterleavedNotSubsampled(reinterpret_cast<const std::int32_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    }
}


void dicomNativeImageCodec::readNotInterleavedNotSubsampled(
        std::uint8_t* pImageSamples,
        std::uint32_t allocatedBits,
        bitDepth_t samplesDepth,
        const std::uint8_t* pLittleEndianTagData,
        size_t numPixels,
        size_t numChannels)
{
    switch(samplesDepth)
    {
    case bitDepth_t::depthU8:
        readNotInterleavedNotSubsampled(pImageSamples, allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS8:
        readNotInterleavedNotSubsampled(reinterpret_cast<std::int8_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU16:
        readNotInterleavedNotSubsampled(reinterpret_cast<std::uint16_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS16:
        readNotInterleavedNotSubsampled(reinterpret_cast<std::int16_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU32:
        readNotInterleavedNotSubsampled(reinterpret_cast<std::uint32_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS32:
        readNotInterleavedNotSubsampled(reinterpret_cast<std::int32_t*>(pImageSamples), allocatedBits, pLittleEndianTagData, numPixels, numChannels);
        break;
    }
}


void dicomNativeImageCodec::write1bitInterleaved(
        const std::uint8_t* pImageSamples,
        bitDepth_t samplesDepth,
        std::uint8_t* pLittleEndianTagData,
        size_t numPixels,
        size_t numChannels)
{
    switch(samplesDepth)
    {
    case bitDepth_t::depthU8:
        write1bitInterleaved(pImageSamples, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS8:
        write1bitInterleaved(reinterpret_cast<const std::int8_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU16:
        write1bitInterleaved(reinterpret_cast<const std::uint16_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS16:
        write1bitInterleaved(reinterpret_cast<const std::int16_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU32:
        write1bitInterleaved(reinterpret_cast<const std::uint32_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS32:
        write1bitInterleaved(reinterpret_cast<const std::int32_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    }
}


void dicomNativeImageCodec::write1bitNotInterleaved(
        const std::uint8_t* pImageSamples,
        bitDepth_t samplesDepth,
        std::uint8_t* pLittleEndianTagData,
        size_t numPixels,
        size_t numChannels)
{
    switch(samplesDepth)
    {
    case bitDepth_t::depthU8:
        write1bitNotInterleaved(pImageSamples, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS8:
        write1bitNotInterleaved(reinterpret_cast<const std::int8_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU16:
        write1bitNotInterleaved(reinterpret_cast<const std::uint16_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS16:
        write1bitNotInterleaved(reinterpret_cast<const std::int16_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU32:
        write1bitNotInterleaved(reinterpret_cast<const std::uint32_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS32:
        write1bitNotInterleaved(reinterpret_cast<const std::int32_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    }
}


void dicomNativeImageCodec::read1bitInterleaved(
        std::uint8_t* pImageSamples,
        bitDepth_t samplesDepth,
        const std::uint8_t* pLittleEndianTagData,
        size_t numPixels,
        size_t numChannels)
{
    switch(samplesDepth)
    {
    case bitDepth_t::depthU8:
        read1bitInterleaved(pImageSamples, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS8:
        read1bitInterleaved(reinterpret_cast<std::int8_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU16:
        read1bitInterleaved(reinterpret_cast<std::uint16_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS16:
        read1bitInterleaved(reinterpret_cast<std::int16_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU32:
        read1bitInterleaved(reinterpret_cast<std::uint32_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS32:
        read1bitInterleaved(reinterpret_cast<std::int32_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    }
}


void dicomNativeImageCodec::read1bitNotInterleaved(
        std::uint8_t* pImageSamples,
        bitDepth_t samplesDepth,
        const std::uint8_t* pLittleEndianTagData,
        size_t numPixels,
        size_t numChannels)
{
    switch(samplesDepth)
    {
    case bitDepth_t::depthU8:
        read1bitNotInterleaved(pImageSamples, pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS8:
        read1bitNotInterleaved(reinterpret_cast<std::int8_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU16:
        read1bitNotInterleaved(reinterpret_cast<std::uint16_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS16:
        read1bitNotInterleaved(reinterpret_cast<std::int16_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthU32:
        read1bitNotInterleaved(reinterpret_cast<std::uint32_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    case bitDepth_t::depthS32:
        read1bitNotInterleaved(reinterpret_cast<std::int32_t*>(pImageSamples), pLittleEndianTagData, numPixels, numChannels);
        break;
    }
}




///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns true if the codec can handle the transfer
//  syntax
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool dicomNativeImageCodec::canHandleTransferSyntax(const std::string& transferSyntax) const
{
    return(
                transferSyntax == "1.2.840.10008.1.2" ||       // Implicit VR little endian
                transferSyntax == "1.2.840.10008.1.2.1" ||     // Explicit VR little endian
                transferSyntax=="1.2.840.10008.1.2.1.99" ||    // Deflated explicit VR little endian
                transferSyntax == "1.2.840.10008.1.2.2");      // Explicit VR big endian
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
bool dicomNativeImageCodec::encapsulated(const std::string& transferSyntax) const
{
    IMEBRA_FUNCTION_START();

    if(!canHandleTransferSyntax(transferSyntax))
    {
        IMEBRA_THROW(CodecWrongTransferSyntaxError, "Cannot handle the transfer syntax");
    }
    return false;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Suggest the number of allocated bits
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dicomNativeImageCodec::suggestAllocatedBits(const std::string& /* transferSyntax */, std::uint32_t highBit) const
{
    if(highBit == 0)
    {
        return 1;
    }

    return (highBit + 8) & 0xfffffff8;
}

size_t dicomNativeImageCodec::getNativeImageSizeBits(std::uint32_t allocatedBits,
                                                  std::uint32_t imageWidth,
                                                  std::uint32_t imageHeight,
                                                  std::uint32_t channelsNumber,
                                                  bool bSubsampledX,
                                                  bool bSubsampledY)
{
    if(bSubsampledX && (imageWidth & 1u) == 1u)
    {
        ++imageWidth;
    }
    if(bSubsampledY && (imageHeight & 1u) == 1u)
    {
        ++imageHeight;
    }

    size_t channelSizeBits = allocatedBits * imageWidth * imageHeight;
    if(!bSubsampledX && !bSubsampledY)
    {
        return channelSizeBits * channelsNumber;
    }

    size_t subsequentChannelsSizeBits = channelSizeBits;
    if(bSubsampledX)
    {
        subsequentChannelsSizeBits /= 2;
    }
    if(bSubsampledY)
    {
        subsequentChannelsSizeBits /= 2;
    }

    return channelSizeBits + subsequentChannelsSizeBits * (channelsNumber - 1);
}

} // namespace codecs

} // namespace implementation

} // namespace imebra

