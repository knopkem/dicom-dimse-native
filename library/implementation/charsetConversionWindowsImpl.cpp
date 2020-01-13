/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file charsetConversionWindows.cpp
    \brief Implementation of the charsetConversion class on Windows.

*/

#include "configurationImpl.h"

#if defined(IMEBRA_USE_WINDOWS_CHARSET)

#include "exceptionImpl.h"
#include "charsetConversionWindowsImpl.h"
#include <memory>

namespace imebra
{

///////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////
charsetConversionWindows::charsetConversionWindows(const std::string& dicomName)
{
    IMEBRA_FUNCTION_START();

    const charsetInformation& info(getDictionary().getCharsetInformation(dicomName));

    m_codePage = info.m_codePage;
    m_bZeroFlag = info.m_bZeroFlag;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Convert a string from unicode to multibyte
//
///////////////////////////////////////////////////////////
std::string charsetConversionWindows::fromUnicode(const std::wstring& unicodeString) const
{
    IMEBRA_FUNCTION_START();

	if(unicodeString.empty())
	{
		return std::string();
	}

	BOOL bUsedDefault = false;
	int requiredChars = ::WideCharToMultiByte(m_codePage, m_bZeroFlag ? 0 : WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK | WC_DEFAULTCHAR, unicodeString.c_str(), (int)(unicodeString.length()), 0, 0, 0, m_bZeroFlag ? 0 : &bUsedDefault);
    if(requiredChars <= 0)
    {
        return "";
    }
    std::string returnString((size_t)requiredChars, char(0));
    ::WideCharToMultiByte(m_codePage, m_bZeroFlag ? 0 : WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK | WC_DEFAULTCHAR, unicodeString.c_str(), (int)(unicodeString.length()), &(returnString[0]), requiredChars, 0, m_bZeroFlag ? 0 : &bUsedDefault);
	if(bUsedDefault)
	{
		return std::string();
	}
	return returnString;

	IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Convert a string from multibyte to unicode
//
///////////////////////////////////////////////////////////
std::wstring charsetConversionWindows::toUnicode(const std::string& asciiString) const
{
    IMEBRA_FUNCTION_START();

	if(asciiString.empty())
	{
		return std::wstring();
	}

	int requiredWChars = ::MultiByteToWideChar(m_codePage, 0, asciiString.c_str(), (int)(asciiString.length()), 0, 0);
    if(requiredWChars <= 0)
    {
        return L"";
    }
    std::wstring returnString((size_t)requiredWChars, char(0));
    ::MultiByteToWideChar(m_codePage, 0, asciiString.c_str(), (int)(asciiString.length()), &(returnString[0]), requiredWChars);
    return returnString;

	IMEBRA_FUNCTION_END();
}

} // namespace imebra

#endif
