/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dateImpl.h
    \brief Declaration of the class date.

*/

#if !defined(imebraDate_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
#define imebraDate_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_

#include <cstdint>

namespace imebra
{

namespace implementation
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// date
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class date
{
public:

    /// \brief Constructor.
    ///
    /// Initialize the Date structure with the specified values.
    ///
    /// \param initialYear    year (0 = unused)
    /// \param initialMonth   month (1...12, 0 = unused)
    /// \param initialDay     day of the month (1...31, 0 = unused)
    /// \param initialHour    hour (0...23)
    /// \param initialMinutes minutes (0...59)
    /// \param initialSeconds seconds (0...59)
    /// \param initialNanoseconds nanoseconds
    /// \param initialOffsetHours   hours offset from UTC
    /// \param initialOffsetMinutes minutes offset from UTC
    ///
    ///////////////////////////////////////////////////////////////////////////////
    date(const std::uint32_t initialYear,
         const std::uint32_t initialMonth,
         const std::uint32_t initialDay,
         const std::uint32_t initialHour,
         const std::uint32_t initialMinutes,
         const std::uint32_t initialSeconds,
         const std::uint32_t initialNanoseconds,
         const std::int32_t initialOffsetHours,
         const std::int32_t initialOffsetMinutes);

    std::uint32_t getYear() const;
    std::uint32_t getMonth() const;
    std::uint32_t getDay() const;
    std::uint32_t getHour() const;
    std::uint32_t getMinutes() const;
    std::uint32_t getSeconds() const;
    std::uint32_t getNanoseconds() const;
    std::int32_t getOffsetHours() const;
    std::int32_t getOffsetMinutes() const;

private:
    std::uint32_t m_year;         ///< Year (0 = unused)
    std::uint32_t m_month;        ///< Month (1...12, 0 = unused)
    std::uint32_t m_day;          ///< Day (1...12, 0 = unused)
    std::uint32_t m_hour;         ///< Hours
    std::uint32_t m_minutes;      ///< Minutes
    std::uint32_t m_seconds;      ///< Seconds
    std::uint32_t m_nanoseconds;  ///< Nanoseconds
    std::int32_t m_offsetHours;   ///< Offset hours from UTC
    std::int32_t m_offsetMinutes; ///< Offset minutes from UTC


};

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDate_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
