/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file MONOCHROME1ToMONOCHROME2.h
    \brief Declaration of the class MONOCHROME1ToMONOCHROME2.

*/

#if !defined(imebraMONOCHROME1ToMONOCHROME2_E27C63E7_A907_4899_9BD3_8026AD7D110C__INCLUDED_)
#define imebraMONOCHROME1ToMONOCHROME2_E27C63E7_A907_4899_9BD3_8026AD7D110C__INCLUDED_

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
///         MONOCHROME1 into the color space MONOCHROME2.
///
/// The input image has to have the colorspace MONOCHROME1,
///  while the output image is created by the transform
///  and will have the colorspace MONOCHROME2.
///
///////////////////////////////////////////////////////////
class MONOCHROME1ToMONOCHROME2: public colorTransform
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

        pInputMemory += inputTopLeftY * inputHandlerWidth + inputTopLeftX;
        pOutputMemory += outputTopLeftY * outputHandlerWidth + outputTopLeftX;

        std::int64_t inputHandlerMinValue = getMinValue<inputType>(inputHighBit);
        std::int64_t outputHandlerMinValue = getMinValue<outputType>(outputHighBit);
        std::int64_t inputHandlerNumValuesMinusOne = ((std::int64_t)1 << (inputHighBit + 1)) - 1;

        for(; inputHeight != 0; --inputHeight)
        {
            for(std::uint32_t scanPixels(inputWidth); scanPixels != 0; --scanPixels)
            {
                *pOutputMemory++ = (outputType)(outputHandlerMinValue + inputHandlerNumValuesMinusOne - (std::int64_t)*pInputMemory++ + inputHandlerMinValue);
            }
            pInputMemory += inputHandlerWidth - inputWidth;
            pOutputMemory += outputHandlerWidth - inputWidth;
        }

        IMEBRA_FUNCTION_END();
    }

};


///////////////////////////////////////////////////////////
/// \brief Transforms an image from the colorspace 
///         MONOCHROME2 into the color space MONOCHROME1.
///
/// The input image has to have the colorspace MONOCHROME2,
///  while the output image is created by the transform
///  and will have the colorspace MONOCHROME1.
///
///////////////////////////////////////////////////////////
class MONOCHROME2ToMONOCHROME1: public MONOCHROME1ToMONOCHROME2
{
public:
    virtual std::string getInitialColorSpace() const override;
    virtual std::string getFinalColorSpace() const override;
};

/// @}

} // namespace colorTransforms

} // namespace transforms

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraMONOCHROME1ToMONOCHROME2_E27C63E7_A907_4899_9BD3_8026AD7D110C__INCLUDED_)
