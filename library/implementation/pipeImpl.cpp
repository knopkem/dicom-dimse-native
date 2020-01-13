/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file pipeImpl.cpp
    \brief Implementation of the the pipe stream used by the imebra library.

*/

#include "pipeImpl.h"
#include "memoryImpl.h"
#include "../include/imebra/exceptions.h"
#include <memory.h>
#include <chrono>

namespace imebra
{

namespace implementation
{


//
// Constructor.
//
///////////////////////////////////////////////////////////
pipeSequenceStream::pipeSequenceStream(size_t bufferSize):
    m_pMemory(std::make_shared<memory>(bufferSize)),
    m_bTerminate(false),
    m_writePosition(0),
    m_readPosition(0),
    m_availableData(0)
{
}


//
// Destructor. Causes the termination of pending
//
///////////////////////////////////////////////////////////

pipeSequenceStream::~pipeSequenceStream()
{
    terminate();
}


//
// Read operation
//
///////////////////////////////////////////////////////////
size_t pipeSequenceStream::read(std::uint8_t* pBuffer, size_t bufferLength)
{
    IMEBRA_FUNCTION_START();

    std::unique_lock<std::mutex> lock(m_positionMutex);

    // Execute until termination or some data has been read
    ///////////////////////////////////////////////////////////
    for(;;)
    {
        if(m_bTerminate.load())
        {
            IMEBRA_THROW(StreamClosedError, "The pipe has been closed");
        }
        if(m_availableData != 0)
        {
            size_t readData(0);
            if(m_readPosition < m_writePosition)
            {
                readData = m_writePosition - m_readPosition;
            }
            else
            {
                readData = m_pMemory->size() - m_readPosition;
            }
            if(readData > bufferLength)
            {
                readData = bufferLength;
            }
            ::memcpy(pBuffer, m_pMemory->data() + m_readPosition, readData);
            m_readPosition += readData;
            if(m_readPosition == m_pMemory->size())
            {
                m_readPosition = 0;
            }
            m_availableData -= readData;
            m_positionConditionVariable.notify_all();
            return readData;
        }

        if(m_availableData == 0)
        {
            m_positionConditionVariable.wait_for(lock, std::chrono::milliseconds(IMEBRA_PIPE_TIMEOUT_MS));
        }
    }

    IMEBRA_FUNCTION_END();
}


//
// Write operation
//
///////////////////////////////////////////////////////////
void pipeSequenceStream::write(const std::uint8_t* pBuffer, size_t bufferLength)
{
    IMEBRA_FUNCTION_START();

    std::unique_lock<std::mutex> lock(m_positionMutex);

    const std::uint8_t* pWriteData(pBuffer);
    size_t remainingData(bufferLength);

    // Loop until termination or all the data has been written
    ///////////////////////////////////////////////////////////
    while(remainingData != 0)
    {
        if(m_bTerminate.load())
        {
            IMEBRA_THROW(StreamClosedError, "The pipe has been closed");
        }

        while(remainingData != 0 && m_availableData != m_pMemory->size())
        {
            size_t writeData(0);
            if(m_writePosition < m_readPosition)
            {
                writeData = m_readPosition - m_writePosition;
            }
            else
            {
                writeData = m_pMemory->size() - m_writePosition;
            }
            if(writeData > remainingData)
            {
                writeData = remainingData;
            }
            ::memcpy(m_pMemory->data() + m_writePosition, pWriteData, writeData);
            pWriteData += writeData;
            m_writePosition += writeData;
            if(m_writePosition == m_pMemory->size())
            {
                m_writePosition = 0;
            }
            remainingData -= writeData;
            m_availableData += writeData;

            m_positionConditionVariable.notify_all();
        }

        if(remainingData != 0 && m_availableData == m_pMemory->size())
        {
            m_positionConditionVariable.wait_for(lock, std::chrono::milliseconds(IMEBRA_PIPE_TIMEOUT_MS));
        }
    }

    IMEBRA_FUNCTION_END();
}


//
// Close the pipe gracefully (waits for the pending data
// to be read)
//
///////////////////////////////////////////////////////////
void pipeSequenceStream::close(unsigned int timeoutMilliseconds)
{
    std::unique_lock<std::mutex> lock(m_positionMutex);

    if(m_bTerminate.load())
    {
        return;
    }

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    std::chrono::milliseconds timeout(timeoutMilliseconds);
    while( (m_availableData != 0) && ((std::chrono::steady_clock::now() - now) < timeout) )
    {
        m_positionConditionVariable.wait_for(lock, std::chrono::milliseconds(IMEBRA_PIPE_TIMEOUT_MS));
    }

    m_bTerminate.store(true);
    m_positionConditionVariable.notify_all();
}


//
// Forces termination of pending operations
//
///////////////////////////////////////////////////////////
void pipeSequenceStream::terminate()
{
    std::unique_lock<std::mutex> lock(m_positionMutex);
    m_bTerminate.store(true);
    m_positionConditionVariable.notify_all();
}


pipeSequenceStreamInput::pipeSequenceStreamInput(std::shared_ptr<pipeSequenceStream> pPipeStream):
    m_pPipeStream(pPipeStream)
{
}

size_t pipeSequenceStreamInput::read(std::uint8_t* pBuffer, size_t bufferLength)
{
    return m_pPipeStream->read(pBuffer, bufferLength);
}

void pipeSequenceStreamInput::terminate()
{
    m_pPipeStream->terminate();
}

pipeSequenceStreamOutput::pipeSequenceStreamOutput(std::shared_ptr<pipeSequenceStream> pPipeStream):
    m_pPipeStream(pPipeStream)
{
}

void pipeSequenceStreamOutput::write(const std::uint8_t* pBuffer, size_t bufferLength)
{
    m_pPipeStream->write(pBuffer, bufferLength);
}


} // namespace implementation

} // namespace imebra


