/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file date.h
    \brief Implementation of the Date class.

*/

#include "../include/imebra/date.h"
#include "../implementation/dateImpl.h"

namespace imebra
{


Date::Date(const Date& source):
    m_pDate(getDateImplementation(source))
{
}

Date::Date(const std::shared_ptr<implementation::date>& pDate):
    m_pDate(pDate)
{
}

const std::shared_ptr<implementation::date>& getDateImplementation(const Date& date)
{
    return date.m_pDate;
}


Date::~Date()
{
}

Date::Date(const std::uint32_t initialYear,
           const std::uint32_t initialMonth,
           const std::uint32_t initialDay,
           const std::uint32_t initialHour,
           const std::uint32_t initialMinutes,
           const std::uint32_t initialSeconds,
           const std::uint32_t initialNanoseconds,
           const std::int32_t initialOffsetHours,
           const std::int32_t initialOffsetMinutes):
    m_pDate(std::make_shared<implementation::date>(
                initialYear,
                initialMonth,
                initialDay,
                initialHour,
                initialMinutes,
                initialSeconds,
                initialNanoseconds,
                initialOffsetHours,
                initialOffsetMinutes))
{
}

std::uint32_t Date::getYear() const
{
    return m_pDate->getYear();
}

std::uint32_t Date::getMonth() const
{
    return m_pDate->getMonth();
}

std::uint32_t Date::getDay() const
{
    return m_pDate->getDay();
}

std::uint32_t Date::getHour() const
{
    return m_pDate->getHour();
}

std::uint32_t Date::getMinutes() const
{
    return m_pDate->getMinutes();
}

std::uint32_t Date::getSeconds() const
{
    return m_pDate->getSeconds();
}

std::uint32_t Date::getNanoseconds() const
{
    return m_pDate->getNanoseconds();
}

std::int32_t Date::getOffsetHours() const
{
    return m_pDate->getOffsetHours();
}

std::int32_t Date::getOffsetMinutes() const
{
    return m_pDate->getOffsetMinutes();
}


}

