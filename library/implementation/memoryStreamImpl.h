/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file memoryStream.h
    \brief Declaration of the memoryStream class.

*/

#if !defined(imebraMemoryStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
#define imebraMemoryStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_

#include "baseStreamImpl.h"
#include "memoryImpl.h"
#include <mutex>

namespace imebra
{

namespace implementation
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class derives from the baseStream 
///         class and implements a memory stream.
///
/// This class can be used to read/write on the allocated
///  memory.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class memoryStreamInput : public baseStreamInput
{

public:
	/// \brief Construct a memoryStream object and attach a
	///         memory object to it.
	///
	/// The attached memory object will be resized if new data
	///  is written and its size is too small.
	///
	/// @param memoryStream the memory object to be used by
	///                      the memoryStream object.
	///
	///////////////////////////////////////////////////////////
    memoryStreamInput(std::shared_ptr<const memory> memoryStream);

	///////////////////////////////////////////////////////////
	//
	// Virtual stream's functions
	//
	///////////////////////////////////////////////////////////
    virtual size_t read(size_t startPosition, std::uint8_t* pBuffer, size_t bufferLength) override;

    virtual void terminate() override;

    virtual bool seekable() const override;


protected:
    std::shared_ptr<const memory> m_memory;

    std::mutex m_mutex;
};

class memoryStreamOutput : public baseStreamOutput
{

public:
    /// \brief Construct a memoryStream object and attach a
    ///         memory object to it.
    ///
    /// The attached memory object will be resized if new data
    ///  is written and its size is too small.
    ///
    /// @param memoryStream the memory object to be used by
    ///                      the memoryStream object.
    ///
    ///////////////////////////////////////////////////////////
    memoryStreamOutput(std::shared_ptr<memory> memoryStream);

    ///////////////////////////////////////////////////////////
    //
    // Virtual stream's functions
    //
    ///////////////////////////////////////////////////////////
    virtual void write(size_t startPosition, const std::uint8_t* pBuffer, size_t bufferLength) override;

protected:
    std::shared_ptr<memory> m_memory;

    std::mutex m_mutex;
};

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraMemoryStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
