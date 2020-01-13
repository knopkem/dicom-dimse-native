/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file MutableMemory.h
    \brief Declaration of the class MutableMemory.

*/

#if !defined(imebraMutableMemory__INCLUDED_)
#define imebraMutableMemory__INCLUDED_

#include <memory>
#include <string>
#include "definitions.h"
#include "memory.h"

namespace imebra
{

namespace implementation
{
class memory;
}

///
/// \brief Manages a writable buffer of memory.
///
/// The buffer of memory is usually associated with a Tag buffer content.
///
/// The memory used by Memory and MutableMemory is managed by MemoryPool.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API MutableMemory: public Memory
{

    friend class MemoryStreamOutput;
    friend class DrawBitmap;
    friend class WritingDataHandlerNumeric;

public:

    /// \brief Construct an empty buffer of memory.
    ///
    /// The memory can be resized later with resize().
    ///
    ///////////////////////////////////////////////////////////////////////////////
    MutableMemory();

    /// \brief Construct a buffer of memory of the specified size.
    ///
    /// The memory can be resized later with resize().
    ///
    /// \param initialSize the initial memory's size, in bytes
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit MutableMemory(size_t initialSize);

    /// \brief Construct a buffer of memory and copy the specified content into it.
    ///
    /// \param sourceMemory the object containing the initial memory's content
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit MutableMemory(const Memory& sourceMemory);

    ///
    /// \brief Copy constructor.
    ///
    /// \param source source MutableMemory object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    MutableMemory(const MutableMemory& source);

    MutableMemory& operator=(const MutableMemory& source) = delete;

    /// \brief Construct a buffer of memory and copy the specified content into it.
    ///
    /// <b>Java</b>
    ///
    /// In Java this method accepts a single parameter (a byte array).
    ///
    /// <b>Python</b>
    ///
    /// In Python this method accepts a single parameter (an array).
    ///
    /// \param source      a pointer to the source data
    /// \param sourceSize  the amount of data to copy into the allocated memory
    ///
    ///////////////////////////////////////////////////////////////////////////////
    explicit MutableMemory(const char* source, size_t sourceSize);

    virtual ~MutableMemory();

    /// \brief Copy the content from another memory object.
    ///
    /// \param sourceMemory the source memory object
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void copyFrom(const Memory& sourceMemory);

    /// \brief Resize the memory to zero bytes.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void clear();

    /// \brief Resize the memory.
    ///
    /// \param newSize the new memory size, in bytes
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void resize(size_t newSize);

    /// \brief Reserve the specified amount of bytes, without changing the memory
    ///        size
    ///
    /// \param reserveSize the number of bytes to reserve
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void reserve(size_t reserveSize);

#ifndef SWIG
    /// \brief Return a pointer to the referenced memory.
    ///
    /// The referenced buffer is owned by the MutableMemory object and must
    /// not be freed by the client.
    ///
    /// \param pDataSize pointer to a variable that will be filled with the
    ///        memory size, in bytes
    /// \return a pointer to the referenced memory.
    ///        The referenced buffer is owned by the MutableMemory object and
    ///        must not be freed by the client.
    ///
    ///
    ///////////////////////////////////////////////////////////////////////////////
    char* data(size_t* pDataSize) const;
#endif

    /// \brief Copy the content of the specified buffer into the MutableMemory
    ///        object.
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

    /// \brief Copy the content of the specified buffer into a region of the
    ///        MutableMemory.
    ///
    /// The memory size remains unchanged.
    ///
    /// <b>Java</b>
    ///
    /// In Java this method accepts two parameters:
    /// - a byte array
    /// - a long integer (destinationOffset)
    ///
    /// <b>Python</b>
    ///
    /// In Python this method accepts two parameters:
    /// - an array
    /// - an integer (destinationOffset)
    ///
    /// \param source     a pointer to the source memory buffer
    /// \param sourceSize the number of bytes to copy
    /// \param destinationOffset the offset into the MutableMemory at which the
    ///                   data must be copied
    ///
    ///////////////////////////////////////////////////////////////////////////////
    void assignRegion(const char* source, size_t sourceSize, size_t destinationOffset);

protected:
    explicit MutableMemory(std::shared_ptr<implementation::memory> pMemory);
};

}

#endif // !defined(imebraMutableMemory__INCLUDED_)
