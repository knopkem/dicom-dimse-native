/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file streamController.cpp
    \brief Implementation of the streamController class.

*/

#include "streamControllerImpl.h"
#include <memory.h>

namespace imebra
{

// Used for the endian check
///////////////////////////////////////////////////////////
static const std::uint16_t m_endianCheck(0x00ff);
static std::uint8_t const * const pBytePointer((std::uint8_t*)&m_endianCheck);
static const streamController::tByteOrdering m_platformByteOrder((*pBytePointer)==0xff ? streamController::lowByteEndian : streamController::highByteEndian);

///////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////
streamController::streamController(size_t virtualStart /* =0 */, size_t virtualLength /* =0 */):
    m_bJpegTags(false),
        m_dataBuffer(size_t(IMEBRA_STREAM_CONTROLLER_MEMORY_SIZE), 0),
        m_virtualStart(virtualStart),
        m_virtualLength(virtualLength),
        m_dataBufferStreamPosition(0),
        m_dataBufferCurrent(0), m_dataBufferEnd(0)
{
}


streamController::streamController(size_t virtualStart, size_t virtualLength,
                 std::uint8_t* pBuffer, size_t bufferSize):
    m_bJpegTags(false),
        m_dataBuffer(size_t(IMEBRA_STREAM_CONTROLLER_MEMORY_SIZE), 0),
        m_virtualStart(virtualStart),
        m_virtualLength(virtualLength),
        m_dataBufferStreamPosition(0),
        m_dataBufferCurrent(0), m_dataBufferEnd(bufferSize)
{
    ::memcpy(&(m_dataBuffer[0]), pBuffer, bufferSize);
}




///////////////////////////////////////////////////////////
//
// Destructor
//
///////////////////////////////////////////////////////////
streamController::~streamController()
{
}


///////////////////////////////////////////////////////////
//
// Retrieve the current position
//
///////////////////////////////////////////////////////////
size_t streamController::position()
{
    return m_dataBufferStreamPosition + m_dataBufferCurrent;
}


///////////////////////////////////////////////////////////
//
// Retrieve the position without considering the virtual
//  start's position
//
///////////////////////////////////////////////////////////
size_t streamController::getControlledStreamPosition()
{
    return m_dataBufferStreamPosition + m_dataBufferCurrent + m_virtualStart;
}


void streamController::reverseEndian(std::uint8_t* pBuffer, const size_t wordLength, const size_t words)
{
    switch(wordLength)
    {
    case 2:
        {
            std::uint16_t* pWord((std::uint16_t*)pBuffer);
            for(size_t scanWords = words; scanWords != 0; --scanWords)
            {
                *pWord = (std::uint16_t)(((*pWord & 0x00ff) << 8) | ((*pWord & 0xff00) >> 8));
                ++pWord;
            }
        }
        return;
    case 4:
        {
            std::uint32_t* pDWord((std::uint32_t*)pBuffer);
            for(size_t scanWords = words; scanWords != 0; --scanWords)
            {
                *pDWord = ((*pDWord & 0xff000000) >> 24) | ((*pDWord & 0x00ff0000) >> 8) | ((*pDWord & 0x0000ff00) << 8) | ((*pDWord & 0x000000ff) << 24);
                ++pDWord;
            }
        }
        return;
    case 8:
        {
            std::uint64_t* pQWord((std::uint64_t*)pBuffer);
            for(std::uint64_t scanWords = words; scanWords != 0; --scanWords)
            {
                *pQWord =
                        ((*pQWord & 0xff00000000000000) >> 56) |
                        ((*pQWord & 0x00ff000000000000) >> 40) |
                        ((*pQWord & 0x0000ff0000000000) >> 24) |
                        ((*pQWord & 0x000000ff00000000) >> 8) |
                        ((*pQWord & 0x00000000ff000000) << 8) |
                        ((*pQWord & 0x0000000000ff0000) << 24) |
                        ((*pQWord & 0x000000000000ff00) << 40) |
                        ((*pQWord & 0x00000000000000ff) << 56);
                ++pQWord;
            }
        }
        return;
    }

}

///////////////////////////////////////////////////////////
//
// Adjust the byte ordering of pBuffer
//
///////////////////////////////////////////////////////////
void streamController::adjustEndian(std::uint8_t* pBuffer, const size_t wordLength, const tByteOrdering endianType, const size_t words /* =1 */)
{
    if(endianType != m_platformByteOrder && wordLength >= 2u)
    {
        reverseEndian(pBuffer, wordLength, words);
    }
}

std::uint16_t streamController::adjustEndian(std::uint16_t buffer, const tByteOrdering endianType)
{
    if(endianType == m_platformByteOrder)
    {
        return buffer;
    }
    return (std::uint16_t)(((buffer & 0xff00) >> 8) | ((buffer & 0xff) << 8));
}

std::uint32_t streamController::adjustEndian(std::uint32_t buffer, const tByteOrdering endianType)
{
    if(endianType == m_platformByteOrder)
    {
        return buffer;
    }
    return (std::uint32_t)(
            ((buffer & 0xff000000) >> 24) |
            ((buffer & 0x00ff0000) >> 8) |
            ((buffer & 0x0000ff00) << 8) |
            ((buffer & 0x000000ff) << 24) );
}

std::uint64_t streamController::adjustEndian(std::uint64_t buffer, const tByteOrdering endianType)
{
    if(endianType == m_platformByteOrder)
    {
        return buffer;
    }
    return (std::uint64_t)(
            ((buffer & 0xff00000000000000) >> 56) |
            ((buffer & 0x00ff000000000000) >> 40) |
            ((buffer & 0x0000ff0000000000) >> 24) |
            ((buffer & 0x000000ff00000000) >> 8) |
            ((buffer & 0x00000000ff000000) << 8) |
            ((buffer & 0x0000000000ff0000) << 24) |
            ((buffer & 0x000000000000ff00) << 40) |
            ((buffer & 0x00000000000000ff) << 56));
}

streamController::tByteOrdering streamController::getPlatformEndian()
{
    return m_platformByteOrder;
}

} // namespace imebra
