/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerStringAS.cpp
    \brief Implementation of the class dataHandlerStringAS.

*/

#include <sstream>
#include <string>
#include <iomanip>
#include "../include/imebra/exceptions.h"
#include "ageImpl.h"
#include "exceptionImpl.h"
#include "dataHandlerStringASImpl.h"
#include "memoryImpl.h"
#include <memory.h>

namespace imebra
{

namespace implementation
{

namespace handlers
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// dataHandlerStringAS
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
// Constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
readingDataHandlerStringAS::readingDataHandlerStringAS(const memory& parseMemory): readingDataHandlerString(parseMemory, tagVR_t::AS, '\\', 0x20)
{
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the age
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<age> readingDataHandlerStringAS::getAge(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    std::string ageString = getString(index);
    if(ageString.size() != 4)
    {
        IMEBRA_THROW(DataHandlerCorruptedBufferError, "The AGE string should be 4 bytes long but it is "<< ageString.size() << " bytes long");
    }
    std::istringstream ageStream(ageString);
    std::uint32_t ageValue;
    if(!(ageStream >> ageValue))
    {
        IMEBRA_THROW(DataHandlerCorruptedBufferError, "The AGE is not a number");
    }
    char unit = ageString[3];
    if(
            unit != (char)ageUnit_t::days &&
            unit != (char)ageUnit_t::weeks &&
            unit == (char)ageUnit_t::months &&
            unit == (char)ageUnit_t::years)
    {
        IMEBRA_THROW(DataHandlerCorruptedBufferError, "The AGE unit should be D, W, M or Y but is ascii code "<< (int)unit);
    }
    return std::make_shared<age>(ageValue, (ageUnit_t)unit);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the age in years as a signed long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::int32_t readingDataHandlerStringAS::getSignedLong(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    return (std::int32_t)getDouble(index);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the age in years as an unsigned long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t readingDataHandlerStringAS::getUnsignedLong(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    return (std::uint32_t)getDouble(index);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the age in years as a double
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
double readingDataHandlerStringAS::getDouble(const size_t /* index */) const
{
    IMEBRA_FUNCTION_START();

    IMEBRA_THROW(DataHandlerConversionError, "Cannot convert an Age to a number")

    IMEBRA_FUNCTION_END();
}


writingDataHandlerStringAS::writingDataHandlerStringAS(const std::shared_ptr<buffer> &pBuffer):
    writingDataHandlerString(pBuffer, tagVR_t::AS, '\\', 4, 4)
{
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the age
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void writingDataHandlerStringAS::setAge(const size_t index, const std::shared_ptr<const age>& pAge)
{
    IMEBRA_FUNCTION_START();

    if(index >= getSize())
    {
        setSize(index + 1);
    }

    std::ostringstream ageStream;
    ageStream << std::setfill('0');
    ageStream << std::setw(3) << pAge->getAgeValue();
    ageStream << std::setw(1) << (char)pAge->getAgeUnits();

    setString(index, ageStream.str());

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the age in years as a signed long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void writingDataHandlerStringAS::setSignedLong(const size_t index, const std::int32_t value)
{
    IMEBRA_FUNCTION_START();

    setDouble(index, (double)value);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the age in years as an unsigned long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void writingDataHandlerStringAS::setUnsignedLong(const size_t index, const std::uint32_t value)
{
    IMEBRA_FUNCTION_START();

    setDouble(index, (double)value);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the age in years as a double
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void writingDataHandlerStringAS::setDouble(const size_t /* index */, const double /* value */)
{
    IMEBRA_FUNCTION_START();

    IMEBRA_THROW(DataHandlerConversionError, "Cannot convert to VR AS from double");

    IMEBRA_FUNCTION_END();
}

void writingDataHandlerStringAS::validate() const
{
    IMEBRA_FUNCTION_START();

    memory parseMemory(m_strings[0].size());
    ::memcpy(parseMemory.data(), m_strings[0].data(), parseMemory.size());
    try
    {
        readingDataHandlerStringAS readingHandler(parseMemory);
    }
    catch(const DataHandlerCorruptedBufferError& e)
    {
        IMEBRA_THROW(DataHandlerConversionError, e.what());
    }
    writingDataHandlerString::validate();

    IMEBRA_FUNCTION_END();
}


} // namespace handlers

} // namespace implementation

} // namespace imebra
