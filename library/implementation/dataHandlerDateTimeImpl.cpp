/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerDateTime.cpp
    \brief Implementation of the dataHandlerDateTime class.

*/

#include <sstream>
#include <iomanip>
#include <stdlib.h>

#include "exceptionImpl.h"
#include "dataHandlerDateTimeImpl.h"
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
// dataHandlerDateTime
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

readingDataHandlerDateTime::readingDataHandlerDateTime(const memory& parseMemory): readingDataHandlerDateTimeBase(parseMemory, tagVR_t::DT)
{

}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Retrieve the date
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<date> readingDataHandlerDateTime::getDate(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    std::string dateTimeString = getString(index);

    std::uint32_t year(0), month(0), day(0);
    parseDate(dateTimeString.substr(0, 8), &year, &month, &day);

    std::uint32_t hour(0), minutes(0), seconds(0), nanoseconds(0);
    std::int32_t offsetHours(0), offsetMinutes(0);
    if(dateTimeString.size() <= 8)
    {
        parseTime("", &hour, &minutes, &seconds, &nanoseconds, &offsetHours, &offsetMinutes);
    }
    else
    {
        parseTime(dateTimeString.substr(8), &hour, &minutes, &seconds, &nanoseconds, &offsetHours, &offsetMinutes);
    }

    return std::make_shared<date>(year, month, day, hour, minutes, seconds, nanoseconds, offsetHours, offsetMinutes);

    IMEBRA_FUNCTION_END();
}


writingDataHandlerDateTime::writingDataHandlerDateTime(const std::shared_ptr<buffer> &pBuffer):
    writingDataHandlerDateTimeBase(pBuffer, tagVR_t::DT, 0, 54)
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
void writingDataHandlerDateTime::setDate(const size_t index, const std::shared_ptr<const date>& pDate)
{
    IMEBRA_FUNCTION_START();

    std::string dateTimeString;
    dateTimeString = buildDate(pDate->getYear(), pDate->getMonth(), pDate->getDay());
    dateTimeString += buildTime(pDate->getHour(), pDate->getMinutes(), pDate->getSeconds(), pDate->getNanoseconds(), pDate->getOffsetHours(), pDate->getOffsetMinutes());
    setString(index, dateTimeString);

    IMEBRA_FUNCTION_END();
}


} // namespace handlers

} // namespace implementation

} // namespace imebra
