/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#include "../include/imebra/lut.h"
#include "../implementation/dataHandlerNumericImpl.h"
#include "../implementation/LUTImpl.h"

namespace imebra
{

LUT::LUT(const std::shared_ptr<implementation::lut>& pLut): m_pLut(pLut)
{
}

LUT::LUT(const LUT& source): m_pLut(getLUTImplementation(source))
{
}

const std::shared_ptr<imebra::implementation::lut>& getLUTImplementation(const LUT& lut)
{
    return lut.m_pLut;
}

LUT::~LUT()
{
}

std::wstring LUT::getDescription() const
{
    return m_pLut->getDescription();
}

ReadingDataHandlerNumeric LUT::getReadingDataHandler() const
{
    IMEBRA_FUNCTION_START();

    return ReadingDataHandlerNumeric(m_pLut->getReadingDataHandler());

    IMEBRA_FUNCTION_END_LOG();
}

size_t LUT::getBits() const
{
    IMEBRA_FUNCTION_START();

    return m_pLut->getBits();

    IMEBRA_FUNCTION_END_LOG();
}

size_t LUT:: getSize() const
{
    IMEBRA_FUNCTION_START();

    return m_pLut->getSize();

    IMEBRA_FUNCTION_END_LOG();
}

std::int32_t LUT::getFirstMapped() const
{
    IMEBRA_FUNCTION_START();

    return m_pLut->getFirstMapped();

    IMEBRA_FUNCTION_END_LOG();
}

std::uint32_t LUT::getMappedValue(std::int32_t index) const
{
    IMEBRA_FUNCTION_START();

    return m_pLut->getMappedValue(index);

    IMEBRA_FUNCTION_END_LOG();
}

}

