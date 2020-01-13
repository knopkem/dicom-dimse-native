/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file stream.cpp
    \brief Implementation of the stream class.

*/

#include "exceptionImpl.h"
#include "fileStreamImpl.h"
#include "../include/imebra/exceptions.h"

#include <sstream>
#include <errno.h>
#include <locale>
#include <codecvt>


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
// Stream
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
// Open a file (unicode)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
fileStream::fileStream(const std::wstring& fileName, openMode mode)
{
    IMEBRA_FUNCTION_START();

#if defined(IMEBRA_WINDOWS)
    std::wstring strMode;
    switch(mode)
    {
    case openMode::read:
        strMode = L"rb";
        break;
    case openMode::write:
        strMode = L"wb";
        break;
    }

    errno_t errorCode = ::_wfopen_s(&m_openFile, fileName.c_str(), strMode.c_str());
     if (errorCode != 0)
     {
         m_openFile = 0;
     }
#else
    // Convert the filename to UTF8
    std::string utf8FileName(std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.to_bytes(fileName));

    std::string strMode;
    switch(mode)
    {
    case openMode::read:
        strMode = "rb";
        break;
    case openMode::write:
        strMode = "wb";
        break;
    }

    m_openFile = ::fopen(utf8FileName.c_str(), strMode.c_str());
    int errorCode = errno;
#endif
    if(m_openFile == 0)
    {
        IMEBRA_THROW(StreamOpenError, "stream::openFile failure - error code: " << errorCode);
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Open a file
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
fileStream::fileStream(const std::string& fileName, openMode mode)
{
    IMEBRA_FUNCTION_START();

    std::string strMode;
    switch(mode)
    {
    case openMode::read:
        strMode = "rb";
        break;
    case openMode::write:
        strMode = "wb";
        break;
    }

#if defined(IMEBRA_WINDOWS)

    errno_t errorCode = ::fopen_s(&m_openFile, fileName.c_str(), strMode.c_str());
     if (errorCode != 0)
     {
         m_openFile = 0;
     }
#else

    m_openFile = ::fopen(fileName.c_str(), strMode.c_str());
    int errorCode = errno;
#endif
    if(m_openFile == 0)
    {
        IMEBRA_THROW(StreamOpenError, "stream::openFile failure - error code: " << errorCode);
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Destructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
fileStream::~fileStream()
{
    ::fclose(m_openFile);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Open a file (ansi)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
fileStreamInput::fileStreamInput(const std::string& fileName): fileStream(fileName, openMode::read)
{
}

fileStreamInput::fileStreamInput(const std::wstring& fileName): fileStream(fileName, openMode::read)
{
}

fileStreamOutput::fileStreamOutput(const std::string& fileName): fileStream(fileName, openMode::write)
{
}

fileStreamOutput::fileStreamOutput(const std::wstring &fileName): fileStream(fileName, openMode::write)
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
void fileStreamOutput::write(size_t startPosition, const std::uint8_t* pBuffer, size_t bufferLength)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    ::fseek(m_openFile, (long)startPosition, SEEK_SET);
    if(ferror(m_openFile) != 0)
    {
        IMEBRA_THROW(StreamWriteError, "stream::seek failure");
    }

    if(::fwrite(pBuffer, 1, bufferLength, m_openFile) != bufferLength)
    {
        IMEBRA_THROW(StreamWriteError, "stream::write failure");
    }

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
size_t fileStreamInput::read(size_t startPosition, std::uint8_t* pBuffer, size_t bufferLength)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    ::fseek(m_openFile, (long)startPosition, SEEK_SET);
    if(ferror(m_openFile) != 0)
    {
        IMEBRA_THROW(StreamReadError, "stream::fseek failure");
    }

    size_t readBytes = (size_t)::fread(pBuffer, 1, bufferLength, m_openFile);
    if(ferror(m_openFile) != 0)
    {
        IMEBRA_THROW(StreamReadError, "stream::read failure");
    }
    return readBytes;

    IMEBRA_FUNCTION_END();
}


void fileStreamInput::terminate()
{
}

bool fileStreamInput::seekable() const
{
    return true;
}

size_t fileStreamInput::getSize() const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    ::fseek(m_openFile, 0, SEEK_END);
    if(ferror(m_openFile) != 0)
    {
        IMEBRA_THROW(StreamReadError, "stream::fseek failure");
    }

    long int position = ::ftell(m_openFile);
    if(position < 0)
    {
        IMEBRA_THROW(StreamReadError, "stream::ftell failure");
    }

    return (size_t)position;

    IMEBRA_FUNCTION_END();

}

} // namespace implementation

} // namespace imebra
