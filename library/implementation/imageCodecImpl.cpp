/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file imageCodecImpl.cpp
    \brief Implementation of the base class for the image codecs.

*/

#include "imageCodecImpl.h"
#include "imageImpl.h"
#include "colorTransformsFactoryImpl.h"
#include "dataHandlerNumericImpl.h"
#include "exceptionImpl.h"
#include <string.h>


namespace imebra
{

namespace implementation
{

namespace codecs
{


std::vector<std::shared_ptr<channel>> imageCodec::allocChannels(std::uint32_t channelsNumber, std::uint32_t width, std::uint32_t height, bool bSubSampledX, bool bSubSampledY)
{
    IMEBRA_FUNCTION_START();

    if(bSubSampledX && (width & 0x1) != 0)
    {
        width++;
    }

    if(bSubSampledY && (height & 0x1) != 0)
    {
        height++;
    }

    std::vector<std::shared_ptr<channel>> channels;
    for(std::uint32_t channelNum = 0; channelNum < channelsNumber; ++channelNum)
    {
        std::uint32_t channelWidth = width;
        std::uint32_t channelHeight = height;
        std::uint32_t samplingFactorX = 1;
        std::uint32_t samplingFactorY = 1;
        if(channelNum != 0)
        {
            if(bSubSampledX)
            {
                channelWidth = width / 2;
            }
            if(bSubSampledY)
            {
                channelHeight = height / 2;
            }
        }
        else
        {
            if(bSubSampledX)
            {
                ++samplingFactorX;
            }
            if(bSubSampledY)
            {
                ++samplingFactorY;
            }
        }

        std::shared_ptr<channel> newChannel(std::make_shared<channel>());
        newChannel->allocate(channelWidth, channelHeight);
        newChannel->m_samplingFactorX = samplingFactorX;
        newChannel->m_samplingFactorY = samplingFactorY;

        channels.push_back(newChannel);
    }

    return channels;

    IMEBRA_FUNCTION_END();
}


std::vector<std::shared_ptr<channel>> imageCodec::splitImageIntoChannels(const std::shared_ptr<const image>& pImage, bool bSubSampledX, bool bSubSampledY)
{
    const std::uint32_t channelsNumber = pImage->getChannelsNumber();
    std::uint32_t imageWidth, imageHeight;
    pImage->getSize(&imageWidth, &imageHeight);
    const std::string colorSpace = pImage->getColorSpace();
    std::vector<std::shared_ptr<channel>> subsampledChannels = allocChannels(channelsNumber, imageWidth, imageHeight, bSubSampledX, bSubSampledY);
    std::uint32_t maxSamplingFactorX = bSubSampledX ? 2 : 1;
    std::uint32_t maxSamplingFactorY = bSubSampledY ? 2 : 1;
    std::shared_ptr<handlers::readingDataHandlerNumericBase> imageHandler = pImage->getReadingDataHandler();
    for(std::uint32_t copyChannels = 0; copyChannels < channelsNumber; ++copyChannels)
    {
        std::shared_ptr<channel> imageChannel = subsampledChannels[copyChannels];
        imageHandler->copyToInt32Interleaved(
                    imageChannel->m_pBuffer,
                    maxSamplingFactorX /imageChannel->m_samplingFactorX,
                    maxSamplingFactorY /imageChannel->m_samplingFactorY,
                    0, 0,
                    subsampledChannels[0]->m_width,
                    subsampledChannels[0]->m_height,
                    copyChannels,
                    imageWidth,
                    imageHeight,
                    channelsNumber);
    }

    return subsampledChannels;

}


void imageCodec::adjustB2Complement(
        std::uint8_t* pImageSamples,
        std::uint32_t highBit,
        bitDepth_t samplesDepth,
        size_t numSamples)
{
    IMEBRA_FUNCTION_START();

    switch(samplesDepth)
    {
    case bitDepth_t::depthS8:
        adjustB2Complement(reinterpret_cast<std::int8_t*>(pImageSamples), highBit, numSamples);
        break;
    case bitDepth_t::depthS16:
        adjustB2Complement(reinterpret_cast<std::int16_t*>(pImageSamples), highBit, numSamples);
        break;
    case bitDepth_t::depthS32:
        adjustB2Complement(reinterpret_cast<std::int32_t*>(pImageSamples), highBit, numSamples);
        break;
    default:
        IMEBRA_THROW(CodecCorruptedFileError, "Signed data on image with non-signed samples");

    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Allocate a channel's memory
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void channel::allocate(std::uint32_t width, std::uint32_t height)
{
    IMEBRA_FUNCTION_START();

    m_width = width;
    m_height = height;
    m_bufferSize = width * height;
    m_memory = std::make_shared<memory>(m_bufferSize * sizeof(std::int32_t) );
    m_pBuffer = reinterpret_cast<std::int32_t*>(m_memory->data());

    ::memset(m_pBuffer, 0, m_bufferSize * sizeof(std::int32_t));

    IMEBRA_FUNCTION_END();
}

} // namespace codecs

} // namespace implementation

} // namespace imebra

