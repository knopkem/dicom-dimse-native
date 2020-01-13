/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerDateTimeBase.cpp
    \brief Implementation of the base class for the date/time handlers.

*/

#include "exceptionImpl.h"
#include "dataHandlerDateTimeBaseImpl.h"
#include "dicomDictImpl.h"
#include "../include/imebra/exceptions.h"
#include <time.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>

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
// dataHandlerDateTimeBase
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

readingDataHandlerDateTimeBase::readingDataHandlerDateTimeBase(const memory& parseMemory, tagVR_t dataType):
    readingDataHandlerString(parseMemory, dataType, '-', 0x20)
{

}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns a long integer representing the date/time (UTC)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::int32_t readingDataHandlerDateTimeBase::getSignedLong(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    return (std::int32_t)getDouble(index);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Cast getSignedLong to unsigned long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t readingDataHandlerDateTimeBase::getUnsignedLong(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    return (std::uint32_t)getDouble(index);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get double
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
double readingDataHandlerDateTimeBase::getDouble(const size_t /* index */) const
{
    IMEBRA_FUNCTION_START();

    IMEBRA_THROW(DataHandlerConversionError, "Cannot convert VR "<< dicomDictionary::getDicomDictionary()->enumDataTypeToString(getDataType()) << " to double");

    IMEBRA_FUNCTION_END();
}


writingDataHandlerDateTimeBase::writingDataHandlerDateTimeBase(const std::shared_ptr<buffer> &pBuffer, tagVR_t dataType, const size_t unitSize, const size_t maxSize):
    writingDataHandlerString(pBuffer, dataType, '-', unitSize, maxSize)
{
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the date as a signed long (from time_t)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void writingDataHandlerDateTimeBase::setSignedLong(const size_t index, const std::int32_t value)
{
    IMEBRA_FUNCTION_START();

    setDouble(index, (double)value);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the date as a long (from time_t)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void writingDataHandlerDateTimeBase::setUnsignedLong(const size_t index, const std::uint32_t value)
{
    IMEBRA_FUNCTION_START();

    setDouble(index, (double)value);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the date as a double (from time_t)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void writingDataHandlerDateTimeBase::setDouble(const size_t /* index */, const double /* value */)
{
    IMEBRA_FUNCTION_START();

    IMEBRA_THROW(DataHandlerConversionError, "Cannot convert from double to VR "<< dicomDictionary::getDicomDictionary()->enumDataTypeToString(getDataType()));

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Parse a date string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void readingDataHandlerDateTimeBase::parseDate(
        const std::string& dateString,
        std::uint32_t* pYear,
        std::uint32_t* pMonth,
        std::uint32_t* pDay) const
{
    IMEBRA_FUNCTION_START();

    if(dateString.size() != 8)
    {
        IMEBRA_THROW(DataHandlerCorruptedBufferError, "The date/time string has the wrong size");
    }

    std::string dateYear=dateString.substr(0, 4);
    std::string dateMonth=dateString.substr(4, 2);
    std::string dateDay=dateString.substr(6, 2);

    std::istringstream yearStream(dateYear);
    yearStream >> (*pYear);

    std::istringstream monthStream(dateMonth);
    monthStream >> (*pMonth);

    std::istringstream dayStream(dateDay);
    dayStream >> (*pDay);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Build a date string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string writingDataHandlerDateTimeBase::buildDate(
        std::uint32_t year,
        std::uint32_t month,
        std::uint32_t day) const
{
    IMEBRA_FUNCTION_START();

        if((year > 9999) || (month < 1) || (month>12) || (day<1) || (day>31))
    {
        year = month = day = 0;
    }

    std::ostringstream dateStream;
    dateStream << std::setfill('0');
    dateStream << std::setw(4) << year;
    dateStream << std::setw(2) << month;
    dateStream << std::setw(2) << day;

    return dateStream.str();

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Parse a time string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void readingDataHandlerDateTimeBase::parseTime(
        const std::string& timeString,
        std::uint32_t* pHour,
        std::uint32_t* pMinutes,
        std::uint32_t* pSeconds,
        std::uint32_t* pNanoseconds,
        std::int32_t* pOffsetHours,
        std::int32_t* pOffsetMinutes) const
{
    IMEBRA_FUNCTION_START();

    std::string fullTimeString(timeString);

    if(fullTimeString.size() < 6)
    {
        fullTimeString.resize(6, '0');
    }
    if(fullTimeString.size() < 7)
    {
        fullTimeString += '.';
    }
    if(fullTimeString.size() < 13)
    {
        fullTimeString.resize(13, '0');
    }
    if(fullTimeString.size() < 14)
    {
        fullTimeString += '+';
    }
    if(fullTimeString.size() < 18)
    {
        fullTimeString.resize(18, '0');
    }

    std::string timeHour = fullTimeString.substr(0, 2);
    std::string timeMinutes = fullTimeString.substr(2, 2);
    std::string timeSeconds = fullTimeString.substr(4, 2);
    std::string timeNanoseconds = fullTimeString.substr(7, 6);
    std::string timeOffsetHours = fullTimeString.substr(13, 3);
    std::string timeOffsetMinutes = fullTimeString.substr(16, 2);

    std::istringstream hourStream(timeHour);
    hourStream >> (*pHour);

    std::istringstream minutesStream(timeMinutes);
    minutesStream >> (*pMinutes);

    std::istringstream secondsStream(timeSeconds);
    secondsStream >> (*pSeconds);

    std::istringstream nanosecondsStream(timeNanoseconds);
    nanosecondsStream >> (*pNanoseconds);

    std::istringstream offsetHoursStream(timeOffsetHours);
    offsetHoursStream >> (*pOffsetHours);

    std::istringstream offsetMinutesStream(timeOffsetMinutes);
    offsetMinutesStream >> (*pOffsetMinutes);

    if(*pOffsetHours < 0)
    {
        *pOffsetMinutes= - *pOffsetMinutes;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Build the time string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string writingDataHandlerDateTimeBase::buildTime(
        std::uint32_t hour,
        std::uint32_t minutes,
        std::uint32_t seconds,
        std::uint32_t nanoseconds,
        std::int32_t offsetHours,
        std::int32_t offsetMinutes
        ) const
{
    IMEBRA_FUNCTION_START();

    if(
        (hour >= 24)
        || (minutes >= 60)
        || (seconds >= 60)
        || (nanoseconds > 999999)
        || (offsetHours < -12)
        || (offsetHours > 12)
        || (offsetMinutes < -59)
        || (offsetMinutes > 59))
    {
        IMEBRA_THROW(DataHandlerConversionError, "Invalid Time values. Hour:" << hour << " minutes:" << minutes << " seconds:" << seconds << " nanoseconds:" << nanoseconds <<
                                         " offsetHours:" << offsetHours << " offsetMinutes:" << offsetMinutes);
    }

    bool bMinus=offsetHours < 0;

    std::ostringstream timeStream;
    timeStream << std::setfill('0');
    timeStream << std::setw(2) << hour;
    timeStream << std::setw(2) << minutes;
    timeStream << std::setw(2) << seconds;
    timeStream << std::setw(1) << ".";
    timeStream << std::setw(6) << nanoseconds;
    timeStream << std::setw(1) << (bMinus ? "-" : "+");
    timeStream << std::setw(2) << labs(offsetHours);
    timeStream << std::setw(2) << labs(offsetMinutes);

    return timeStream.str();

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Build the time string (without timezone)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string writingDataHandlerDateTimeBase::buildTimeSimple(
        std::uint32_t hour,
        std::uint32_t minutes,
        std::uint32_t seconds,
        std::uint32_t nanoseconds
        ) const
{
    IMEBRA_FUNCTION_START();

    if(
        (hour >= 24)
        || (minutes >= 60)
        || (seconds >= 60)
        || (nanoseconds > 999999))
    {
        IMEBRA_THROW(DataHandlerConversionError, "Invalid Time values hour:" << hour << " minutes:" << minutes << " seconds:" << seconds << " nanoseconds:" << nanoseconds);
    }

    std::ostringstream timeStream;
    timeStream << std::setfill('0');
    timeStream << std::setw(2) << hour;
    timeStream << std::setw(2) << minutes;
    timeStream << std::setw(2) << seconds;
    if(nanoseconds != 0)
    {
        timeStream << std::setw(1) << ".";
        timeStream << std::setw(6) << nanoseconds;
    }

    return timeStream.str();

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Add the specified char to the left of a string until
//  its length reaches the desidered value
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string writingDataHandlerDateTimeBase::padLeft(const std::string& source, const char fillChar, const size_t length) const
{
    IMEBRA_FUNCTION_START();

        if(source.size() >= length)
        {
            return source;
        }

        std::string paddedString(length - source.size(), fillChar);
        paddedString += source;

    return paddedString;

    IMEBRA_FUNCTION_END();
}

} // namespace handlers

} // namespace implementation

} // namespace imebra



