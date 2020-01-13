/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dicomNativeImageCodecImpl.h
    \brief Declaration of the class dicomnativeImageCodec.

*/

#if !defined(imebraDicomNativeImageCodec_CC44A2C5_2B8C_42c1_9704_3F9C582643B9__INCLUDED_)
#define imebraDicomNativeImageCodec_CC44A2C5_2B8C_42c1_9704_3F9C582643B9__INCLUDED_

#include "imageCodecImpl.h"
#include "dataImpl.h"
#include "dataSetImpl.h"
#include "streamControllerImpl.h"

namespace imebra
{

namespace implementation
{

namespace codecs
{

/// \addtogroup group_codecs
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief The Dicom Native codec.
///
/// This class is used to decode and encode a native DICOM
/// image.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dicomNativeImageCodec : public imageCodec
{
public:
    // Get an image from a dicom structure
    ///////////////////////////////////////////////////////////
    virtual std::shared_ptr<image> getImage(const std::string& transferSyntax,
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
                                            std::shared_ptr<streamReader> pSourceStream) const override;

    // Return the default planar configuration
    ///////////////////////////////////////////////////////////
    virtual bool defaultInterleaved() const override;

    // Write an image into a dicom structure
    ///////////////////////////////////////////////////////////
    virtual void setImage(
        std::shared_ptr<streamWriter> pDestStream,
        std::shared_ptr<const image> pImage,
        const std::string& transferSyntax,
        imageQuality_t imageQuality,
        std::uint32_t allocatedBits,
        bool bSubSampledX,
        bool bSubSampledY,
        bool bInterleaved,
        bool b2Complement) const override;

    // Returns true if the codec can handle the transfer
    //  syntax
    ///////////////////////////////////////////////////////////
    virtual bool canHandleTransferSyntax(const std::string& transferSyntax) const override;

    // Returns true if the transfer syntax has to be
    //  encapsulated
    //
    ///////////////////////////////////////////////////////////
    virtual bool encapsulated(const std::string& transferSyntax) const override;

    // Returns the suggested allocated bits
    ///////////////////////////////////////////////////////////
    virtual std::uint32_t suggestAllocatedBits(const std::string& transferSyntax, std::uint32_t highBit) const override;

    // Returns the image size, in bits
    ///////////////////////////////////////////////////////////
    static size_t getNativeImageSizeBits(std::uint32_t allocatedBits,
                                      std::uint32_t imageWidth,
                                      std::uint32_t imageHeight,
                                      std::uint32_t channelsNumber,
                                      bool bSubsampledX,
                                      bool bSubsampledY);

protected:


    template<typename samplesType_t> static void writeInterleavedNotSubsampled(
            const samplesType_t* pImageSamples,
            std::uint32_t allocatedBits,
            std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels)
    {
        std::uint32_t allocatedBytes = allocatedBits / 8;
        const std::uint8_t mask(0xff);
        for(size_t numSamples(numPixels * numChannels); numSamples != 0; --numSamples)
        {
            for(std::uint32_t shiftRight(0); shiftRight != allocatedBytes; ++shiftRight)
            {
                *(pLittleEndianTagData++) = static_cast<std::uint8_t>((*pImageSamples >> (shiftRight * 8)) & mask);
            }
            ++pImageSamples;
        }
    }

    static void writeInterleavedNotSubsampled(
            const std::uint8_t* pImageSamples,
            std::uint32_t allocatedBits,
            bitDepth_t samplesDepth,
            std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels);

    template<typename samplesType_t> static void readInterleavedNotSubsampled(
            samplesType_t* pImageSamples,
            std::uint32_t allocatedBits,
            const std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels)
    {
        std::uint32_t allocatedBytes = allocatedBits / 8;
        for(size_t numSamples(numPixels * numChannels); numSamples != 0; --numSamples)
        {
            samplesType_t value(0);
            for(std::uint32_t shiftLeft(0); shiftLeft != allocatedBytes; ++shiftLeft)
            {
                value = static_cast<samplesType_t>(value | static_cast<samplesType_t>(*(pLittleEndianTagData++)) << (shiftLeft * 8));
            }
            *(pImageSamples++) = value;
        }
    }

    static void readInterleavedNotSubsampled(
            std::uint8_t* pImageSamples,
            std::uint32_t allocatedBits,
            bitDepth_t samplesDepth,
            const std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels);


    template<typename samplesType_t> static void writeNotInterleavedNotSubsampled(
            const samplesType_t* pImageSamples,
            std::uint32_t allocatedBits,
            std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels)
    {
        std::uint32_t allocatedBytes = allocatedBits / 8;
        const std::uint8_t mask(0xff);
        for(size_t scanChannels(0); scanChannels != numChannels; ++scanChannels)
        {
            const samplesType_t* pScanImageSamples = pImageSamples + scanChannels;
            for(size_t scanPixels(numPixels); scanPixels != 0; --scanPixels)
            {
                for(std::uint32_t shiftRight(0); shiftRight != allocatedBytes; ++shiftRight)
                {
                    *(pLittleEndianTagData++) = static_cast<std::uint8_t>((*pScanImageSamples >> (shiftRight * 8)) & mask);
                }
                pScanImageSamples += numChannels;
            }
        }
    }


    static void writeNotInterleavedNotSubsampled(
            const std::uint8_t* pImageSamples,
            std::uint32_t allocatedBits,
            bitDepth_t samplesDepth,
            std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels);

    template<typename samplesType_t> static void readNotInterleavedNotSubsampled(
            samplesType_t* pImageSamples,
            std::uint32_t allocatedBits,
            const std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels)
    {
        std::uint32_t allocatedBytes = allocatedBits / 8;
        for(size_t scanChannels(0); scanChannels != numChannels; ++scanChannels)
        {
            samplesType_t* pScanImageSamples = pImageSamples + scanChannels;
            for(size_t scanPixels(numPixels); scanPixels != 0; --scanPixels)
            {
                samplesType_t value(0);
                for(std::uint32_t shiftLeft(0); shiftLeft != allocatedBytes; ++shiftLeft)
                {
                    value = static_cast<samplesType_t>(value | (static_cast<samplesType_t>(*(pLittleEndianTagData++)) << (shiftLeft * 8)));
                }
                *pScanImageSamples = value;
                pScanImageSamples += numChannels;
            }
        }
    }


    static void readNotInterleavedNotSubsampled(
            std::uint8_t* pImageSamples,
            std::uint32_t allocatedBits,
            bitDepth_t samplesDepth,
            const std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels);


    template<typename samplesType_t> static void writeInterleavedSubsampled(
            const std::vector<std::shared_ptr<channel>> pImageSamples,
            std::uint32_t allocatedBits,
            std::uint8_t* pLittleEndianTagData)
    {
        const std::uint32_t allocatedBytes = allocatedBits / 8;

        std::uint32_t maxSamplingFactorX = 1;
        std::uint32_t maxSamplingFactorY = 1;
        for (const std::shared_ptr<const channel>& pChannel: pImageSamples)
        {
            maxSamplingFactorX = std::max(maxSamplingFactorX, pChannel->m_samplingFactorX);
            maxSamplingFactorY = std::max(maxSamplingFactorY, pChannel->m_samplingFactorY);
        }

        const std::uint8_t mask(0xff);

        // First copy the first channel
        size_t blocksX = pImageSamples[0]->m_width / maxSamplingFactorX;
        size_t blocksY = pImageSamples[0]->m_height / maxSamplingFactorY;
        size_t imageWidth = pImageSamples[0]->m_width;

        std::uint8_t* pScanDestination(pLittleEndianTagData);
        for(size_t scanBlocksY(0); scanBlocksY != blocksY; ++scanBlocksY )
        {
            const samplesType_t* pScanImageSamples =
                    reinterpret_cast<const samplesType_t*>(pImageSamples[0]->m_pBuffer) + scanBlocksY * imageWidth * maxSamplingFactorY;
            for(size_t scanBlocksX(0); scanBlocksX != blocksX; ++scanBlocksX )
            {
                for(size_t scanInsideBlockY(0); scanInsideBlockY != maxSamplingFactorY; ++scanInsideBlockY)
                {
                    for(size_t scanInsideBlockX(0); scanInsideBlockX != maxSamplingFactorX; ++scanInsideBlockX)
                    {
                        const samplesType_t pixel = *(pScanImageSamples + scanInsideBlockX + scanInsideBlockY * imageWidth);
                        for(std::uint32_t shiftRight(0); shiftRight != allocatedBytes; ++shiftRight)
                        {
                            *(pScanDestination++) = static_cast<std::uint8_t>((pixel >> (shiftRight * 8)) & mask);
                        }
                    }
                }
                pScanImageSamples += maxSamplingFactorX;
                for(size_t channelNumber(1u); channelNumber < pImageSamples.size(); ++channelNumber)
                {
                    const samplesType_t pixel = pImageSamples[channelNumber]->m_pBuffer[scanBlocksY * blocksX + scanBlocksX];
                    for(std::uint32_t shiftRight(0); shiftRight != allocatedBytes; ++shiftRight)
                    {
                        *(pScanDestination++) = static_cast<std::uint8_t>((pixel >> (shiftRight * 8)) & mask);
                    }
                }
            }
        }
    }


    template<typename samplesType_t> static void readInterleavedSubsampled(
            const std::vector<std::shared_ptr<channel>> pImageSamples,
            std::uint32_t allocatedBits,
            const std::uint8_t* pLittleEndianTagData)
    {
        const std::uint32_t allocatedBytes = allocatedBits / 8;

        std::uint32_t maxSamplingFactorX = 1;
        std::uint32_t maxSamplingFactorY = 1;
        for (const std::shared_ptr<const channel>& pChannel: pImageSamples)
        {
            maxSamplingFactorX = std::max(maxSamplingFactorX, pChannel->m_samplingFactorX);
            maxSamplingFactorY = std::max(maxSamplingFactorY, pChannel->m_samplingFactorY);
        }

        // First copy the first channel
        size_t blocksX = pImageSamples[0]->m_width / maxSamplingFactorX;
        size_t blocksY = pImageSamples[0]->m_height / maxSamplingFactorY;
        size_t imageWidth = pImageSamples[0]->m_width;

        const std::uint8_t* pScanSource(pLittleEndianTagData);
        for(size_t scanBlocksY(0); scanBlocksY != blocksY; ++scanBlocksY )
        {
            samplesType_t* pScanImageSamples =
                    reinterpret_cast<samplesType_t*>(pImageSamples[0]->m_pBuffer) + scanBlocksY * imageWidth * maxSamplingFactorY;
            for(size_t scanBlocksX(0); scanBlocksX != blocksX; ++scanBlocksX )
            {
                for(size_t scanInsideBlockY(0); scanInsideBlockY != maxSamplingFactorY; ++scanInsideBlockY)
                {
                    for(size_t scanInsideBlockX(0); scanInsideBlockX != maxSamplingFactorX; ++scanInsideBlockX)
                    {
                        samplesType_t pixel(0);
                        for(std::uint32_t shiftLeft(0); shiftLeft != allocatedBytes; ++shiftLeft)
                        {
                            pixel = pixel | (static_cast<samplesType_t>(*(pScanSource++)) << (shiftLeft * 8));
                        }
                        *(pScanImageSamples + scanInsideBlockX + scanInsideBlockY * imageWidth) = pixel;
                    }
                }
                pScanImageSamples += maxSamplingFactorX;
                for(size_t channelNumber(1u); channelNumber < pImageSamples.size(); ++channelNumber)
                {
                    samplesType_t pixel(0);
                    for(std::uint32_t shiftLeft(0); shiftLeft != allocatedBytes; ++shiftLeft)
                    {
                        pixel = pixel | (static_cast<samplesType_t>(*(pScanSource++)) << (shiftLeft * 8));
                    }
                    pImageSamples[channelNumber]->m_pBuffer[scanBlocksY * blocksX + scanBlocksX] = pixel;
                }
            }
        }
    }


    template<typename samplesType_t> static void write1bitInterleaved(
            const samplesType_t* pImageSamples,
            std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels)
    {
        size_t leftShift(0);
        for(size_t numSamples(numPixels * numChannels); numSamples != 0; --numSamples)
        {
            *pLittleEndianTagData = *pLittleEndianTagData | static_cast<std::uint8_t>((*pImageSamples++ & 1) << leftShift);
            if(++leftShift == 8)
            {
                leftShift = 0;
                ++pLittleEndianTagData;
            }
        }
    }

    static void write1bitInterleaved(
            const std::uint8_t* pImageSamples,
            bitDepth_t samplesDepth,
            std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels);

    template<typename samplesType_t> static void write1bitNotInterleaved(
            const samplesType_t* pImageSamples,
            std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels)
    {
        size_t leftShift(0);
        for(size_t channel(0); channel != numChannels; ++channel)
        {
            const samplesType_t* pScanImageSamples(pImageSamples + channel);
            for(size_t numSamples(numPixels); numSamples != 0; --numSamples)
            {
                *pLittleEndianTagData = *pLittleEndianTagData | static_cast<std::uint8_t>((*pScanImageSamples & 1) << leftShift);
                pScanImageSamples += numChannels;
                if(++leftShift == 8)
                {
                    leftShift = 0;
                    ++pLittleEndianTagData;
                }
            }
        }
    }

    static void write1bitNotInterleaved(
            const std::uint8_t* pImageSamples,
            bitDepth_t samplesDepth,
            std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels);

    template<typename samplesType_t> static void read1bitInterleaved(
            samplesType_t* pImageSamples,
            const std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels)
    {
        size_t rightShift(0);
        for(size_t numSamples(numPixels * numChannels); numSamples != 0; --numSamples)
        {
            *pImageSamples++ = (*pLittleEndianTagData >> rightShift) & 1;
            if(++rightShift == 8)
            {
                rightShift = 0;
                ++pLittleEndianTagData;
            }
        }
    }

    static void read1bitInterleaved(
            std::uint8_t* pImageSamples,
            bitDepth_t samplesDepth,
            const std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels);

    template<typename samplesType_t> static void read1bitNotInterleaved(
            samplesType_t* pImageSamples,
            const std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels)
    {
        size_t rightShift(0);
        for(size_t channel(0); channel != numChannels; ++channel)
        {
            samplesType_t* pScanImageSamples(pImageSamples + channel);
            for(size_t numSamples(numPixels); numSamples != 0; --numSamples)
            {
                *pScanImageSamples = (*pLittleEndianTagData >> rightShift) & 1;
                pScanImageSamples += numChannels;
                if(++rightShift == 8)
                {
                    rightShift = 0;
                    ++pLittleEndianTagData;
                }
            }
        }
    }

    static void read1bitNotInterleaved(
            std::uint8_t* pImageSamples,
            bitDepth_t samplesDepth,
            const std::uint8_t* pLittleEndianTagData,
            size_t numPixels,
            size_t numChannels);

};


/// @}

} // namespace codecs

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDicomNativeImageCodec_CC44A2C5_2B8C_42c1_9704_3F9C582643B9__INCLUDED_)
