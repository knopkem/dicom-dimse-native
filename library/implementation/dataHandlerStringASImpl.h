/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerStringAS.h
    \brief Declaration of the class dataHandlerStringAS.

*/

#if !defined(imebraDataHandlerStringAS_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
#define imebraDataHandlerStringAS_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_

#include "dataHandlerStringImpl.h"
#include "../include/imebra/definitions.h"


namespace imebra
{

namespace implementation
{

namespace handlers
{


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief Handles the Dicom data type "AS" (age string)
///
/// The handler supplies two additional functions designed
///  to set/get the age (setAge() and getAge()) and
///  overwrite the functions getSignedLong(),
///  getUnsignedLong(), getDouble(), setSignedLong(),
///  setUnsignedLong(), setDouble() to make them work with
///  the years.
///
/// setDouble() and getDouble() work also with fraction
///  of a year, setting the age unit correctly (days,
///  weeks, months or years).
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class readingDataHandlerStringAS : public readingDataHandlerString
{
public:
    readingDataHandlerStringAS(const memory& parseMemory);

    /// \brief Retrieve the age value and its unit from the
    ///         buffer handled by this handler.
    ///
    /// @param index the zero based index of the age value to
    ///               modify
    /// @return the age
    ///
    ///////////////////////////////////////////////////////////
    virtual std::shared_ptr<age> getAge(const size_t index) const override;

    /// \brief Retrieve the age from the handled buffer.
    ///
    /// The returned value is always expressed in years.
    ///
    /// @param index the zero based index of the age value to
    ///               read
    /// @return the age contained in the buffer converted into
    ///          years
    ///
    ///////////////////////////////////////////////////////////
    virtual std::int32_t getSignedLong(const size_t index) const override;

    /// \brief Retrieve the age from the handled buffer.
    ///
    /// The returned value is always expressed in years.
    ///
    /// @param index the zero based index of the age value to
    ///               read
    /// @return the age contained in the buffer converted into
    ///          years
    ///
    ///////////////////////////////////////////////////////////
    virtual std::uint32_t getUnsignedLong(const size_t index) const override;

    /// \brief Retrieve the age from the handled buffer.
    ///
    /// The returned value is always expressed in years.
    /// The function can return fraction of a year if the
    ///  age contained in the buffer is expressed in days,
    ///  weeks or months.
    ///
    /// @param index the zero based index of the age value to
    ///               read
    /// @return the age contained in the buffer converted into
    ///          years
    ///
    ///////////////////////////////////////////////////////////
    virtual double getDouble(const size_t index) const override;
};


class writingDataHandlerStringAS: public writingDataHandlerString
{
public:
    writingDataHandlerStringAS(const std::shared_ptr<buffer>& pBuffer);

    /// \brief Set the value of the age string and specify
    ///         its unit.
    ///
    /// @param index the zero based index of the age value to
    ///               read
    /// @param pAge  the age to be written in the buffer
    /// @param unit  the units used for the parameter age
    ///
    ///////////////////////////////////////////////////////////
    virtual void setAge(const size_t index, const std::shared_ptr<const age>& pAge) override;

    /// \brief Write the specified age into the handled buffer.
    ///
    /// @param index the zero based index of the age value to
    ///               modify
    /// @param value the age to be written, in years
    ///
    ///////////////////////////////////////////////////////////
    virtual void setSignedLong(const size_t index, const std::int32_t value) override;

    /// \brief Write the specified age into the handled buffer.
    ///
    /// @param index the zero based index of the age value to
    ///               modify
    /// @param value the age to be written, in years
    ///
    ///////////////////////////////////////////////////////////
    virtual void setUnsignedLong(const size_t index, const std::uint32_t value) override;

    /// \brief Write the specified age into the handled buffer.
    ///
    /// If a fraction of a year is specified, then the function
    ///  set the age in days, weeks or months according to
    ///  the value of the fraction.
    ///
    /// @param index the zero based index of the age value to
    ///               modify
    /// @param value the age to be written, in years
    ///
    ///////////////////////////////////////////////////////////
    virtual void setDouble(const size_t index, const double value) override;

    virtual void validate() const override;

};

} // namespace handlers

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDataHandlerStringAS_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
