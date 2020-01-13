/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file memoryStream.cpp
    \brief Implementation of the memoryStream class.

*/

#include "exceptionImpl.h"
#include "memoryStreamImpl.h"
#include <string.h>

namespace imebra
{

namespace implementation
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// memoryStream
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
memoryStreamInput::memoryStreamInput(std::shared_ptr<const memory> memoryStream): m_memory(memoryStream)
{
}


memoryStreamOutput::memoryStreamOutput(std::shared_ptr<memory> memoryStream): m_memory(memoryStream)
{
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write raw data into the stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void memoryStreamOutput::write(size_t startPosition, const std::uint8_t* pBuffer, size_t bufferLength)
{
    IMEBRA_FUNCTION_START();

	// Nothing happens if we have nothing to write
	///////////////////////////////////////////////////////////
	if(bufferLength == 0)
	{
		return;
	}

    std::lock_guard<std::mutex> lock(m_mutex);

	// Copy the buffer into the memory
	///////////////////////////////////////////////////////////
	if(startPosition + bufferLength > m_memory->size())
	{
        size_t newSize = startPosition + bufferLength;
        size_t reserveSize = ((newSize + 1023) >> 10) << 10; // preallocate blocks of 1024 bytes
		m_memory->reserve(reserveSize);
		m_memory->resize(startPosition + bufferLength);
	}

	::memcpy(m_memory->data() + startPosition, pBuffer, bufferLength);

	IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read raw data from the stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
size_t memoryStreamInput::read(size_t startPosition, std::uint8_t* pBuffer, size_t bufferLength)
{
    IMEBRA_FUNCTION_START();

	if(bufferLength == 0)
	{
		return 0;
	}

    std::lock_guard<std::mutex> lock(m_mutex);

	// Don't read if the requested position isn't valid
	///////////////////////////////////////////////////////////
    size_t memorySize = m_memory->size();
	if(startPosition >= memorySize)
	{
		return 0;
	}

	// Check if all the bytes are available
	///////////////////////////////////////////////////////////
    size_t copySize = bufferLength;
	if(startPosition + bufferLength > memorySize)
	{
		copySize = memorySize - startPosition;
	}

	if(copySize == 0)
	{
		return 0;
	}

	::memcpy(pBuffer, m_memory->data() + startPosition, copySize);

	return copySize;

	IMEBRA_FUNCTION_END();
}


void memoryStreamInput::terminate()
{

}


bool memoryStreamInput::seekable() const
{
    return true;
}


} // namespace implementation

} // namespace imebra
