/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dateImpl.cpp
    \brief Implementation of the class date.

*/

#include "dateImpl.h"

namespace imebra
{

namespace implementation
{

date::date(const std::uint32_t initialYear,
           const std::uint32_t initialMonth,
           const std::uint32_t initialDay,
           const std::uint32_t initialHour,
           const std::uint32_t initialMinutes,
           const std::uint32_t initialSeconds,
           const std::uint32_t initialNanoseconds,
           const std::int32_t initialOffsetHours,
           const std::int32_t initialOffsetMinutes):
    m_year(initialYear), m_month(initialMonth), m_day(initialDay),
    m_hour(initialHour), m_minutes(initialMinutes), m_seconds(initialSeconds), m_nanoseconds(initialNanoseconds),
    m_offsetHours(initialOffsetHours), m_offsetMinutes(initialOffsetMinutes)
{
}

std::uint32_t date::getYear() const
{
    return m_year;
}

std::uint32_t date::getMonth() const
{
    return m_month;
}

std::uint32_t date::getDay() const
{
    return m_day;
}

std::uint32_t date::getHour() const
{
    return m_hour;
}

std::uint32_t date::getMinutes() const
{
    return m_minutes;
}

std::uint32_t date::getSeconds() const
{
    return m_seconds;
}

std::uint32_t date::getNanoseconds() const
{
    return m_nanoseconds;
}

std::int32_t date::getOffsetHours() const
{
    return m_offsetHours;
}

std::int32_t date::getOffsetMinutes() const
{
    return m_offsetMinutes;
}


} // namespace implementation

} // namespace imebra

