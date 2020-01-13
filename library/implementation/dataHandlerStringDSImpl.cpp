/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerStringDS.cpp
    \brief Implementation of the class dataHandlerStringDS.

*/

#include <sstream>
#include <iomanip>

#include "exceptionImpl.h"
#include "dataHandlerStringDSImpl.h"

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
// dataHandlerStringDS
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

readingDataHandlerStringDS::readingDataHandlerStringDS(const memory& parseMemory): readingDataHandlerString(parseMemory, tagVR_t::DS, '\\', 0x20)
{
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the value as a signed long.
// Overwritten to use getDouble()
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::int32_t readingDataHandlerStringDS::getSignedLong(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    return (std::int32_t)getDouble(index);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the value as an unsigned long.
// Overwritten to use getDouble()
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t readingDataHandlerStringDS::getUnsignedLong(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    return (std::uint32_t)getDouble(index);

    IMEBRA_FUNCTION_END();
}


writingDataHandlerStringDS::writingDataHandlerStringDS(const std::shared_ptr<buffer> pBuffer):
    writingDataHandlerString(pBuffer, tagVR_t::DS, '\\', 0, 16)
{

}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the value as a signed long.
// Overwritten to use setDouble()
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void writingDataHandlerStringDS::setSignedLong(const size_t index, const std::int32_t value)
{
    IMEBRA_FUNCTION_START();

    setDouble(index, (double)value);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the value as an unsigned long.
// Overwritten to use setDouble()
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void writingDataHandlerStringDS::setUnsignedLong(const size_t index, const std::uint32_t value)
{
    IMEBRA_FUNCTION_START();

    setDouble(index, (double)value);

    IMEBRA_FUNCTION_END();
}

} // namespace handlers

} // namespace implementation

} // namespace imebra
