/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file colorTransform.cpp
    \brief Implementation of the base class for the color transforms.

*/

#include "exceptionImpl.h"
#include "colorTransformImpl.h"
#include "colorTransformsFactoryImpl.h"
#include "imageImpl.h"
#include "LUTImpl.h"
#include "../include/imebra/exceptions.h"

namespace imebra
{

namespace implementation
{

namespace transforms
{

namespace colorTransforms
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// colorTransform
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
// Transformation
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void colorTransform::checkColorSpaces(const std::string& inputHandlerColorSpace, const std::string& outputHandlerColorSpace) const
{
    IMEBRA_FUNCTION_START();

	if(inputHandlerColorSpace != getInitialColorSpace())
	{
        IMEBRA_THROW(ColorTransformWrongColorSpaceError, "The image's color space cannot be handled by the transform");
	}

	if(outputHandlerColorSpace != getFinalColorSpace())
	{
        IMEBRA_THROW(ColorTransformWrongColorSpaceError, "The image's color space cannot be handled by the transform");
	}

	IMEBRA_FUNCTION_END();
}

void colorTransform::checkHighBit(std::uint32_t inputHighBit, std::uint32_t outputHighBit) const
{
    IMEBRA_FUNCTION_START();

    if(inputHighBit != outputHighBit)
    {
        IMEBRA_THROW(TransformDifferentHighBitError, "Different high bit (input = " << inputHighBit << ", output = " << outputHighBit << ")");
    }

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<image> colorTransform::allocateOutputImage(
        bitDepth_t inputDepth,
        const std::string& /* inputColorSpace */,
        std::uint32_t inputHighBit,
        std::shared_ptr<palette> inputPalette,
        std::uint32_t outputWidth, std::uint32_t outputHeight) const
{
    IMEBRA_FUNCTION_START();

    if(inputPalette != 0)
    {
        std::uint8_t bits = inputPalette->getRed()->getBits();
        inputHighBit = bits - 1;
        if(bits > 8)
        {
            inputDepth = bitDepth_t::depthU16;
        }
        else
        {
            inputDepth = bitDepth_t::depthU8;
        }
    }

    return std::make_shared<image>(outputWidth, outputHeight, inputDepth, getFinalColorSpace(), inputHighBit);

    IMEBRA_FUNCTION_END();
}

} // namespace colorTransforms

} // namespace transforms

} // namespace implementation

} // namespace imebra
