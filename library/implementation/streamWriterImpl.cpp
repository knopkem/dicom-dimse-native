/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file streamWriter.cpp
    \brief Implementation of the streamWriter class.

*/

#include "streamWriterImpl.h"
#include <string.h>
#include "../include/imebra/exceptions.h"

namespace imebra
{

namespace implementation
{

///////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////
streamWriter::streamWriter(std::shared_ptr<baseStreamOutput> pControlledStream):
    streamController(0, 0),
    m_pControlledStream(pControlledStream),
    m_outBitsBuffer(0),
    m_outBitsNum(0)
{
}


///////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////
streamWriter::streamWriter(std::shared_ptr<baseStreamOutput> pControlledStream, size_t virtualStart, size_t virtualLength):
    streamController(virtualStart, virtualLength),
    m_pControlledStream(pControlledStream),
    m_outBitsBuffer(0),
    m_outBitsNum(0)
{
}


///////////////////////////////////////////////////////////
//
// Destructor
//
///////////////////////////////////////////////////////////
streamWriter::~streamWriter()
{
    try
    {
        flushDataBuffer();
    }
    catch(const StreamClosedError&)
    {

    }
}


///////////////////////////////////////////////////////////
//
// Flush the data buffer
//
///////////////////////////////////////////////////////////
void streamWriter::flushDataBuffer()
{
    IMEBRA_FUNCTION_START();

    if(m_dataBufferCurrent == 0)
    {
        return;
    }
    m_pControlledStream->write(m_dataBufferStreamPosition + m_virtualStart, m_dataBuffer.data(), m_dataBufferCurrent);
    m_dataBufferStreamPosition += m_dataBufferCurrent;
    m_dataBufferCurrent = 0;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Write into the stream
//
///////////////////////////////////////////////////////////
void streamWriter::write(const std::uint8_t* pBuffer, size_t bufferLength)
{
    IMEBRA_FUNCTION_START();

    while(bufferLength != 0)
    {
        if(m_dataBufferCurrent == m_dataBuffer.size())
        {
            flushDataBuffer();
            if(bufferLength > (size_t)(m_dataBuffer.size() - m_dataBufferCurrent) )
            {
                m_pControlledStream->write(m_dataBufferStreamPosition + m_virtualStart, pBuffer, bufferLength);
                m_dataBufferStreamPosition += bufferLength;
                return;
            }
        }
        size_t copySize = (size_t)(m_dataBuffer.size() - m_dataBufferCurrent);
        if(copySize > bufferLength)
        {
            copySize = bufferLength;
        }
        ::memcpy(&(m_dataBuffer[m_dataBufferCurrent]), pBuffer, copySize);
        pBuffer += copySize;
        bufferLength -= copySize;
        m_dataBufferCurrent += copySize;
    }

    IMEBRA_FUNCTION_END();
}

} // namespace implementation

} // namespace imebra
