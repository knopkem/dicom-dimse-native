/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/// \file writingDataHandler.h
/// \brief Declaration of the class WritingDataHandler.
///
///////////////////////////////////////////////////////////////////////////////

#if !defined(imebraWritingDataHandler__INCLUDED_)
#define imebraWritingDataHandler__INCLUDED_

#include <string>
#include <memory>
#include "definitions.h"

namespace imebra
{

class Date;
class Age;
class PatientName;
class UnicodePatientName;

namespace implementation
{

namespace handlers
{
    class writingDataHandler;
}

}

///
/// \brief The WritingDataHandler class allows to write the content
///        of a Dicom tag's buffer.
///
/// WritingDataHandler is able to write into the buffer's content strings,
/// numbers, date/time or ages.
///
/// In order to obtain a WritingDataHandler object for a specific tag stored
/// in a DataSet, call DataSet::getWritingDataHandler() or
/// Tag::getWritingDataHandler().
///
/// The WritingDataHandler object always works on a new and clean memory area.
/// The buffer's memory is replaced by the WritingDataHandler memory when the
/// data handler is destroyed.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API WritingDataHandler
{

    friend class MutableDataSet;
    friend class MutableTag;

public:
    ///
    /// \brief Copy constructor.
    ///
    /// \param source source WritingDataHandler object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    WritingDataHandler(const WritingDataHandler& source);

    WritingDataHandler& operator=(const WritingDataHandler& source) = delete;

    /// \brief Destructor: replaces the tag buffer's memory with the memory created
    ///        by this WritingDataHandler.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    virtual ~WritingDataHandler();

    /// \brief Resize the memory to contain the specified number of elements.
    ///
    /// By default, the WritingDataHandler buffer allocates an empty memory block
    /// that must be resized in order to be filled with data.
    ///
    /// The type of the contained elements depends on the tag's VR. The VR can be
    /// retrieved with getDataType().
    ///
    /// \param elementsNumber the number of elements to store in the buffer.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void setSize(size_t elementsNumber);

    /// \brief Retrieve the number of elements that can be stored in the buffer
    ///        controlled by WritingDataHandler.
    ///
    /// The memory size can be changed with setSize().
    ///
    /// The type of the contained elements depends on the tag's VR. The VR can be
    /// retrieved with getDataType().
    ///
    /// \return the number of elements that can be stored in the buffer
    ///
    ///////////////////////////////////////////////////////////////////////////////
    size_t getSize() const;

    /// \brief Returns the data type (VR) of the data handled by the data handler.
    ///
    /// \return the data type of the handled data
    ///
    ///////////////////////////////////////////////////////////////////////////////
    tagVR_t getDataType() const;

    /// \brief Write a signed long integer (32 bit).
    ///
    /// If the value cannot be converted from a signed long integer
    /// then throws DataHandlerConversionError.
    ///
    /// \param index the element number within the buffer. Must be smaller than
    ///        getSize()
    /// \param value the value to write
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void setSignedLong(size_t index, std::int32_t value);

    /// \brief Write an unsigned long integer (32 bit).
    ///
    /// If the value cannot be converted from an unsigned long integer
    /// then throws DataHandlerConversionError.
    ///
    /// \param index the element number within the buffer. Must be smaller than
    ///        getSize()
    /// \param value the value to write
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void setUnsignedLong(size_t index, std::uint32_t value);

    /// \brief Write a double floating point value (64 bit).
    ///
    /// If the value cannot be converted from a double floating point
    /// then throws DataHandlerConversionError.
    ///
    /// \param index the element number within the buffer. Must be smaller than
    ///        getSize()
    /// \param value the value to write
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void setDouble(size_t index, double value);

    /// \brief Write a UTF8 string.
    ///
    /// If the value cannot be converted from a string
    /// then throws DataHandlerConversionError.
    ///
    /// \param index the element number within the buffer. Must be smaller than
    ///        getSize()
    /// \param value the value to write
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void setString(size_t index, const std::string& value);

    /// \brief Write an Unicode string.
    ///
    /// If the value cannot be converted from a Unicode string
    /// then throws DataHandlerConversionError.
    ///
    /// \param index the element number within the buffer. Must be smaller than
    ///        getSize()
    /// \param value the value to write
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void setUnicodeString(size_t index, const std::wstring& value);

    /// \brief Write a date and/or a time.
    ///
    /// If the value cannot be converted from a Date
    /// then throws DataHandlerConversionError.
    ///
    /// \param index the element number within the buffer. Must be smaller than
    ///        getSize()
    /// \param date  the value to write
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void setDate(size_t index, const Date& date);

    /// \brief Write an Age value.
    ///
    /// If the value cannot be converted from an Age
    /// then throws DataHandlerConversionError.
    ///
    /// \param index the element number within the buffer. Must be smaller than
    ///        getSize()
    /// \param age   the value to write
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void setAge(size_t index, const Age& age);

    /// \brief Write a PatientName value.
    ///
    /// If the value cannot be converted from a PatientName value
    /// then throws DataHandlerConversionError.
    ///
    /// \param index the element number within the buffer. Must be smaller than
    ///        getSize()
    /// \param age   the value to write
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void setPatientName(size_t index, const PatientName& patientName);

#ifndef SWIG // Use UTF8 strings only with SWIG

    /// \brief Write a UnicodePatientName value.
    ///
    /// If the value cannot be converted from a PatientName value
    /// then throws DataHandlerConversionError.
    ///
    /// \param index the element number within the buffer. Must be smaller than
    ///        getSize()
    /// \param age   the value to write
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void setUnicodePatientName(size_t index, const UnicodePatientName& patientName);

protected:
    explicit WritingDataHandler(const std::shared_ptr<implementation::handlers::writingDataHandler>& pDataHandler);

private:
    friend const std::shared_ptr<implementation::handlers::writingDataHandler>& getWritingDataHandlerImplementation(const WritingDataHandler& writingHandler);
    std::shared_ptr<implementation::handlers::writingDataHandler> m_pDataHandler;
#endif
};

}

#endif // !defined(imebraWritingDataHandler__INCLUDED_)
