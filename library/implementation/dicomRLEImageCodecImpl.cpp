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
#include "dicomRLEImageCodecImpl.h"
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
std::shared_ptr<image> dicomRLEImageCodec::getImage(const std::string& /* transferSyntax */,
                                                    const std::string& colorSpace,
                                                    std::uint32_t channelsNumber,
                                                    std::uint32_t imageWidth,
                                                    std::uint32_t imageHeight,
                                                    bool bSubSampledX,
                                                    bool bSubSampledY,
                                                    bool bInterleaved,
                                                    bool b2Complement,
                                                    std::uint8_t allocatedBits,
                                                    std::uint8_t storedBits,
                                                    std::uint8_t highBit,
                                                    std::shared_ptr<streamReader> pSourceStream) const

{
    IMEBRA_FUNCTION_START();

    if(bInterleaved)
    {
        IMEBRA_THROW(CodecCorruptedFileError, "RLE encoding does not allow planar configuration = 0");
    }

    if(bSubSampledX || bSubSampledY)
    {
        IMEBRA_THROW(CodecCorruptedFileError, "Cannot read subsampled RLE compressed images");
    }

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
    std::shared_ptr<handlers::writingDataHandlerNumericBase> handler = pImage->getWritingDataHandler();
    std::uint32_t tempChannelsNumber = pImage->getChannelsNumber();

    if(tempChannelsNumber != channelsNumber)
    {
        IMEBRA_THROW(CodecCorruptedFileError,  "The color space " << colorSpace << " requires " << tempChannelsNumber << " but the dataset declares " << channelsNumber << " channels");
    }

    // Allocate the dicom channels
    ///////////////////////////////////////////////////////////
    std::vector<std::shared_ptr<channel>> channels = allocChannels(channelsNumber, imageWidth, imageHeight, bSubSampledX, bSubSampledY);

    std::uint32_t mask = (std::uint32_t)( ((std::uint64_t)1 << (highBit + 1)) - 1);
    mask -= (std::uint32_t)(((std::uint64_t)1 << (highBit + 1 - storedBits)) - 1);

    readRLECompressed(channels, imageWidth, imageHeight, channelsNumber, pSourceStream.get(), allocatedBits, mask);

    if(b2Complement)
    {
        for(const std::shared_ptr<channel>& pChannel: channels)
        {
            {
                adjustB2Complement(pChannel->m_memory->data(), highBit, bitDepth_t::depthS32, pChannel->m_memory->size() / sizeof(std::uint32_t));
            }
        }
    }

    // Copy the dicom channels into the image
    ///////////////////////////////////////////////////////////
    std::uint32_t maxSamplingFactorX = bSubSampledX ? 2 : 1;
    std::uint32_t maxSamplingFactorY = bSubSampledY ? 2 : 1;
    for(std::uint32_t copyChannels = 0; copyChannels < channelsNumber; ++copyChannels)
    {
        std::shared_ptr<channel> dicomChannel = channels[copyChannels];
        handler->copyFromInt32Interleaved(
                    dicomChannel->m_pBuffer,
                    maxSamplingFactorX /dicomChannel->m_samplingFactorX,
                    maxSamplingFactorY /dicomChannel->m_samplingFactorY,
                    0, 0,
                    dicomChannel->m_width * maxSamplingFactorX / dicomChannel->m_samplingFactorX,
                    dicomChannel->m_height * maxSamplingFactorY / dicomChannel->m_samplingFactorY,
                    copyChannels,
                    imageWidth,
                    imageHeight,
                    channelsNumber);

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
bool dicomRLEImageCodec::defaultInterleaved() const
{
    return false;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write a RLE compressed image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomRLEImageCodec::writeRLECompressed(
        const std::vector<std::shared_ptr<channel>>& channels,
        std::uint32_t imageWidth,
        std::uint32_t imageHeight,
        std::uint32_t channelsNumber,
        streamWriter* pDestStream,
        std::uint8_t allocatedBits,
        std::uint32_t mask
        )
{
    IMEBRA_FUNCTION_START();

    std::uint32_t segmentsOffset[16];
    ::memset(segmentsOffset, 0, sizeof(segmentsOffset));

    // The first phase fills the segmentsOffset pointers, the
    //  second phase writes to the stream.
    ///////////////////////////////////////////////////////////
    for(int phase = 0; phase < 2; ++phase)
    {
        if(phase == 1)
        {
            pDestStream->adjustEndian((std::uint8_t*)segmentsOffset, 4, streamController::lowByteEndian, sizeof(segmentsOffset) / sizeof(segmentsOffset[0]));
            pDestStream->write((std::uint8_t*)segmentsOffset, sizeof(segmentsOffset));
        }

        std::uint32_t segmentNumber = 0;
        std::uint32_t offset = 64;

        for(std::uint32_t scanChannels = 0; scanChannels < channelsNumber; ++scanChannels)
        {
            std::vector<std::uint8_t> rowBytes(imageWidth);

            std::vector<std::uint8_t> differentBytes;
            differentBytes.reserve(imageWidth);

            for(std::int32_t rightShift = (std::int32_t)(((allocatedBits + 7) & 0xfffffff8) - 8); rightShift >= 0; rightShift -= 8)
            {
                std::int32_t* pPixel = channels[scanChannels]->m_pBuffer;

                if(phase == 0)
                {
                    segmentsOffset[++segmentNumber] = offset;
                    segmentsOffset[0] = segmentNumber;
                }
                else
                {
                    offset = segmentsOffset[++segmentNumber];
                }

                for(std::uint32_t scanY = imageHeight; scanY != 0; --scanY)
                {
                    std::uint8_t* rowBytesPointer = &(rowBytes[0]);

                    for(std::uint32_t scanX = imageWidth; scanX != 0; --scanX)
                    {
                        *(rowBytesPointer++) = (std::uint8_t)( ((std::uint32_t)*pPixel & mask) >> rightShift);
                        ++pPixel;
                    }

                    for(size_t scanBytes = 0; scanBytes < imageWidth; /* left empty */)
                    {
                        std::uint8_t currentByte = rowBytes[scanBytes];

                        // Calculate the run-length
                        ///////////////////////////
                        size_t runLength(1);
                        for(; ((scanBytes + runLength) != imageWidth) && rowBytes[scanBytes + runLength] == currentByte; ++runLength)
                        {
                        }

                        // Write the runlength
                        //////////////////////
                        if(runLength > 3)
                        {
                            if(!differentBytes.empty())
                            {
                                offset += (std::uint32_t)writeRLEDifferentBytes(&differentBytes, pDestStream, phase == 1);
                            }
                            if(runLength > 128)
                            {
                                runLength = 128;
                            }
                            offset += 2;
                            scanBytes += runLength;
                            if(phase == 1)
                            {
                                std::uint8_t lengthByte = (std::uint8_t)(1 - runLength);
                                pDestStream->write(&lengthByte, 1);
                                pDestStream->write(&currentByte, 1);
                            }
                            continue;
                        }

                        // Remmember sequence of different bytes
                        ////////////////////////////////////////
                        differentBytes.push_back(currentByte);
                        ++scanBytes;
                    } // for(std::uint32_t scanBytes = 0; scanBytes < imageWidth; )

                    offset += (std::uint32_t)writeRLEDifferentBytes(&differentBytes, pDestStream, phase == 1);

                } // for(std::uint32_t scanY = imageHeight; scanY != 0; --scanY)

                if((offset & 1) != 0)
                {
                    ++offset;
                    if(phase == 1)
                    {
                        const std::uint8_t command = 0x80;
                        pDestStream->write(&command, 1);
                    }
                }

            } // for(std::int32_t rightShift = ((allocatedBits + 7) & 0xfffffff8) -8; rightShift >= 0; rightShift -= 8)

        } // for(int scanChannels = 0; scanChannels < channelsNumber; ++scanChannels)

    } // for(int phase = 0; phase < 2; ++phase)

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write RLE sequence of different bytes
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
size_t dicomRLEImageCodec::writeRLEDifferentBytes(std::vector<std::uint8_t>* pDifferentBytes, streamWriter* pDestStream, bool bWrite)
{
    IMEBRA_FUNCTION_START();

    size_t writtenLength = 0;
    for(size_t offset(0); offset != pDifferentBytes->size();)
    {
        size_t writeSize = pDifferentBytes->size() - offset;
        if(writeSize > 128)
        {
            writeSize = 128;
        }
        writtenLength += writeSize + 1;
        if(bWrite)
        {
            const std::uint8_t writeLength((std::uint8_t)(writeSize - 1));
            pDestStream->write(&writeLength, 1);
            pDestStream->write(&(pDifferentBytes->at(offset)), writeSize);
        }
        offset += writeSize;
    }
    pDifferentBytes->clear();

    // return number of written bytes
    /////////////////////////////////
    return writtenLength;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read a RLE compressed image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomRLEImageCodec::readRLECompressed(
        const std::vector<std::shared_ptr<channel>>& channels,
        std::uint32_t imageWidth,
        std::uint32_t imageHeight,
        std::uint32_t channelsNumber,
        streamReader* pSourceStream,
        std::uint8_t allocatedBits,
        std::uint32_t mask)
{
    IMEBRA_FUNCTION_START();

    // Copy the RLE header into the segmentsOffset array
    //  and adjust the byte endian to the machine architecture
    ///////////////////////////////////////////////////////////
    std::uint32_t segmentsOffset[16];
    ::memset(segmentsOffset, 0, sizeof(segmentsOffset));
    pSourceStream->read((std::uint8_t*)segmentsOffset, 64);
    pSourceStream->adjustEndian((std::uint8_t*)segmentsOffset, 4, streamController::lowByteEndian, sizeof(segmentsOffset) / sizeof(segmentsOffset[0]));

    //
    // Scan all the RLE segments
    //
    ///////////////////////////////////////////////////////////
    std::uint32_t loopsNumber = channelsNumber;
    std::uint32_t loopSize = imageWidth * imageHeight;

    std::uint32_t currentSegmentOffset = sizeof(segmentsOffset);
    std::uint8_t segmentNumber = 0;
    for(std::uint32_t channel = 0; channel<loopsNumber; ++channel)
    {
        for(std::int32_t leftShift = (std::int32_t)(((allocatedBits + 7) & 0xfffffff8) - 8); leftShift >= 0; leftShift -= 8)
        {
            // Prepare to scan all the RLE segment
            ///////////////////////////////////////////////////////////
            std::uint32_t segmentOffset = segmentsOffset[++segmentNumber]; // Get the offset
            pSourceStream->seekForward(segmentOffset - currentSegmentOffset);
            currentSegmentOffset = segmentOffset;

            std::uint8_t  rleByte = 0;         // RLE code
            std::uint8_t  copyBytes = 0;       // Number of bytes to copy
            std::uint8_t  runByte = 0;         // Byte to use in run-lengths
            std::uint8_t  runLength = 0;       // Number of bytes with the same information (runByte)
            std::uint8_t  copyBytesBuffer[0x81];

            std::int32_t* pChannelMemory = channels[channel]->m_pBuffer;
            std::uint32_t channelSize = loopSize;
            std::uint8_t* pScanCopyBytes;

            // Read the RLE segment
            ///////////////////////////////////////////////////////////
            pSourceStream->read(&rleByte, 1);
            ++currentSegmentOffset;
            while(channelSize != 0)
            {
                if(rleByte==0x80)
                {
                    pSourceStream->read(&rleByte, 1);
                    ++currentSegmentOffset;
                    continue;
                }

                // Copy the specified number of bytes
                ///////////////////////////////////////////////////////////
                if(rleByte<0x80)
                {
                    copyBytes = ++rleByte;
                    if(copyBytes < channelSize)
                    {
                        pSourceStream->read(copyBytesBuffer, copyBytes + 1);
                        currentSegmentOffset += copyBytes + 1;
                        rleByte = copyBytesBuffer[copyBytes];
                    }
                    else
                    {
                        pSourceStream->read(copyBytesBuffer, copyBytes);
                        currentSegmentOffset += copyBytes;
                    }
                    pScanCopyBytes = copyBytesBuffer;
                    while(copyBytes-- && channelSize != 0)
                    {
                        *pChannelMemory |= (std::int32_t) ((((std::uint32_t)(*pScanCopyBytes++)) << leftShift) & mask);
                        ++pChannelMemory;
                        --channelSize;
                    }
                    continue;
                }

                // Copy the same byte several times
                ///////////////////////////////////////////////////////////
                runLength = (std::uint8_t)(1-rleByte);
                if(runLength < channelSize)
                {
                    pSourceStream->read(copyBytesBuffer, 2);
                    currentSegmentOffset += 2;
                    runByte = copyBytesBuffer[0];
                    rleByte = copyBytesBuffer[1];
                }
                else
                {
                    pSourceStream->read(&runByte, 1);
                    ++currentSegmentOffset;
                }
                while(runLength-- && channelSize != 0)
                {
                    *pChannelMemory |= (std::int32_t) (((std::uint32_t)runByte << leftShift) & mask);
                    ++pChannelMemory;
                    --channelSize;
                }

            } // ...End of the segment scanning loop

        } // ...End of the leftshift calculation

    } // ...Channels scanning loop

    IMEBRA_FUNCTION_END();
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
void dicomRLEImageCodec::setImage(
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

    if(bSubSampledX || bSubSampledY)
    {
        IMEBRA_THROW(std::logic_error, "Cannot write subsampled RLE compressed images");
    }

    if(bInterleaved)
    {
        IMEBRA_THROW(std::logic_error, "Cannot write interleaved RLE compressed images");
    }

    // First calculate the attributes we want to use.
    // Return an exception if they are different from the
    //  old ones and bDontChangeAttributes is true
    ///////////////////////////////////////////////////////////
    std::uint32_t imageWidth, imageHeight;
    pImage->getSize(&imageWidth, &imageHeight);

    std::string colorSpace = pImage->getColorSpace();
    std::uint32_t highBit = pImage->getHighBit();

    std::shared_ptr<handlers::readingDataHandlerNumericBase> imageHandler = pImage->getReadingDataHandler();
    std::uint32_t channelsNumber = pImage->getChannelsNumber();

    // Copy the image into the dicom channels
    ///////////////////////////////////////////////////////////
    std::vector<std::shared_ptr<channel>> channels = splitImageIntoChannels(pImage, bSubSampledX, bSubSampledY);

    std::uint32_t mask = (std::uint32_t)(((std::uint64_t)1 << (highBit + 1)) - 1);

    writeRLECompressed(
                channels,
                imageWidth,
                imageHeight,
                channelsNumber,
                pDestStream.get(),
                (std::uint8_t)allocatedBits,
                mask);

    IMEBRA_FUNCTION_END();
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
bool dicomRLEImageCodec::canHandleTransferSyntax(const std::string& transferSyntax) const
{
    return(transferSyntax == "1.2.840.10008.1.2.5");     // RLE compression
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
bool dicomRLEImageCodec::encapsulated(const std::string& transferSyntax) const
{
    IMEBRA_FUNCTION_START();

    if(!canHandleTransferSyntax(transferSyntax))
    {
        IMEBRA_THROW(CodecWrongTransferSyntaxError, "Cannot handle the transfer syntax");
    }
    return (true);

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
std::uint32_t dicomRLEImageCodec::suggestAllocatedBits(const std::string& /* transferSyntax */, std::uint32_t highBit) const
{
    return (highBit + 8) & 0xfffffff8;
}

} // namespace codecs

} // namespace implementation

} // namespace imebra

