/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file charsetConversionIconv.cpp
    \brief Implementation of the charsetConversion class using Iconv.

*/

#include "configurationImpl.h"

#if defined(IMEBRA_USE_ICONV)

#include "exceptionImpl.h"
#include "charsetConversionIconvImpl.h"
#include "../include/imebra/exceptions.h"

#include <memory>
#include <array>

namespace imebra
{

///////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////
charsetConversionIconv::charsetConversionIconv(const std::string& dicomName)
{
    IMEBRA_FUNCTION_START();

    const charsetInformation& info(getDictionary().getCharsetInformation(dicomName));

    std::string toCodeIgnore(info.m_isoRegistration);
    toCodeIgnore += "//IGNORE";

    // Check little endian/big endian
    ///////////////////////////////////////////////////////////
    std::uint16_t m_endianCheck=0x00ff;
    const char* utfCode;
    static_assert(sizeof(wchar_t) == 2 || sizeof(wchar_t) == 4, "Unsupported UTF char size");
    if(sizeof(wchar_t) == 2)
    {
        utfCode = (*((std::uint8_t*)&m_endianCheck) == 0xff) ? "UTF-16LE" : "UTF-16BE";
    }
    else
    {
        utfCode = (*((std::uint8_t*)&m_endianCheck) == 0xff) ? "UTF-32LE" : "UTF-32BE";
    }

    m_iconvToUnicode = iconv_open(utfCode, info.m_isoRegistration.c_str());
    m_iconvFromUnicode = iconv_open(toCodeIgnore.c_str(), utfCode);
    if(m_iconvToUnicode == (iconv_t)-1 || m_iconvFromUnicode == (iconv_t)-1)
    {
        IMEBRA_THROW(CharsetConversionNoSupportedTableError, "Table " << dicomName << " not supported by the system");
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Destructor
//
///////////////////////////////////////////////////////////
charsetConversionIconv::~charsetConversionIconv()
{
    iconv_close(m_iconvToUnicode);
    iconv_close(m_iconvFromUnicode);
}

///////////////////////////////////////////////////////////
//
// Convert a string from unicode to multibyte
//
///////////////////////////////////////////////////////////
std::string charsetConversionIconv::fromUnicode(const std::wstring& unicodeString) const
{
    IMEBRA_FUNCTION_START();

    if(unicodeString.empty())
    {
        return std::string();
    }

    return myIconv(m_iconvFromUnicode, (char*)unicodeString.c_str(), unicodeString.length() * sizeof(wchar_t));

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Convert a string from multibyte to unicode
//
///////////////////////////////////////////////////////////
std::wstring charsetConversionIconv::toUnicode(const std::string& asciiString) const
{
    IMEBRA_FUNCTION_START();

    if(asciiString.empty())
    {
        return std::wstring();
    }

    std::string convertedString(myIconv(m_iconvToUnicode, (char*)asciiString.c_str(), asciiString.length()));
    std::wstring returnString((wchar_t*)convertedString.c_str(), convertedString.size() / sizeof(wchar_t));

    return returnString;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// In Posix systems executes an iconv
//
///////////////////////////////////////////////////////////
#if defined(IMEBRA_WINDOWS)
std::string charsetConversionIconv::myIconv(iconv_t context, const char* inputString, size_t inputStringLengthBytes) const
#else
std::string charsetConversionIconv::myIconv(iconv_t context, char* inputString, size_t inputStringLengthBytes) const
#endif
{
    IMEBRA_FUNCTION_START();

    std::string finalString;

    // Reset the state
    ///////////////////////////////////////////////////////////
    iconv(context, 0, 0, 0, 0);

    // Buffer for the conversion
    ///////////////////////////////////////////////////////////
    //char conversionBuffer[1024];
    std::array<char, 1024> conversionBuffer;

    // Convert the string
    ///////////////////////////////////////////////////////////
    for(size_t iconvReturn = (size_t)-1; iconvReturn == (size_t)-1;)
    {
        // Executes the conversion
        ///////////////////////////////////////////////////////////
        size_t progressiveOutputBufferSize = conversionBuffer.size();
        char* progressiveOutputBuffer(conversionBuffer.data());
        iconvReturn = iconv(context, &inputString, &inputStringLengthBytes, &progressiveOutputBuffer, &progressiveOutputBufferSize);

        // Update buffer's size
        ///////////////////////////////////////////////////////////
        size_t outputLengthBytes = conversionBuffer.size() - progressiveOutputBufferSize;

        finalString.append(conversionBuffer.data(), outputLengthBytes);

        // Throw if an invalid sequence is found
        ///////////////////////////////////////////////////////////
        if(iconvReturn == (size_t)-1 && errno != E2BIG)
        {
            return std::string();
        }
    }

    return finalString;

    IMEBRA_FUNCTION_END();
}

} // namespace imebra

#endif
