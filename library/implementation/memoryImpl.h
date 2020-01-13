/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file memory.h
    \brief Declaration of the memory manager and the memory class.

*/

#if !defined(imebraMemory_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_)
#define imebraMemory_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_

#include <list>
#include <map>
#include <memory>
#include <array>

#ifdef __APPLE__
#include <pthread.h>
#else
#include <thread>
#endif


#if(!defined IMEBRA_MEMORY_POOL_SLOTS)
    #define IMEBRA_MEMORY_POOL_SLOTS 256
#endif
#if(!defined IMEBRA_MEMORY_POOL_MAX_SIZE)
    #define IMEBRA_MEMORY_POOL_MAX_SIZE 20000000
#endif
#if(!defined IMEBRA_MEMORY_POOL_MIN_SIZE)
    #define IMEBRA_MEMORY_POOL_MIN_SIZE 1024
#endif


namespace imebra
{

namespace implementation
{

typedef std::basic_string<std::uint8_t> stringUint8;

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class holds an allocated array of bytes.
///
/// New instances of this class should be obtained through
///  the class \ref memoryPool; 
///  call \ref memoryPool::getMemory() in order to
///  obtain a new instance of memory.
///
/// This class is usually used by \ref implementation::buffer 
///  objects to store the tags values.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class memory
{
public:
    /// \brief Construct an empty memory object
    ///
    ///////////////////////////////////////////////////////////
	memory();

    /// \brief Construct a memory object using the passed
    ///        buffer.
    ///
    /// Takes ownership of the buffer and deletes it in
    ///  the destructor.
    ///
    /// @param pBuffer buffer containing the data. The
    ///                memory object will take ownership
    ///                of it.
    ///
    ///////////////////////////////////////////////////////////
    memory(stringUint8* pBuffer);

    /// \brief Constructs the memory object and allocate
	///         the requested amount of memory
	///
	/// @param initialSize the initial size of the allocated
	///                      memory, in bytes
	///
	///////////////////////////////////////////////////////////
    memory(size_t initialSize);

    /// \brief Destruct the memory object.
    ///
    /// The owned buffer is passed to the memoryPool for
    ///  possible reuse: memoryPool will decide if to keep
    ///  the buffer or to delete it.
    ///
    ///////////////////////////////////////////////////////////
    ~memory();

	/// \brief Copy the content of the memory managed
	///         by another memory object into the memory 
	///         managed by this object.
	///
	/// @param sourceMemory a pointer to the memory object from
	///                      which the data has to be copied
	///
	///////////////////////////////////////////////////////////
    void copyFrom(const std::shared_ptr<const memory>& sourceMemory);

	/// \brief Clear the content of the memory object and
	///         set its size to 0 bytes.
	///
	///////////////////////////////////////////////////////////
	void clear();

	/// \brief Resize the memory buffer.
	///
	/// @param newSize  the new size of the buffer, in bytes
	///
	///////////////////////////////////////////////////////////
    void resize(size_t newSize);

	/// \brief Reserve the specified quantity of bytes for
	///         the memory object. This doesn't modify the
	///         actual size of the memory object.
	///
	/// @param reserveSize   the number of bytes to reserve for
	///                       the memory object.
	///
	///////////////////////////////////////////////////////////
    void reserve(size_t reserveSize);

	/// \brief Return the size of the managed
	///         memory in bytes.
	///
	/// @return the size of the managed memory, in bytes
	///
	///////////////////////////////////////////////////////////
    size_t size() const;

	/// \brief Return a pointer to the memory managed by the
	///         object.
	///
	/// @return a pointer to the data managed by the object
	///
	///////////////////////////////////////////////////////////
	std::uint8_t* data();

    const std::uint8_t* data() const;

	/// \brief Return true if the size of the managed memory
	///         is 0.
	///
	/// @return true if the managed memory's size is 0 or
	///          false otherwise
	///
	///////////////////////////////////////////////////////////
    bool empty() const;

	/// \brief Copy the specified array of bytes into the
	///         managed memory.
	///
	/// @param pSource      a pointer to the buffer containing
	///                      the bytes to be copied
	/// @param sourceLength the number of bytes stored in the
	///                      buffer pSource and to be copied
	///                      into the managed memory
	///
	///////////////////////////////////////////////////////////
    void assign(const std::uint8_t* pSource, const size_t sourceLength);

    /// \brief Copy the specified array of bytes into a region
    ///         of the managed memory.
    ///
    /// @param pSource      a pointer to the buffer containing
    ///                      the bytes to be copied
    /// @param sourceLength the number of bytes stored in the
    ///                      buffer pSource and to be copied
    ///                      into the managed memory
    /// @param destinationOffset the offset at which the data
    ///                      must be copied
    ///
    ///////////////////////////////////////////////////////////
    void assignRegion(const std::uint8_t* pSource, const size_t sourceLength, const size_t destinationOffset);


protected:
    std::unique_ptr<stringUint8> m_pMemoryBuffer;
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief Stores unused memory objects (see 
///         \ref imebra::memory) so they can be reused 
///         when needed.
///
/// One instance of this class is statically allocated
///  by the library. Don't allocate new instance of this
///  class.
///
/// To obtain a pointer to the statically allocated 
///  instance of memoryPool call the static function
///  memoryPool::getMemoryPool().
///
/// When the reference counter of a \ref memory object
///  reaches 0, the memory object may not be deleted 
///  immediatly; infact, if the memory managed by the 
///  memory pool matches some predefined requisites, the
///  memory object is temporarily stored in the memory
///  pool and reused when a request for a \ref memory
///  object is received.
///
/// The memory pool tries to reuse the \ref memory
///  object that manages an amount of memory similar to
///  the amount of memory requested through getMemory().
///
/// When a memory object is not used for a while then it
///  is deleted permanently.
///
///////////////////////////////////////////////////////////
class memoryPool
{
	friend class memory;
    friend class memoryPoolGetter;

    memoryPool(size_t memoryMinSize, size_t poolMaxSize);
public:
    ~memoryPool();

    void setMinMaxMemory(size_t memoryMinSize, size_t poolMaxSize);

    size_t getUnusedMemorySize();

	/// \brief Discard all the currently unused memory.
	///
    /// \return true if some unused memory has been deleted,
    ///         false if the memory pool was already empty
	///////////////////////////////////////////////////////////
    bool flush();

protected:
    /// \brief Retrieve a new or reused
    ///         \ref imebra::memory object.
    ///
    /// The function look for an unused \ref memory object
    ///  that has a managed string with the same size of the
    ///  specified one and tries to reuse it.
    ///
    /// If none of the unused objects has the requested
    ///  size, then a new memory object is created and
    ///  returned.
    ///
    /// @param requestedSize the size that the string managed
    ///                       by the returned memory object
    ///                       must have
    /// @return              a pointer to the reused or new
    ///                       memory object: in any case the
    ///                       reference counter of the returned
    ///                       object will be 1
    ///
    ///////////////////////////////////////////////////////////
    stringUint8* getMemory(size_t requestedSize);

    /// \internal
	/// \brief Called by \ref memory before the object
	///         is deleted.
	///
    /// This function returns takes ownership of the object and
    ///  will delete it when necessary
	///
	/// @param pMemoryToReuse a pointer to the memory object
	///                        that call this function
	///
	///////////////////////////////////////////////////////////
    void reuseMemory(stringUint8* pMemoryToReuse);

    std::array<size_t, IMEBRA_MEMORY_POOL_SLOTS> m_memorySize;
    std::array<stringUint8*, IMEBRA_MEMORY_POOL_SLOTS>  m_memoryPointer;

    size_t m_minMemoryBlockSize;
    size_t m_maxMemoryUsageSize;
    size_t m_actualSize;
    size_t m_firstUsedCell;
    size_t m_firstFreeCell;

};

class memoryPoolGetter
{
protected:
    memoryPoolGetter();
    ~memoryPoolGetter();

public:
    static memoryPoolGetter& getMemoryPoolGetter();

    memoryPool& getMemoryPoolLocal();

protected:
#ifdef __APPLE__
    static void deleteMemoryPool(void* pMemoryPool);
    pthread_key_t m_key;
#endif
    std::new_handler m_oldNewHandler;

protected:
    /// \internal
    /// \brief Call flush(). Throws bad_alloc() if flush()
    ///        returns false.
    ///
    ///////////////////////////////////////////////////////////
    static void newHandler();

#ifndef __APPLE__
    thread_local static std::unique_ptr<memoryPool> m_pool;
#endif
};

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraMemory_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_)
