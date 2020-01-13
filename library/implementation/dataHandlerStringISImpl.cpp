/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerStringIS.cpp
    \brief Implementation of the class dataHandlerStringIS.

*/

#include "exceptionImpl.h"
#include "dataHandlerStringISImpl.h"

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
// dataHandlerStringIS
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

readingDataHandlerStringIS::readingDataHandlerStringIS(const memory& parseMemory): readingDataHandlerString(parseMemory, tagVR_t::IS, '\\', 0x20)
{
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a value as a double.
// Overwritten to use getSignedLong()
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
double readingDataHandlerStringIS::getDouble(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    return (double)getSignedLong(index);

    IMEBRA_FUNCTION_END();
}

writingDataHandlerStringIS::writingDataHandlerStringIS(const std::shared_ptr<buffer> pBuffer):
    writingDataHandlerString(pBuffer, tagVR_t::IS, '\\', 0, 12)
{

}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set a value as a double.
// Overwritten to use setSignedLong()
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void writingDataHandlerStringIS::setDouble(const size_t index, const double value)
{
    IMEBRA_FUNCTION_START();

    setSignedLong(index, (std::int32_t)value);

    IMEBRA_FUNCTION_END();
}

} // namespace handlers

} // namespace implementation

} // namespace imebra
