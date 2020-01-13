/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file charsetConversion.cpp
    \brief Implementation of the charsetConversion class.

*/

#include "exceptionImpl.h"
#include "charsetConversionImpl.h"
#include "../include/imebra/exceptions.h"
#include <memory>
#include <sstream>

namespace imebra
{

charsetDictionary::charsetDictionary()
{
    IMEBRA_FUNCTION_START();

    registerCharset("ISO_IR 6", "", "ISO-IR-6", "US-ASCII", 20127, false);
    registerCharset("ISO_IR 100", "", "ISO-IR-100", "ISO-8859-1", 28591, false);
    registerCharset("ISO_IR 101", "", "ISO-IR-101", "ISO-8859-2", 28592, false);
    registerCharset("ISO_IR 109", "", "ISO-IR-109", "ISO-8859-3", 28593, false);
    registerCharset("ISO_IR 110", "", "ISO-IR-110", "ISO-8859-4", 28594, false);
    registerCharset("ISO_IR 144", "", "ISO-IR-144", "ISO-8859-5", 28595, false);
    registerCharset("ISO_IR 127", "", "ISO-IR-127", "ISO-8859-6", 28596, false);
    registerCharset("ISO_IR 126", "", "ISO-IR-126", "ISO-8859-7", 28597, false);
    registerCharset("ISO_IR 138", "", "ISO-IR-138", "ISO-8859-8", 28598, false);
    registerCharset("ISO_IR 148", "", "ISO-IR-148", "ISO-8859-9", 28599, false);
    registerCharset("ISO_IR 149", "", "ISO-IR-149", "EUC-KR", 949, false);
    registerCharset("ISO_IR 13",  "", "ISO-IR-13", "EUC_JP", 50930, false);
    registerCharset("ISO_IR 14",  "", "ISO-IR-14", "EUC-JP", 932, false );
    registerCharset("ISO_IR 166", "", "ISO-IR-166", "TIS-620", 874, false);
    registerCharset("ISO_IR 87",  "", "ISO-IR-87", "EUC-JP", 20932, false);
    registerCharset("ISO_IR 159", "", "ISO-IR-159", "JIS_X0212-1990", 20932, false);

    registerCharset("ISO 2022 IR 6",   "\x1b\x28\x42", "ISO-IR-6", "US-ASCII", 20127, false);
    registerCharset("ISO 2022 IR 100", "\x1b\x2d\x41", "ISO-IR-100", "ISO-8859-1", 28591, false);
    registerCharset("ISO 2022 IR 101", "\x1b\x2d\x42", "ISO-IR-101", "ISO-8859-2", 28592, false);
    registerCharset("ISO 2022 IR 109", "\x1b\x2d\x43", "ISO-IR-109", "ISO-8859-3", 28593, false);
    registerCharset("ISO 2022 IR 110", "\x1b\x2d\x44", "ISO-IR-110", "ISO-8859-4", 28594, false);
    registerCharset("ISO 2022 IR 144", "\x1b\x2d\x4c", "ISO-IR-144", "ISO-8859-5", 28595, false);
    registerCharset("ISO 2022 IR 127", "\x1b\x2d\x47", "ISO-IR-127", "ISO-8859-6", 28596, false);
    registerCharset("ISO 2022 IR 126", "\x1b\x2d\x46", "ISO-IR-126", "ISO-8859-7", 28597, false);
    registerCharset("ISO 2022 IR 138", "\x1b\x2d\x48", "ISO-IR-138", "ISO-8859-8", 28598, false);
    registerCharset("ISO 2022 IR 148", "\x1b\x2d\x4d", "ISO-IR-148", "ISO-8859-9", 28599, false);
    registerCharset("ISO 2022 IR 149", "\x1b\x24\x29\x43", "ISO-IR-149", "EUC-KR", 949, false);
    registerCharset("ISO 2022 IR 13",  "\x1b\x29\x49", "ISO-IR-13", "EUC_JP", 50930, false);
    registerCharset("ISO 2022 IR 14",  "\x1b\x28\x4a", "ISO-IR-14", "EUC-JP", 932, false);
    registerCharset("ISO 2022 IR 166", "\x1b\x2d\x54", "ISO-IR-166", "TIS-620", 874, false);
    registerCharset("ISO 2022 IR 87",  "\x1b\x24\x42", "ISO-IR-87", "EUC-JP", 20932, false);
    registerCharset("ISO 2022 IR 159", "\x1b\x24\x28\x44", "ISO-IR-159", "JIS_X0212-1990", 20932, false);
    registerCharset("ISO_IR 192", "", "UTF-8", "UTF-8", 65001, true);
    registerCharset("GB18030",    "", "GB18030", "GB18030", 54936, true);

    IMEBRA_FUNCTION_END();
}

const charsetInformation& charsetDictionary::getCharsetInformation(const std::string& dicomName) const
{
    IMEBRA_FUNCTION_START();

    std::string normalizedName(charsetConversionBase::normalizeIsoCharset((dicomName)));
    dictionary_t::const_iterator findInfo(m_dictionary.find(normalizedName));
    if(findInfo == m_dictionary.end())
    {
        IMEBRA_THROW(CharsetConversionNoTableError, "Charset table " << dicomName << " not found in the charset dictionary");
    }
    return findInfo->second;

    IMEBRA_FUNCTION_END();
}

const charsetDictionary::escapeSequences_t& charsetDictionary::getEscapeSequences() const
{
    return m_escapeSequences;
}

void charsetDictionary::registerCharset(const std::string& dicomName, const std::string& escapeSequence, const std::string& isoName, const std::string& javaName, const unsigned long windowsPage, const bool bZeroFlag)
{
    IMEBRA_FUNCTION_START();

    m_dictionary.insert(std::pair<std::string, charsetInformation>(
                            charsetConversionBase::normalizeIsoCharset(dicomName),
                            charsetInformation(dicomName, escapeSequence, isoName, javaName, windowsPage, bZeroFlag)));

    if(!escapeSequence.empty())
    {
        m_escapeSequences[escapeSequence] = dicomName;
    }

    IMEBRA_FUNCTION_END();
}

charsetInformation::charsetInformation(const std::string &dicomName, const std::string &escapeSequence, const std::string &isoRegistration, const std::string &javaRegistration, unsigned long codePage, bool bZeroFlag):
    m_dicomName(dicomName),
    m_escapeSequence(escapeSequence),
    m_isoRegistration(isoRegistration),
    m_javaRegistration(javaRegistration),
    m_codePage(codePage),
    m_bZeroFlag(bZeroFlag)
{

}

charsetInformation::charsetInformation(const charsetInformation &right):
    m_dicomName(right.m_dicomName),
    m_escapeSequence(right.m_escapeSequence),
    m_isoRegistration(right.m_isoRegistration),
    m_javaRegistration(right.m_javaRegistration),
    m_codePage(right.m_codePage),
    m_bZeroFlag(right.m_bZeroFlag)
{

}


//////////////////////////////////////////////////////////
//
// Destructor
//
///////////////////////////////////////////////////////////
charsetConversionBase::~charsetConversionBase()
{
}


const charsetDictionary& charsetConversionBase::getDictionary() const
{
    IMEBRA_FUNCTION_START();

    static charsetDictionary dictionary;
    return dictionary;

    IMEBRA_FUNCTION_END();
}


std::string charsetConversionBase::normalizeIsoCharset(const std::string &isoCharset)
{
    IMEBRA_FUNCTION_START();

    std::string normalizedIsoCharset;

    normalizedIsoCharset.reserve(isoCharset.size());

    for(size_t scanChars(0), endChars(isoCharset.size()); scanChars != endChars; ++scanChars)
    {
        char isoChar(isoCharset[scanChars]);

        if(isoChar >= 'a' && isoChar <= 'z')
        {
            normalizedIsoCharset.push_back((char)(isoChar - ('a' - 'A')));
            continue;
        }

        if( (isoChar >= 'A' && isoChar <= 'Z') || (isoChar >= '0' && isoChar <= '9'))
        {
            normalizedIsoCharset.push_back(isoChar);
        }
    }

    return normalizedIsoCharset;

    IMEBRA_FUNCTION_END();
}

} // namespace imebra

