/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file RGBToYBRFULL.h
    \brief Declaration of the class RGBToYBRFULL.

*/

#if !defined(imebraRGBToYBRFULL_E27C63E7_A907_4899_9BD3_8026AD7D110C__INCLUDED_)
#define imebraRGBToYBRFULL_E27C63E7_A907_4899_9BD3_8026AD7D110C__INCLUDED_

#include "colorTransformImpl.h"


namespace imebra
{

namespace implementation
{

namespace transforms
{

namespace colorTransforms
{

/// \addtogroup group_transforms
///
/// @{

///////////////////////////////////////////////////////////
/// \brief Transforms an image from the colorspace 
///         RGB into the color space YBR_FULL.
///
/// The input image has to have the colorspace RGB,
///  while the output image is created by the transform
///  and will have the colorspace YBR_FULL.
///
///////////////////////////////////////////////////////////
class RGBToYBRFULL: public colorTransform
{
public:
    virtual std::string getInitialColorSpace() const override;
    virtual std::string getFinalColorSpace() const override;

    DEFINE_RUN_TEMPLATE_TRANSFORM;

    template <class inputType, class outputType>
    void templateTransform(
            const inputType* inputHandlerData,
            outputType* outputHandlerData,
            bitDepth_t /* inputDepth */, std::uint32_t inputHandlerWidth, const std::string& inputHandlerColorSpace,
            std::shared_ptr<palette> /* inputPalette */,
            std::uint32_t inputHighBit,
            std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight,
            bitDepth_t /* outputDepth */, std::uint32_t outputHandlerWidth, const std::string& outputHandlerColorSpace,
            std::shared_ptr<palette> /* outputPalette */,
            std::uint32_t outputHighBit,
            std::uint32_t outputTopLeftX, std::uint32_t outputTopLeftY) const

    {
        IMEBRA_FUNCTION_START();

        checkColorSpaces(inputHandlerColorSpace, outputHandlerColorSpace);
        checkHighBit(inputHighBit, outputHighBit);

        const inputType* pInputMemory(inputHandlerData);
        outputType* pOutputMemory(outputHandlerData);

        pInputMemory += (inputTopLeftY * inputHandlerWidth + inputTopLeftX) * 3;
        pOutputMemory += (outputTopLeftY * outputHandlerWidth + outputTopLeftX) * 3;

        std::int64_t inputHandlerMinValue = getMinValue<inputType>(inputHighBit);
        std::int64_t outputHandlerMinValue = getMinValue<outputType>(outputHighBit);
        std::int64_t outputMiddleValue(outputHandlerMinValue + ((std::int64_t)1 << outputHighBit));

        std::int64_t sourceR, sourceG, sourceB;
        for(; inputHeight != 0; --inputHeight)
        {
            for(std::uint32_t scanPixels(inputWidth); scanPixels != 0; --scanPixels)
            {
                sourceR = (std::int64_t)*pInputMemory++ - inputHandlerMinValue;
                sourceG = (std::int64_t)*pInputMemory++ - inputHandlerMinValue;
                sourceB = (std::int64_t)*pInputMemory++ - inputHandlerMinValue;
                *(pOutputMemory++) = (outputType) ( ((4899 * sourceR + 9617 * sourceG + 1868 * sourceB) / 16384) + outputHandlerMinValue );
                *(pOutputMemory++) = (outputType) ( outputMiddleValue + ((8192 * sourceB - 2765 * sourceR - 5427 * sourceG) / 16384));
                *(pOutputMemory++) = (outputType) ( outputMiddleValue + ((8192 * sourceR - 6860 * sourceG - 1332 * sourceB) / 16384));
            }
            pInputMemory += (inputHandlerWidth - inputWidth) * 3;
            pOutputMemory += (outputHandlerWidth - inputWidth) * 3;
        }

        IMEBRA_FUNCTION_END();
    }
};


///////////////////////////////////////////////////////////
/// \brief Transforms an image from the colorspace
///         RGB into the color space YBR_ICT.
///
/// The input image has to have the colorspace RGB,
///  while the output image is created by the transform
///  and will have the colorspace YBR_ICT.
///
///////////////////////////////////////////////////////////
class RGBToYBRICT: public RGBToYBRFULL
{
public:

    virtual std::string getFinalColorSpace() const override;

};

/// @}

} // namespace colorTransforms

} // namespace transforms

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraRGBToYBRFULL_E27C63E7_A907_4899_9BD3_8026AD7D110C__INCLUDED_)
