/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file memoryPool.h
    \brief Declaration of the class MemoryPool.

*/

#if !defined(imebraMemoryPool__INCLUDED_)
#define imebraMemoryPool__INCLUDED_

#include <memory>
#include <string>
#include "definitions.h"

namespace imebra
{

namespace implementation
{
    class memoryPool;
}

///
/// \brief Used by Imebra to allocate memory.
///
/// MemoryPool keeps around recently deleted memory regions so they can be
/// repurposed quickly when new memory regions are requested.
///
/// Each thread has its own MemoryPool object.
///
///////////////////////////////////////////////////////////////////////////////
class IMEBRA_API MemoryPool
{
public:
    /// \brief Release all the unused memory regions.
    ///
    ///////////////////////////////////////////////////////////////////////////////
    static void flush();

    /// \brief Return the total size of the memory that has been released but not
    ///        yet freed.
    ///
    /// \return the total size of the memory released but not yet freed
    ///
    ///////////////////////////////////////////////////////////////////////////////
    static size_t getUnusedMemorySize();

    /// \brief Set the maximum size of the kept unused memory.
    ///
    /// \param minMemoryBlockSize  if a memory region is smaller than this size
    ///                            then when it is released it is deleted
    ///                            immediately, otherwise it is kept in the memory
    ///                            pool
    /// \param maxMemoryPoolSize   the maximum size of the sum of all the unused
    ///                            memory regions. When the total size of the
    ///                            unused memory regions is greater than this
    ///                            parameter then the oldest memory regions are
    ///                            deleted permanently
    ///
    ///////////////////////////////////////////////////////////////////////////////
    static void setMemoryPoolSize(size_t minMemoryBlockSize, size_t maxMemoryPoolSize);
};

}

#endif // !defined(imebraMemoryPool__INCLUDED_)
