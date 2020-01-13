/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerDate.cpp
    \brief Implementation of the dataHandlerDate class.

*/

#include <sstream>
#include <iomanip>

#include "exceptionImpl.h"
#include "dataHandlerDateImpl.h"
#include "dateImpl.h"

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
// dataHandlerDate
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

readingDataHandlerDate::readingDataHandlerDate(const memory& parseMemory): readingDataHandlerDateTimeBase(parseMemory, tagVR_t::DA)
{

}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the date
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<date> readingDataHandlerDate::getDate(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    std::uint32_t year(0), month(0), day(0);
    std::string dateString = getString(index);
    parseDate(dateString, &year, &month, &day);
    return std::make_shared<date>(year, month, day, 0, 0, 0, 0, 0, 0);

    IMEBRA_FUNCTION_END();
}


writingDataHandlerDate::writingDataHandlerDate(const std::shared_ptr<buffer> &pBuffer):
    writingDataHandlerDateTimeBase(pBuffer, tagVR_t::DA, 0, 18)
{
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the date
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void writingDataHandlerDate::setDate(const size_t index, const std::shared_ptr<const date>& pDate)
{
    IMEBRA_FUNCTION_START();

    setString(index, buildDate(pDate->getYear(), pDate->getMonth(), pDate->getDay()));

    IMEBRA_FUNCTION_END();
}

} // namespace handlers

} // namespace implementation

} // namespace imebra
