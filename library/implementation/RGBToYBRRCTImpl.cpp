/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file RGBToYBRRCT.cpp
    \brief Implementation of the class RGBToYBRRCT.

*/

#include "exceptionImpl.h"
#include "RGBToYBRRCTImpl.h"
#include "dataHandlerImpl.h"
#include "dataSetImpl.h"
#include "imageImpl.h"

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
//
//
// Return the initial color space
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string RGBToYBRRCT::getInitialColorSpace() const
{
    return "RGB";
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the final color space (YBR_FULL)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string RGBToYBRRCT::getFinalColorSpace() const
{
    return "YBR_RCT";
}

void RGBToYBRRCT::checkHighBit(std::uint32_t inputHighBit, std::uint32_t outputHighBit) const
{
    IMEBRA_FUNCTION_START();

    if(inputHighBit + 1 != outputHighBit)
    {
        IMEBRA_THROW(TransformDifferentHighBitError, "Different high bit (input = " << inputHighBit << ", output = " << outputHighBit << ")");
    }

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<image> RGBToYBRRCT::allocateOutputImage(
        bitDepth_t inputDepth,
        const std::string& /* inputColorSpace */,
        std::uint32_t inputHighBit,
        std::shared_ptr<palette> /* inputPalette */,
        std::uint32_t outputWidth, std::uint32_t outputHeight) const
{
    IMEBRA_FUNCTION_START();

    std::uint32_t outputHighBit = inputHighBit + 1;
    bitDepth_t outputDepth(inputDepth);
    if(((int)inputDepth & 1) == 0)
    {
        if(outputHighBit >= 16)
        {
            outputDepth = bitDepth_t::depthU32;
        }
        else if(outputHighBit >= 8)
        {
            outputDepth = bitDepth_t::depthU16;
        }
        else
        {
            outputDepth = bitDepth_t::depthU8;
        }
    }
    else
    {
        if(outputHighBit >= 16)
        {
            outputDepth = bitDepth_t::depthS32;
        }
        else if(outputHighBit >= 8)
        {
            outputDepth = bitDepth_t::depthS16;
        }
        else
        {
            outputDepth = bitDepth_t::depthS8;
        }
    }

    return std::make_shared<image>(outputWidth, outputHeight, outputDepth, getFinalColorSpace(), outputHighBit);

    IMEBRA_FUNCTION_END();
}



} // namespace colorTransforms

} // namespace transforms

} // namespace implementation

} // namespace imebra

