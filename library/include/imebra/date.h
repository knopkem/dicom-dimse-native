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
    \brief Declaration of the Date class.

*/

#if !defined(imebraDate__INCLUDED_)
#define imebraDate__INCLUDED_

#include "definitions.h"
#include <memory>

namespace imebra
{

namespace implementation
{
class date;
}

///
/// \brief Stores a Date, Time or Date/Time value.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API Date
{
    friend class DataSet;
    friend class ReadingDataHandler;

public:
    ///
    /// \brief Copy constructor.
    ///
    /// \param source source Date object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    Date(const Date& source);

    Date& operator=(const Date& source) = delete;

    virtual ~Date();

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
    ///////////////////////////////////////////////////////////////////////////////
    Date(const std::uint32_t initialYear,
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

#ifndef SWIG
protected:
    explicit Date(const std::shared_ptr<implementation::date>& pDate);

private:
    friend const std::shared_ptr<implementation::date>& getDateImplementation(const Date& date);
    std::shared_ptr<implementation::date> m_pDate;
#endif

};

}

#endif // imebraDate__INCLUDED_
