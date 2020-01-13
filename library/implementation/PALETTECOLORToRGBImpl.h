/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file PALETTECOLORToRGB.h
    \brief Declaration of the class PALETTECOLORToRGB.

*/

#if !defined(imebraPALETTECOLORToRGB_E27C63E7_A907_4899_9BD3_8026AD7D110C__INCLUDED_)
#define imebraPALETTECOLORToRGB_E27C63E7_A907_4899_9BD3_8026AD7D110C__INCLUDED_

#include "colorTransformImpl.h"
#include "LUTImpl.h"
#include "dataSetImpl.h"

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
///         PALETTE COLOR into the color space RGB.
///
/// The input image has to have the colorspace
///  PALETTE COLOR, while the output image is created by 
///  the transform and will have the colorspace RGB.
///
/// The transform needs a dataSet or a palette in order
///  to perform the conversion.
/// Use declareDataSet() to declare the dataSet or setLut()
///  to declare the palette.
///
///////////////////////////////////////////////////////////
class PALETTECOLORToRGB: public colorTransform
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
            std::shared_ptr<palette> inputPalette,
            std::uint32_t /* inputHighBit */,
            std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight,
            bitDepth_t /* outputDepth */, std::uint32_t outputHandlerWidth, const std::string& outputHandlerColorSpace,
            std::shared_ptr<palette> /* outputPalette */,
            std::uint32_t outputHighBit,
            std::uint32_t outputTopLeftX, std::uint32_t outputTopLeftY) const

    {
        IMEBRA_FUNCTION_START();

        checkColorSpaces(inputHandlerColorSpace, outputHandlerColorSpace);
        std::uint32_t inputHighBit = inputPalette->getRed()->getBits() - 1;
        checkHighBit(inputHighBit, outputHighBit);

        const inputType* pInputMemory(inputHandlerData);
        outputType* pOutputMemory(outputHandlerData);

        lut* pRed = inputPalette->getRed().get();
        lut* pGreen = inputPalette->getGreen().get();
        lut* pBlue = inputPalette->getBlue().get();

        pInputMemory += inputTopLeftY * inputHandlerWidth + inputTopLeftX;
        pOutputMemory += (outputTopLeftY * outputHandlerWidth + outputTopLeftX) * 3;

        std::int64_t outputHandlerMinValue = getMinValue<outputType>(outputHighBit);

        std::int32_t paletteValue;
        for(; inputHeight != 0; --inputHeight)
        {
            for(std::uint32_t scanPixels(inputWidth); scanPixels != 0; --scanPixels)
            {
                paletteValue = (std::int32_t) (*pInputMemory++);
                *pOutputMemory++ = (outputType)(pRed->getMappedValue(paletteValue) + outputHandlerMinValue);
                *pOutputMemory++ = (outputType)(pGreen->getMappedValue(paletteValue) + outputHandlerMinValue);
                *pOutputMemory++ = (outputType)(pBlue->getMappedValue(paletteValue) + outputHandlerMinValue);
            }
            pInputMemory += inputHandlerWidth - inputWidth;
            pOutputMemory += (outputHandlerWidth - inputWidth) * 3;
        }

        IMEBRA_FUNCTION_END();
    }

};

/// @}

} // namespace colorTransforms

} // namespace transforms

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraPALETTECOLORToRGB_E27C63E7_A907_4899_9BD3_8026AD7D110C__INCLUDED_)
