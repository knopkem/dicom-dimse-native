/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file streamReader.cpp
    \brief Implementation of the streamReader class.

*/

#include "streamReaderImpl.h"
#include "streamWriterImpl.h"
#include <string.h>
#include <vector>

namespace imebra
{

namespace implementation
{

///////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////
streamReader::streamReader(std::shared_ptr<baseStreamInput> pControlledStream):
    streamController(0, 0),
    m_pControlledStream(pControlledStream)
{
}

streamReader::streamReader(std::shared_ptr<baseStreamInput> pControlledStream, size_t virtualStart, size_t virtualLength):
    streamController(virtualStart, virtualLength),
    m_pControlledStream(pControlledStream)
{
    IMEBRA_FUNCTION_START();

    if(virtualLength == 0)
    {
        IMEBRA_THROW(StreamEOFError, "Virtual stream with zero length");
    }

    IMEBRA_FUNCTION_END();
}


streamReader::streamReader(std::shared_ptr<baseStreamInput> pControlledStream, size_t virtualStart, size_t virtualLength, std::uint8_t* pBuffer, size_t bufferLength):
    streamController(virtualStart, virtualLength, pBuffer, bufferLength),
    m_pControlledStream(pControlledStream)
{
    IMEBRA_FUNCTION_START();

    if(virtualLength == 0)
    {
        IMEBRA_THROW(StreamEOFError, "Virtual stream with zero length");
    }

    IMEBRA_FUNCTION_END();
}


streamReader::streamReader(std::shared_ptr<baseStreamInput> pControlledStream, size_t virtualStart, std::uint8_t* pBuffer, size_t bufferLength):
    streamController(virtualStart, 0, pBuffer, bufferLength),
    m_pControlledStream(pControlledStream)
{
}


streamReader::~streamReader()
{
    // If this is a virtual stream then read the remaining bytes
    try
    {
        if(m_virtualLength != 0 && position() != m_virtualLength)
        {
            std::vector<std::uint8_t> buffer(m_virtualLength - position());
            read(buffer.data(), buffer.size());
        }
    }
    catch(const StreamEOFError&)
    {
        // Avoid rethrowing
    }
}

std::shared_ptr<baseStreamInput> streamReader::getControlledStream()
{
    return m_pControlledStream;
}

std::shared_ptr<streamReader> streamReader::getReader(size_t virtualLength)
{
    IMEBRA_FUNCTION_START();

    if(virtualLength == 0)
    {
        IMEBRA_THROW(StreamEOFError, "Virtual stream with zero length");
    }

    size_t currentPosition = position();
    if(currentPosition + virtualLength > m_virtualLength && m_virtualLength != 0)
    {
        virtualLength = m_virtualLength - currentPosition;
    }

    size_t readerBufferSize = m_dataBufferEnd - m_dataBufferCurrent;
    if(readerBufferSize > virtualLength)
    {
        readerBufferSize = virtualLength;
    }
    std::uint8_t* pReaderBuffer = &(m_dataBuffer[m_dataBufferCurrent]);

    // Use seek instead of seekforward to avoid copying data into the forwarded writers
    seek(position() + virtualLength);

    std::shared_ptr<streamReader> reader = std::make_shared<streamReader>(
                m_pControlledStream,
                currentPosition + m_virtualStart,
                virtualLength,
                pReaderBuffer,
                readerBufferSize);

    for(std::shared_ptr<streamWriter>& pWriter: m_forwardStream)
    {
        reader->addForwardWriter(pWriter);
    }

    return reader;

    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
//
// Returns true if the last byte has been read
//
///////////////////////////////////////////////////////////
bool streamReader::endReached()
{
    IMEBRA_FUNCTION_START();

    try
    {
        return (m_dataBufferCurrent == m_dataBufferEnd && fillDataBuffer() == 0);
    }
    catch(StreamClosedError&)
    {
        return true;
    }
    catch(StreamEOFError&)
    {
        return true;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Refill the data buffer
//
///////////////////////////////////////////////////////////
size_t streamReader::fillDataBuffer()
{
    IMEBRA_FUNCTION_START();

    size_t readBytes = fillDataBuffer(&(m_dataBuffer[0]), m_dataBuffer.size());
    if(readBytes == 0)
    {
        m_dataBufferCurrent = m_dataBufferEnd = 0;
        return 0;
    }
    m_dataBufferEnd = readBytes;
    m_dataBufferCurrent = 0;
    return readBytes;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Read data from the stream into the specified buffer
//
///////////////////////////////////////////////////////////
size_t streamReader::fillDataBuffer(std::uint8_t* pDestinationBuffer, size_t readLength)
{
    IMEBRA_FUNCTION_START();

    m_dataBufferStreamPosition = position();
    if(m_virtualLength != 0)
    {
        if(m_dataBufferStreamPosition >= m_virtualLength)
        {
            m_dataBufferStreamPosition = m_virtualLength;
            return 0;
        }
        if(m_dataBufferStreamPosition + readLength > m_virtualLength)
        {
            readLength = m_virtualLength - m_dataBufferStreamPosition;
        }
    }
    return m_pControlledStream->read(m_dataBufferStreamPosition + m_virtualStart, pDestinationBuffer, readLength);

    IMEBRA_FUNCTION_END();
}



///////////////////////////////////////////////////////////
//
// Return the specified number of bytes from the stream
//
///////////////////////////////////////////////////////////
void streamReader::read(std::uint8_t* pBuffer, size_t bufferLength)
{
    // Special case with length == 1 (read just one byte)
    if(bufferLength == 1 && m_dataBufferCurrent != m_dataBufferEnd)
    {
        *pBuffer = m_dataBuffer[m_dataBufferCurrent++];

        for(std::shared_ptr<streamWriter>& pWriter: m_forwardStream)
        {
            pWriter->write(pBuffer, bufferLength);
        }

        return;
    }

    IMEBRA_FUNCTION_START();

    std::uint8_t* pReadBuffer(pBuffer);
    while(bufferLength != 0)
    {
        size_t readBytes = readSome(pReadBuffer, bufferLength);
        pReadBuffer += readBytes;
        bufferLength -= readBytes;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Return the specified number of bytes from the stream
//
///////////////////////////////////////////////////////////
size_t streamReader::readSome(std::uint8_t* pBuffer, size_t bufferLength)
{
    IMEBRA_FUNCTION_START();

    size_t originalSize(bufferLength);

    std::uint8_t* const pOriginalBuffer(pBuffer);

    while(bufferLength != 0)
    {
        // Update the data buffer if it is empty
        ///////////////////////////////////////////////////////////
        if(m_dataBufferCurrent == m_dataBufferEnd)
        {
            if(bufferLength != originalSize)
            {
                for(std::shared_ptr<streamWriter>& pWriter: m_forwardStream)
                {
                    pWriter->write(pOriginalBuffer, originalSize - bufferLength);
                }
                return originalSize - bufferLength;
            }
            if(bufferLength >= m_dataBuffer.size())
            {
                // read the data directly into the destination buffer
                ///////////////////////////////////////////////////////////
                size_t readBytes(fillDataBuffer(pBuffer, bufferLength));

                m_dataBufferCurrent = m_dataBufferEnd = 0;
                m_dataBufferStreamPosition += readBytes;
                pBuffer += readBytes;
                bufferLength -= readBytes;
                if(readBytes == 0)
                {
                    IMEBRA_THROW(StreamEOFError, "Attempt to read past the end of the file");
                }
                continue;
            }

            if(fillDataBuffer() == 0)
            {
                IMEBRA_THROW(StreamEOFError, "Attempt to read past the end of the file");
            }
        }

        // Copy the available data into the return buffer
        ///////////////////////////////////////////////////////////
        size_t copySize = bufferLength;
        size_t maxSize = (size_t)(m_dataBufferEnd - m_dataBufferCurrent);
        if(copySize > maxSize)
        {
            copySize = maxSize;
        }
        ::memcpy(pBuffer, &(m_dataBuffer[m_dataBufferCurrent]), copySize);
        bufferLength -= copySize;
        pBuffer += copySize;
        m_dataBufferCurrent += copySize;
    }

    for(std::shared_ptr<streamWriter>& pWriter: m_forwardStream)
    {
        pWriter->write(pOriginalBuffer, originalSize);
    }

    return originalSize;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Causes current and subsequent read operations
// to fail with StreamClosedError.
//
///////////////////////////////////////////////////////////
void streamReader::terminate()
{
    m_pControlledStream->terminate();
}


///////////////////////////////////////////////////////////
//
// Seek the read position
//
///////////////////////////////////////////////////////////
void streamReader::seek(size_t newPosition)
{
    // The requested position is already in the data buffer?
    ///////////////////////////////////////////////////////////
    size_t bufferEndPosition = m_dataBufferStreamPosition + m_dataBufferEnd;
    if(newPosition >= m_dataBufferStreamPosition && newPosition < bufferEndPosition)
    {
        m_dataBufferCurrent = newPosition - m_dataBufferStreamPosition;
        return;
    }

    // The requested position is not in the data buffer
    ///////////////////////////////////////////////////////////
    m_dataBufferCurrent = m_dataBufferEnd = 0;
    m_dataBufferStreamPosition = newPosition;
}

void streamReader::seekForward(std::uint32_t newPosition)
{
    IMEBRA_FUNCTION_START();

    if(m_forwardStream.empty())
    {
        seek(position() + (size_t)newPosition);
    }
    else
    {
        // Use read to forward the data to the write streams
        std::uint8_t buffer[IMEBRA_STREAM_CONTROLLER_MEMORY_SIZE];
        size_t readSize(sizeof(buffer));
        while(newPosition != 0)
        {
            if(readSize > newPosition)
            {
                readSize = newPosition;
            }
            read(buffer, readSize);
            newPosition -= (std::uint32_t)readSize;

        }
    }

    IMEBRA_FUNCTION_END();
}

bool streamReader::seekable() const
{
    return m_pControlledStream->seekable();
}

size_t streamReader::getVirtualLength() const
{
    return m_virtualLength;
}

void streamReader::addForwardWriter(const std::shared_ptr<streamWriter>& pWriter)
{
    m_forwardStream.push_back(pWriter);
}

void streamReader::removeForwardWriter(const std::shared_ptr<streamWriter>& pWriter)
{
    for(forwardList_t::iterator scan(m_forwardStream.begin()); scan != m_forwardStream.end(); ++scan)
    {
        if((*scan).get() == pWriter.get())
        {
            m_forwardStream.erase(scan);
            return;
        }
    }
}


forwardStream::forwardStream(const std::shared_ptr<streamReader>& pSource, const std::shared_ptr<streamWriter>& pDestination):
    m_pSource(pSource), m_pDestination(pDestination)
{
    m_pSource->addForwardWriter(pDestination);
}

forwardStream::~forwardStream()
{
    m_pSource->removeForwardWriter(m_pDestination);
}


} // namespace implementation

} // namespace imebra
