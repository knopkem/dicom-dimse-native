/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file VOILUT.h
    \brief Declaration of the class VOILUT.

*/

#if !defined(imebraVOILUT_8347C70F_1FC8_4df8_A887_8DE9E968B2CF__INCLUDED_)
#define imebraVOILUT_8347C70F_1FC8_4df8_A887_8DE9E968B2CF__INCLUDED_

#include "imageImpl.h"
#include "LUTImpl.h"
#include "transformImpl.h"
#include <string>
#include <cmath>


namespace imebra
{

namespace implementation
{

class lut;
class VOIDescription;

namespace transforms
{

/// \addtogroup group_transforms
///
/// @{

///////////////////////////////////////////////////////////
/// \brief Transforms the value of the input image's pixels
///         by using the presentation VOI/LUT defined in
///         the dataSet.
///
/// The dataSet could define more than one VOI/LUT: by
///  default the transform uses the first VOI or LUT
///  defined in the dataset.
///
/// To control which VOI/LUT is used for the
///  transformation the application must use the functions
///  getVOILUTId(), getVOILUTDescription(), setVOILUT() or
///  set the VOI or the LUT directly by calling
///  setCenterWidth() or setLUT().
///
///////////////////////////////////////////////////////////
class VOILUT: public transforms::transform
{
public:
    /// \brief Constructor.
    ///
    /// @param pDataSet the dataset from which the input image
    ///        comes from
    ///
    ///////////////////////////////////////////////////////////
    VOILUT(): m_windowCenter(0), m_windowWidth(0), m_function(dicomVOIFunction_t::linear)
    {}

    VOILUT(double center, double width, dicomVOIFunction_t function);

    VOILUT(const std::shared_ptr<lut>& pLut);

    /// \brief Finds and apply the optimal VOI values.
    ///
    /// @param inputImage    the image for which the optimal
    ///                      VOI must be found
    /// @param inputTopLeftX the horizontal coordinate of the
    ///                       top-left corner of the area for
    ///                       which the optimal VOI must be
    ///                       found
    /// @param inputTopLeftY the vertical coordinate of the
    ///                       top-left corner of the area for
    ///                       which the optimal VOI must be
    ///                       found
    /// @param inputWidth    the width of the area for which
    ///                       the optimal VOI must be found
    /// @param inputHeight   the height of the area for which
    ///                       the optimal VOI must be found
    ///
    ///////////////////////////////////////////////////////////
    static std::shared_ptr<VOIDescription> getOptimalVOI(const std::shared_ptr<imebra::implementation::image>& inputImage, std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight);

    DEFINE_RUN_TEMPLATE_TRANSFORM;

    // The actual transformation is done here
    //
    ///////////////////////////////////////////////////////////
    template <class inputType, class outputType>
            void templateTransform(
                    const inputType* inputHandlerData,
                    outputType* outputHandlerData,
                    bitDepth_t /* inputDepth */, std::uint32_t inputHandlerWidth, const std::string& /* inputHandlerColorSpace */,
                    std::shared_ptr<palette> /* inputPalette */,
                    std::uint32_t /* inputHighBit */,
                    std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight,
                    bitDepth_t /* outputDepth */, std::uint32_t outputHandlerWidth, const std::string& /* outputHandlerColorSpace */,
                    std::shared_ptr<palette> /* outputPalette */,
                    std::uint32_t outputHighBit,
                    std::uint32_t outputTopLeftX, std::uint32_t outputTopLeftY) const

    {
        IMEBRA_FUNCTION_START();

        const inputType* pInputMemory(inputHandlerData);
        outputType* pOutputMemory(outputHandlerData);

        pInputMemory += inputTopLeftY * inputHandlerWidth + inputTopLeftX;
        pOutputMemory += outputTopLeftY * outputHandlerWidth + outputTopLeftX;

        std::int64_t outputHandlerMinValue = getMinValue<outputType>(outputHighBit);

        //
        // LUT found
        //
        ///////////////////////////////////////////////////////////
        if(m_pLUT != 0 && m_pLUT->getSize() != 0)
        {
            for(; inputHeight != 0; --inputHeight)
            {
                for(std::uint32_t scanPixels(inputWidth); scanPixels != 0; --scanPixels)
                {
                    *(pOutputMemory++) = (outputType)( outputHandlerMinValue + m_pLUT->getMappedValue((std::int32_t)*pInputMemory++ ));
                }
                pInputMemory += (inputHandlerWidth - inputWidth);
                pOutputMemory += (outputHandlerWidth - inputWidth);
            }
            return;
        }

        //
        // LUT not found.
        // Use the window's center/width
        //
        ///////////////////////////////////////////////////////////
        std::int64_t outputHandlerNumValues = (std::int64_t)1 << (outputHighBit + 1);
        std::int64_t outputMin(outputHandlerMinValue);
        std::int64_t outputMax(outputHandlerMinValue + outputHandlerNumValues - 1);
        double outputMaxMinusOutputMin = (double)(outputMax - outputMin);
        double inputValue;
        std::int64_t outputValue;

        switch(m_function)
        {
        case dicomVOIFunction_t::linearExact:
            {
                for(; inputHeight != 0; --inputHeight)
                {
                    for(std::uint32_t scanPixels(inputWidth); scanPixels != 0; --scanPixels)
                    {
                        inputValue = ((double)*(pInputMemory++));
                        outputValue = (std::int64_t)(((inputValue - m_windowCenter) / m_windowWidth) * outputMaxMinusOutputMin + (double)outputMin);
                        if(outputValue < outputMin)
                        {
                            *pOutputMemory++ = (outputType)outputMin;
                        }
                        else if(outputValue > outputMax)
                        {
                            *pOutputMemory++ = (outputType)outputMax;
                        }
                        else
                        {
                            *pOutputMemory++ = (outputType)outputValue;
                        }
                    }

                    pInputMemory += (inputHandlerWidth - inputWidth);
                    pOutputMemory += (outputHandlerWidth - inputWidth);
                }
            }
            break;
        case dicomVOIFunction_t::sigmoid:
            {
                for(; inputHeight != 0; --inputHeight)
                {
                    for(std::uint32_t scanPixels(inputWidth); scanPixels != 0; --scanPixels)
                    {
                        inputValue = ((double)*(pInputMemory++));
                        outputValue = (std::int64_t)(outputMaxMinusOutputMin/(1.0 + std::exp(-4.0 * (inputValue - m_windowCenter)/ m_windowWidth)));
                        if(outputValue < outputMin)
                        {
                            *pOutputMemory++ = (outputType)outputMin;
                        }
                        else if(outputValue > outputMax)
                        {
                            *pOutputMemory++ = (outputType)outputMax;
                        }
                        else
                        {
                            *pOutputMemory++ = (outputType)outputValue;
                        }
                    }

                    pInputMemory += (inputHandlerWidth - inputWidth);
                    pOutputMemory += (outputHandlerWidth - inputWidth);
                }
            }
            break;
        case dicomVOIFunction_t::linear:
            {
                if(m_windowWidth <= 1)
                {
                    for(; inputHeight != 0; --inputHeight)
                    {
                        for(std::uint32_t scanPixels(inputWidth); scanPixels != 0; --scanPixels)
                        {
                            inputValue = ((double)*(pInputMemory++));
                            if(inputValue <= m_windowCenter - 0.5)
                            {
                                *pOutputMemory++ = (outputType)outputMin;
                            }
                            else
                            {
                                *pOutputMemory++ = (outputType)outputMax;
                            }
                        }
                        pInputMemory += (inputHandlerWidth - inputWidth);
                        pOutputMemory += (outputHandlerWidth - inputWidth);
                    }
                }
                else
                {
                    for(; inputHeight != 0; --inputHeight)
                    {
                        for(std::uint32_t scanPixels(inputWidth); scanPixels != 0; --scanPixels)
                        {
                            inputValue = ((double)*(pInputMemory++));
                            outputValue = (std::int64_t)(((inputValue - (m_windowCenter - 0.5)) / (m_windowWidth - 1.0) + 0.5) * outputMaxMinusOutputMin + (double)outputMin);
                            if(outputValue < outputMin)
                            {
                                *pOutputMemory++ = (outputType)outputMin;
                            }
                            else if(outputValue > outputMax)
                            {
                                *pOutputMemory++ = (outputType)outputMax;
                            }
                            else
                            {
                                *pOutputMemory++ = (outputType)outputValue;
                            }
                        }

                        pInputMemory += (inputHandlerWidth - inputWidth);
                        pOutputMemory += (outputHandlerWidth - inputWidth);
                    }
                }
            }
            break;
        }

        IMEBRA_FUNCTION_END();
    }

    virtual bool isEmpty() const override;

    virtual std::shared_ptr<image> allocateOutputImage(
            bitDepth_t inputDepth,
            const std::string& inputColorSpace,
            std::uint32_t inputHighBit,
            std::shared_ptr<palette> inputPalette,
            std::uint32_t outputWidth, std::uint32_t outputHeight) const override;

protected:

    // Find the optimal VOI
    //
    ///////////////////////////////////////////////////////////
    template <class inputType> static
            void templateFindOptimalVOI(
                    inputType* inputHandlerData, size_t /* inputHandlerSize */, std::uint32_t inputHandlerWidth,
                    std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight,
                    std::shared_ptr<VOIDescription>& voiDescription)
    {
        IMEBRA_FUNCTION_START();

        inputType* pInputMemory(inputHandlerData + inputHandlerWidth * inputTopLeftY + inputTopLeftX);
        inputType minValue(*pInputMemory);
        inputType maxValue(minValue);
        inputType value;
        for(std::uint32_t scanY(inputHeight); scanY != 0; --scanY)
        {
            for(std::uint32_t scanX(inputWidth); scanX != 0; --scanX)
            {
                value = *(pInputMemory++);
                if(value < minValue)
                {
                    minValue = value;
                }
                else if(value > maxValue)
                {
                    maxValue = value;
                }
            }
            pInputMemory += inputHandlerWidth - inputWidth;
        }

        double center = (double)((std::int64_t)maxValue + (std::int64_t)minValue + 1) / 2;
        double width = 2.0 * (center - (double)minValue);
        voiDescription = std::make_shared<VOIDescription>(
                    center,
                    width,
                    dicomVOIFunction_t::linear,
                    "");

        IMEBRA_FUNCTION_END();

    }

    std::shared_ptr<const lut> m_pLUT;
    double m_windowCenter;
    double m_windowWidth;
    dicomVOIFunction_t m_function;
};

/// @}

} // namespace transforms

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraVOILUT_8347C70F_1FC8_4df8_A887_8DE9E968B2CF__INCLUDED_)
