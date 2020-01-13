/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/// \file writingDataHandlerNumeric.h
/// \brief Declaration of the class WritingDataHandlerNumeric.
///
///////////////////////////////////////////////////////////////////////////////

#if !defined(imebraWritingDataHandlerNumeric__INCLUDED_)
#define imebraWritingDataHandlerNumeric__INCLUDED_

#include <string>
#include <memory>
#include "definitions.h"
#include "mutableMemory.h"
#include "writingDataHandler.h"


namespace imebra
{

namespace implementation
{
namespace handlers
{
    class writingDataHandlerNumericBase;
}
}

class ReadingDataHandlerNumeric;

///
/// \brief Specialized WritingDataHandler for numeric data types.
///
/// Includes few methods that allow accessing the raw memory containing the
/// buffer's data.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API WritingDataHandlerNumeric: public WritingDataHandler
{

    friend class MutableImage;
    friend class MutableDataSet;
    friend class MutableTag;
    friend class ReadingDataHandlerNumeric;

public:
    ///
    /// \brief Copy constructor.
    ///
    /// \param source source WritingDataHandlerNumeric object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    WritingDataHandlerNumeric(const WritingDataHandlerNumeric& source);

    WritingDataHandlerNumeric& operator=(const WritingDataHandlerNumeric& source) = delete;

    virtual ~WritingDataHandlerNumeric();

    /// \brief Return a MutableMemory object referencing the raw buffer's data.
    ///
    /// \return a MutableMemory object referencing the raw buffer's data
    ///
    ///////////////////////////////////////////////////////////////////////////////
    MutableMemory getMemory() const;

    /// \brief Copy the content of the specified buffer into the content managed
    ///        by data handler.
    ///
    /// <b>Java</b>
    ///
    /// In Java this method accepts a single parameter (a byte array).
    ///
    /// <b>Python</b>
    ///
    /// In Python this method accepts a single parameter (an array).
    ///
    /// \param source     a pointer to the source memory buffer
    /// \param sourceSize the number of bytes to copy and the new memory size
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void assign(const char* source, size_t sourceSize);

#ifndef SWIG
    /// \brief Returns a pointer to the buffer's raw memory content.
    ///
    /// The referenced buffer is owned by the WritingDataHandlerNumeric
    /// object and must not be freed by the client.
    ///
    /// \param pDataSize a variable that will contain the raw memory's size in
    ///                  bytes
    /// \return a pointer to the buffer's raw memory. The referenced buffer
    ///         is owned by the WritingDataHandlerNumeric object and must not be
    ///         freed by the client
    ///
    ///////////////////////////////////////////////////////////////////////////////
    char* data(size_t* pDataSize) const;
#endif

    /// \brief Copies the raw memory content into the specified buffer.
    ///
    /// If the allocated buffer is not large enough then the method doesn't
    ///  copy any data and just returns the required buffer' size.
    ///
    /// \param destination     a pointer to the allocated buffer
    /// \param destinationSize the size of the allocated buffer, in bytes
    /// \return the number of bytes to be copied into the pre-allocated buffer
    ///
    ///////////////////////////////////////////////////////////////////////////////
    size_t data(char* destination, size_t destinationSize) const;

    /// \brief Returns the number of bytes occupied by the numbers handled by the
    ///        data handler
    ///
    /// \return the number of bytes occupied by the numbers handled by the data
    ///         handler
    ///
    ///////////////////////////////////////////////////////////////////////////////
    size_t getUnitSize() const;

    /// \brief Returns true if the numbers handled by the data handler are signed,
    ///        false otherwise.
    ///
    /// \return true if the numbers handled by the data handler are signed, false
    ///         otherwise
    ///
    ///////////////////////////////////////////////////////////////////////////////
    bool isSigned() const;

    /// \brief Returns true if the numbers stored in the buffer are floating point
    ///        numbers, false otherwise.
    ///
    /// \return true if the numbers stored in the buffer are floating point
    ///         numbers, false otherwise
    ///
    ///////////////////////////////////////////////////////////////////////////////
    bool isFloat() const;

    /// \brief Copies data from another data handler, converting the data type
    ///        if necessary.
    ///
    /// The data handler is resized to the same size of the source data handler.
    ///
    /// \param source the data handler from which the data must be copied
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void copyFrom(const ReadingDataHandlerNumeric& source);

protected:
    explicit WritingDataHandlerNumeric(const std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase>& pDataHandler);
};

}

#endif // !defined(imebraWritingDataHandlerNumeric__INCLUDED_)
