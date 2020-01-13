/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/// \file readingDataHandlerNumeric.h
/// \brief Declaration of the class ReadingDataHandler and
///        WritingDataHandler.
///
///////////////////////////////////////////////////////////////////////////////

#if !defined(imebraReadingDataHandlerNumeric__INCLUDED_)
#define imebraReadingDataHandlerNumeric__INCLUDED_

#include <string>
#include <memory>
#include "definitions.h"
#include "memory.h"
#include "readingDataHandler.h"

namespace imebra
{

namespace implementation
{
namespace handlers
{
    class readingDataHandlerNumericBase;
}
}

class WritingDataHandlerNumeric;

///
/// \brief Specialized ReadingDataHandler for numeric data types.
///
/// Includes few methods that allow accessing the raw memory containing the
/// buffer's data.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API ReadingDataHandlerNumeric: public ReadingDataHandler
{

public:
    friend class Image;
    friend class DataSet;
    friend class Tag;
    friend class LUT;

public:
    ///
    /// \brief Copy constructor.
    ///
    /// \param source source ReadingDataHandlerNumeric object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    ReadingDataHandlerNumeric(const ReadingDataHandlerNumeric& source);

    ReadingDataHandlerNumeric& operator=(const ReadingDataHandlerNumeric& source) = delete;

    virtual ~ReadingDataHandlerNumeric();

    /// \brief Return a Memory object referencing the raw buffer's data.
    ///
    /// \return a Memory object referencing the raw buffer's data
    ///
    ///////////////////////////////////////////////////////////////////////////////
    const Memory getMemory() const;

    /// \brief Copies the buffer's raw memory content into the specified buffer.
    ///
    /// If the allocated buffer is not large enough then the method doesn't
    ///  copy any data and just returns the required buffer' size.
    ///
    /// <b>Java</b>
    ///
    /// In Java this method accepts a single parameter (a byte array).
    /// The size of the byte array must be equal or greater than the number of
    /// bytes stored by the data handler.
    ///
    /// <b>Python</b>
    ///
    /// In Python this method accepts a single parameter (an array).
    /// The size of the array (in bytes) must be equal or greater than the number
    /// of bytes stored by the data handler.
    ///
    /// \param destination     a pointer to the allocated buffer
    /// \param destinationSize the size of the allocated buffer
    /// \return the number of bytes copied into the pre-allocated buffer, or the
    ///         desired size of destination if destinationSize is smaller than
    ///         the return value
    ///
    ///////////////////////////////////////////////////////////////////////////////
    size_t data(char* destination, size_t destinationSize) const;

#ifndef SWIG
    /// \brief Returns a pointer to the buffer's raw memory content.
    ///
    /// The referenced buffer is owned by the ReadingDataHandlerNumeric object and
    /// must not be freed by the client.
    ///
    /// \param pDataSize a variable that will contain the raw memory's size in
    ///                  bytes
    /// \return a pointer to the buffer's raw memory.
    ///        The referenced buffer is owned by the ReadingDataHandlerNumeric
    ///        object and must not be freed by the client.
    ///
    ///
    ///////////////////////////////////////////////////////////////////////////////
    const char* data(size_t* pDataSize) const;
#endif

    /// \brief Returns the number of bytes occupied by the numbers handled by the
    ///        data handler
    ///
    /// \return the number of bytes occupied by the numbers handled by the data
    ///         handler
    ///
    ///////////////////////////////////////////////////////////////////////////////
    size_t getUnitSize() const;

    /// \brief Returns true if the numbers stored in the buffer are signed, false
    ///        otherwise.
    ///
    /// \return true if the numbers stored in the buffer are signed, false
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

    /// \brief Copies the content of the data handler into another data handler,
    ///        converting the data to the destination handler data type.
    ///
    /// \warning the size of the destination data handler stays unchanged: if
    ///          the destination too small to contain all the data to be copied
    ///          then only a part of the data will be copied.
    ///
    /// \param destination the destination data handler
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void copyTo(const WritingDataHandlerNumeric& destination);

protected:
    explicit ReadingDataHandlerNumeric(const std::shared_ptr<implementation::handlers::readingDataHandlerNumericBase>& pDataHandler);
};

}

#endif // !defined(imebraReadingDataHandlerNumeric__INCLUDED_)
