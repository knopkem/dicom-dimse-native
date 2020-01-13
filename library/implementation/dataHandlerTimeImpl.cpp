/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerTime.cpp
    \brief Implementation of the class dataHandlerTime.

*/

#include <sstream>
#include <iomanip>
#include <stdlib.h>

#include "exceptionImpl.h"
#include "dataHandlerTimeImpl.h"
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
// dataHandlerTime
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

readingDataHandlerTime::readingDataHandlerTime(const memory& parseMemory): readingDataHandlerDateTimeBase(parseMemory, tagVR_t::TM)
{
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the date/time
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<date> readingDataHandlerTime::getDate(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    std::uint32_t hour(0), minutes(0), seconds(0), nanoseconds(0);
    std::int32_t offsetHours(0), offsetMinutes(0);

    parseTime(getString(index), &hour, &minutes, &seconds, &nanoseconds, &offsetHours, &offsetMinutes);

    return std::make_shared<date>(0, 0, 0, hour, minutes, seconds, nanoseconds, offsetHours, offsetMinutes);

    IMEBRA_FUNCTION_END();

}

writingDataHandlerTime::writingDataHandlerTime(const std::shared_ptr<buffer> &pBuffer):
    writingDataHandlerDateTimeBase(pBuffer, tagVR_t::TM, 0, 28)
{

}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the date/time
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void writingDataHandlerTime::setDate(const size_t index, const std::shared_ptr<const date>& pDate)
{
    IMEBRA_FUNCTION_START();

    std::string timeString = buildTimeSimple(pDate->getHour(), pDate->getMinutes(), pDate->getSeconds(), pDate->getNanoseconds());
    setString(index, timeString);

    IMEBRA_FUNCTION_END();
}


} // namespace handlers

} // namespace implementation

} // namespace imebra
