/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file VOILUT.cpp
    \brief Implementation of the class VOILUT.

*/

#include "exceptionImpl.h"
#include "VOILUTImpl.h"
#include "dataSetImpl.h"
#include "VOIDescriptionImpl.h"
#include "../include/imebra/exceptions.h"

namespace imebra
{

namespace implementation
{

namespace transforms
{

VOILUT::VOILUT(double center, double width, dicomVOIFunction_t function):
    m_pLUT(nullptr), m_windowCenter(center), m_windowWidth(width), m_function(function)
{
}

VOILUT::VOILUT(const std::shared_ptr<lut>& pLut):
    m_pLUT(pLut), m_windowCenter(0.0), m_windowWidth(0.0)
{
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns true if the transform is empty
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool VOILUT::isEmpty() const
{
    return m_windowWidth < 1.0 && (m_pLUT == nullptr || m_pLUT->getSize() == 0);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Allocate the output image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<image> VOILUT::allocateOutputImage(
        bitDepth_t inputDepth,
        const std::string& inputColorSpace,
        std::uint32_t inputHighBit,
        std::shared_ptr<palette> /* inputPalette */,
        std::uint32_t outputWidth, std::uint32_t outputHeight) const
{
    IMEBRA_FUNCTION_START();

    if(isEmpty())
    {
        return std::make_shared<image>(outputWidth, outputHeight, inputDepth, inputColorSpace, inputHighBit);
    }

    if(m_pLUT != 0 && m_pLUT->getSize() != 0)
    {
        std::uint8_t bits = m_pLUT->getBits();

        if(bits > 8)
        {
            inputDepth = bitDepth_t::depthU16;
        }
        else
        {
            inputDepth = bitDepth_t::depthU8;
        }
        return std::make_shared<image>(outputWidth, outputHeight, inputDepth, inputColorSpace, bits - 1);
    }

    //
    // LUT not found.
    // Use the window's center/width
    //
    ///////////////////////////////////////////////////////////
    if(m_windowWidth <= 1)
    {
        return std::make_shared<image>(outputWidth, outputHeight, inputDepth, inputColorSpace, inputHighBit);
    }

    if(inputDepth == bitDepth_t::depthS8)
        inputDepth = bitDepth_t::depthU8;
    if(inputDepth == bitDepth_t::depthS16 || inputDepth == bitDepth_t::depthU32 || inputDepth == bitDepth_t::depthS32)
        inputDepth = bitDepth_t::depthU16;

    return std::make_shared<image>(outputWidth, outputHeight, inputDepth, inputColorSpace, inputHighBit);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Finds and applies the optimal VOI.
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<VOIDescription> VOILUT::getOptimalVOI(const std::shared_ptr<imebra::implementation::image>& inputImage, std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight)
{
    IMEBRA_FUNCTION_START();

    std::uint32_t width, height;
    inputImage->getSize(&width, &height);

    if(inputTopLeftX + inputWidth > width || inputTopLeftY + inputHeight > height)
    {
        IMEBRA_THROW(TransformInvalidAreaError, "The input and/or output areas are invalid");
    }

    std::shared_ptr<handlers::readingDataHandlerNumericBase> handler(inputImage->getReadingDataHandler());
    std::shared_ptr<VOIDescription> voiDescription;
    HANDLER_CALL_TEMPLATE_FUNCTION_WITH_PARAMS(templateFindOptimalVOI, handler, width, inputTopLeftX, inputTopLeftY, inputWidth, inputHeight, voiDescription);
    return voiDescription;

    IMEBRA_FUNCTION_END();
}




} // namespace transforms

} // namespace implementation

} // namespace imebra
