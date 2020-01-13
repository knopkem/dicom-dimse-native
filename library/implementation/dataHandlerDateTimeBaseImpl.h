/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerDateTimeBase.h
    \brief Declaration of the base class for the time/date handlers.

*/

#if !defined(imebraDataHandlerDateTimeBase_85665C7B_8DDF_479e_8CC0_83E95CB625DC__INCLUDED_)
#define imebraDataHandlerDateTimeBase_85665C7B_8DDF_479e_8CC0_83E95CB625DC__INCLUDED_

#include "dataHandlerStringImpl.h"


namespace imebra
{

namespace implementation
{

namespace handlers
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class is used as base class by the handlers
///         that manage the date and the time
///
/// This class supplies the methods setSignedLong(), 
///  setUnsignedLong(), setDouble(), getSignedLong(),
///  getUnsignedLong(), getDouble(). Those methods work
///  with time_t structure
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class readingDataHandlerDateTimeBase : public readingDataHandlerString
{

public:
    readingDataHandlerDateTimeBase(const memory& parseMemory, tagVR_t dataType);

    virtual std::int32_t getSignedLong(const size_t index) const override;
    virtual std::uint32_t getUnsignedLong(const size_t index) const override;
    virtual double getDouble(const size_t index) const override;

protected:
	void parseDate(
        const std::string& dateString,
        std::uint32_t* pYear,
        std::uint32_t* pMonth,
        std::uint32_t* pDay) const;

	void parseTime(
        const std::string& timeString,
        std::uint32_t* pHour,
        std::uint32_t* pMinutes,
        std::uint32_t* pSeconds,
        std::uint32_t* pNanoseconds,
		std::int32_t* pOffsetHours,
		std::int32_t* pOffsetMinutes) const;
};

class writingDataHandlerDateTimeBase: public writingDataHandlerString
{
public:
    writingDataHandlerDateTimeBase(const std::shared_ptr<buffer>& pBuffer, tagVR_t dataType, const size_t unitSize, const size_t maxSize);

    virtual void setSignedLong(const size_t index, const std::int32_t value) override;
    virtual void setUnsignedLong(const size_t index, const std::uint32_t value) override;
    virtual void setDouble(const size_t index, const double value) override;

protected:
    std::string buildDate(
        std::uint32_t year,
        std::uint32_t month,
        std::uint32_t day) const;

    std::string buildTime(
        std::uint32_t hour,
        std::uint32_t minutes,
        std::uint32_t seconds,
        std::uint32_t nanoseconds,
        std::int32_t offsetHours,
        std::int32_t offsetMinutes
        ) const;

    std::string buildTimeSimple(
        std::uint32_t hour,
        std::uint32_t minutes,
        std::uint32_t seconds,
        std::uint32_t nanoseconds
        ) const;

    std::string padLeft(const std::string& source, const char fillChar, const size_t length) const;

};

} // namespace handlers

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDataHandlerDateTimeBase_85665C7B_8DDF_479e_8CC0_83E95CB625DC__INCLUDED_)
