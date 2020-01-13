/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file baseSequenceStreamImpl.cpp
    \brief Implementation of the baseSequenceStreamImpl class.

*/

#include "baseSequenceStreamImpl.h"

namespace imebra
{

namespace implementation
{

baseSequenceStreamInput::baseSequenceStreamInput():
    m_currentPosition(0)
{
}


size_t baseSequenceStreamInput::read(size_t startPosition, std::uint8_t* pBuffer, size_t bufferLength)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    if(startPosition < m_currentPosition)
    {
        throw std::logic_error("Cannot seek backward while reading from a sequence stream");
    }
    if(startPosition > m_currentPosition)
    {
        // The following buffer can be static because we  only write
        // inside it, never read from it, so multithreading issues
        // are not a problem
        static std::uint8_t buffer[32768];
        while(m_currentPosition < startPosition)
        {
            size_t readSize(startPosition - m_currentPosition);
            if(readSize > sizeof(buffer))
            {
                readSize = sizeof(buffer);
            }
            size_t availableBytes(read(buffer, readSize));
            if(availableBytes == 0)
            {
                return 0;
            }
            m_currentPosition += availableBytes;
        }
    }
    size_t returnBytes(read(pBuffer, bufferLength));
    m_currentPosition += returnBytes;

    return returnBytes;

    IMEBRA_FUNCTION_END();
}



baseSequenceStreamOutput::baseSequenceStreamOutput():
    m_currentPosition(0)
{
}

void baseSequenceStreamOutput::write(size_t startPosition, const std::uint8_t* pBuffer, size_t bufferLength)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    if(startPosition < m_currentPosition)
    {
        throw std::logic_error("Cannot seek backward while writing to a sequence stream");
    }
    if(startPosition > m_currentPosition)
    {
        // The following buffer can be static because we  only read from
        // it, never write into it, so multithreading issues are not a
        // problem
        static std::uint8_t buffer[32768] = {0};
        while(m_currentPosition < startPosition)
        {
            size_t writeSize(startPosition - m_currentPosition);
            if(writeSize > sizeof(buffer))
            {
                writeSize = sizeof(buffer);
            }
            write(buffer, writeSize);
            m_currentPosition += writeSize;
        }
    }
    write(pBuffer, bufferLength);

    m_currentPosition += bufferLength;

    IMEBRA_FUNCTION_END();
}


} // namespace implementation


} // namespace imebra

