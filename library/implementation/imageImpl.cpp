/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file image.cpp
    \brief Implementation of the class image.

*/

#include "exceptionImpl.h"
#include "imageImpl.h"
#include "colorTransformsFactoryImpl.h"
#include "bufferImpl.h"
#include "../include/imebra/exceptions.h"

namespace imebra
{

namespace implementation
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// image
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

image::image(uint32_t width, uint32_t height, bitDepth_t depth, const std::string &colorSpace, uint32_t highBit):
    m_channelsNumber(0),
    m_imageDepth(depth),
    m_highBit(highBit),
    m_width(width),
    m_height(height)
{
    IMEBRA_FUNCTION_START();

    if(width == 0 || height == 0)
    {
        IMEBRA_THROW(ImageInvalidSizeError, "An invalid image's size has been specified");
    }

    // Normalize the color space (remove _420 & _422 and
    //  make it uppercase).
    ///////////////////////////////////////////////////////////
    m_colorSpace = transforms::colorTransforms::colorTransformsFactory::normalizeColorSpace(colorSpace);

    // Find the number of channels to allocate
    ///////////////////////////////////////////////////////////
    m_channelsNumber = transforms::colorTransforms::colorTransformsFactory::getNumberOfChannels(colorSpace);
    if(m_channelsNumber == 0)
    {
        IMEBRA_THROW(ImageUnknownColorSpaceError, "Cannot recognize the specified color space");
    }

    // Find the datatype to use to allocate the
    //  buffer (datatypes are in Dicom standard, plus SB
    //  for signed bytes).
    ///////////////////////////////////////////////////////////
    std::uint8_t defaultHighBit = 0;

    switch(depth)
    {
    case bitDepth_t::depthU8:
        m_bufferDataType = tagVR_t::OB;
        defaultHighBit=7;
        break;
    case bitDepth_t::depthS8:
        m_bufferDataType = tagVR_t::SB;
        defaultHighBit=7;
        break;
    case bitDepth_t::depthU16:
        m_bufferDataType = tagVR_t::US;
        defaultHighBit=15;
        break;
    case bitDepth_t::depthS16:
        m_bufferDataType = tagVR_t::SS;
        defaultHighBit=15;
        break;
    case bitDepth_t::depthU32:
        m_bufferDataType = tagVR_t::UL;
        defaultHighBit=31;
        break;
    case bitDepth_t::depthS32:
        m_bufferDataType = tagVR_t::SL;
        defaultHighBit=31;
        break;
    default:
        IMEBRA_THROW(ImageUnknownDepthError, "Unknown depth");
    }

    // Adjust the high bit value
    ///////////////////////////////////////////////////////////
    if(highBit>defaultHighBit)
        m_highBit=defaultHighBit;
    else
        m_highBit=highBit;

    IMEBRA_FUNCTION_END();
}

image::~image()
{

}


void image::setPalette(std::shared_ptr<palette> imagePalette)
{
    m_palette = imagePalette;
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve an handler to the image's buffer.
// The image's components are interleaved.
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<handlers::readingDataHandlerNumericBase> image::getReadingDataHandler() const
{
    IMEBRA_FUNCTION_START();

    // If a valid buffer with the same data type is already
    //  allocated then use it.
    ///////////////////////////////////////////////////////////
    if(m_buffer.get() == 0)
    {
        const std::shared_ptr<const charsetsList_t> pCharsets(std::make_shared<const charsetsList_t>());
        buffer temporaryBuffer(pCharsets);
        {
            std::shared_ptr<handlers::writingDataHandler> imageHandler(temporaryBuffer.getWritingDataHandler(m_bufferDataType, m_width * m_height * m_channelsNumber));
        }
        return std::dynamic_pointer_cast<handlers::readingDataHandlerNumericBase>(temporaryBuffer.getReadingDataHandler(m_bufferDataType));
    }

    return std::dynamic_pointer_cast<handlers::readingDataHandlerNumericBase>(m_buffer->getReadingDataHandler(m_bufferDataType));

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<handlers::writingDataHandlerNumericBase> image::getWritingDataHandler()
{
    IMEBRA_FUNCTION_START();

    // If a valid buffer with the same data type is already
    //  allocated then use it.
    ///////////////////////////////////////////////////////////
    if(m_buffer == 0)
    {
        const std::shared_ptr<const charsetsList_t> pCharsets(std::make_shared<const charsetsList_t>());
        m_buffer = std::make_shared<buffer>(pCharsets);
    }

    std::shared_ptr<handlers::writingDataHandler> imageHandler(m_buffer->getWritingDataHandler(m_bufferDataType, m_width * m_height * m_channelsNumber));

    return std::dynamic_pointer_cast<handlers::writingDataHandlerNumericBase>(imageHandler);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the bit depth
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bitDepth_t image::getDepth() const
{
    return m_imageDepth;
}


bool image::isSigned() const
{
    return m_imageDepth == bitDepth_t::depthS8 ||
            m_imageDepth == bitDepth_t::depthS16 ||
            m_imageDepth == bitDepth_t::depthS32;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the high bit
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t image::getHighBit() const
{
    return m_highBit;
}

std::shared_ptr<palette> image::getPalette() const
{
    return m_palette;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the colorspace
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string image::getColorSpace() const
{
    return m_colorSpace;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the numbero of allocated channels
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t image::getChannelsNumber() const
{
    return m_channelsNumber;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the image's size in pixels
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void image::getSize(std::uint32_t* pWidth, std::uint32_t* pHeight) const
{
    if(pWidth)
    {
        *pWidth = m_width;
    }

    if(pHeight)
    {
        *pHeight = m_height;
    }
}

} // namespace implementation

} // namespace imebra
