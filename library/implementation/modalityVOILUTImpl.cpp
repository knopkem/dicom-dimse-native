/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file modalityVOILUT.cpp
    \brief Implementation of the class modalityVOILUT.

*/

#include "exceptionImpl.h"
#include "modalityVOILUTImpl.h"
#include "dataSetImpl.h"
#include "colorTransformsFactoryImpl.h"
#include <math.h>
#include <limits>

namespace imebra
{

namespace implementation
{

namespace transforms
{


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Modality VOILUT transform
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
modalityVOILUT::modalityVOILUT(std::shared_ptr<const dataSet> pDataSet):
        m_pDataSet(pDataSet), m_voiLut(0), m_rescaleIntercept(pDataSet->getDouble(0x0028, 0, 0x1052, 0, 0, 0)), m_rescaleSlope(1.0), m_bEmpty(true)

{
    IMEBRA_FUNCTION_START();

    try
    {
        m_rescaleSlope = m_pDataSet->getDouble(0x0028, 0, 0x1053, 0x0, 0);
        m_bEmpty = false;
    }
    catch(const MissingDataElementError&)
    {
        try
        {
            m_voiLut = pDataSet->getLut(0x0028, 0x3000, 0);
            m_bEmpty = m_voiLut->getSize() == 0;
        }
        catch(const MissingDataElementError&)
        {
            // Nothing to do. Transform is empty
        }

    }

    IMEBRA_FUNCTION_END();
}

bool modalityVOILUT::isEmpty() const
{
    return m_bEmpty;
}


std::shared_ptr<image> modalityVOILUT::allocateOutputImage(
        bitDepth_t inputDepth,
        const std::string& inputColorSpace,
        std::uint32_t inputHighBit,
        std::shared_ptr<palette> /* inputPalette */,
        std::uint32_t outputWidth, std::uint32_t outputHeight) const
{
    IMEBRA_FUNCTION_START();

    if(!colorTransforms::colorTransformsFactory::isMonochrome(inputColorSpace))
    {
        IMEBRA_THROW(ModalityVOILUTError, "modalityVOILUT can process only monochromatic images");
    }

    if(isEmpty())
    {
        return std::make_shared<image>(outputWidth, outputHeight, inputDepth, inputColorSpace, inputHighBit);
    }

    // LUT
    ///////////////////////////////////////////////////////////
    if(m_voiLut != 0 && m_voiLut->getSize() != 0)
    {
        std::uint8_t bits(m_voiLut->getBits());

        bitDepth_t depth;
        if(bits > 8)
        {
            depth = bitDepth_t::depthU16;
        }
        else
        {
            depth = bitDepth_t::depthU8;
        }

        return std::make_shared<image>(outputWidth, outputHeight, depth, inputColorSpace, bits - 1);
    }

    // Rescale
    ///////////////////////////////////////////////////////////
    if(fabs(m_rescaleSlope) <= std::numeric_limits<double>::denorm_min())
    {
        return std::make_shared<image>(outputWidth, outputHeight, inputDepth, inputColorSpace, inputHighBit);
    }

    std::int32_t value0 = 0;
    std::int32_t value1 = ((std::int32_t)1 << (inputHighBit + 1)) - 1;
    if(inputDepth == bitDepth_t::depthS16 || inputDepth == bitDepth_t::depthS8)
    {
        value0 = (std::int32_t)((std::int32_t)-1 * ((std::int32_t)1 << inputHighBit));
        value1 = ((std::int32_t)1 << inputHighBit);
    }
    std::int32_t finalValue0((std::int32_t) ((double)value0 * m_rescaleSlope + m_rescaleIntercept) );
    std::int32_t finalValue1((std::int32_t) ((double)value1 * m_rescaleSlope + m_rescaleIntercept) );

    std::int32_t minValue, maxValue;
    if(finalValue0 < finalValue1)
    {
        minValue = finalValue0;
        maxValue = finalValue1;
    }
    else
    {
        minValue = finalValue1;
        maxValue = finalValue0;
    }

    if(minValue >= 0 && maxValue <= 255)
    {
        return std::make_shared<image>(outputWidth, outputHeight, bitDepth_t::depthU8, inputColorSpace, 7);
    }
    if(minValue >= -128 && maxValue <= 127)
    {
        return std::make_shared<image>(outputWidth, outputHeight, bitDepth_t::depthS8, inputColorSpace, 7);
    }
    if(minValue >= 0 && maxValue <= 65535)
    {
        return std::make_shared<image>(outputWidth, outputHeight, bitDepth_t::depthU16, inputColorSpace, 15);
    }
    if(minValue >= -32768 && maxValue <= 32767)
    {
        return std::make_shared<image>(outputWidth, outputHeight, bitDepth_t::depthS16, inputColorSpace, 15);
    }

    return std::make_shared<image>(outputWidth, outputHeight, bitDepth_t::depthS32, inputColorSpace, 31);

    IMEBRA_FUNCTION_END();
}



} // namespace transforms

} // namespace implementation

} // namespace imebra
