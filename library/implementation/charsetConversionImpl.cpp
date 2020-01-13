/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#include "charsetConversionImpl.h"
#include "exceptionImpl.h"
#include "../include/imebra/exceptions.h"
#include <sstream>

namespace imebra
{

namespace implementation
{

std::string dicomConversion::convertFromUnicode(const std::wstring& unicodeString, const charsetsList_t& charsets)
{
    IMEBRA_FUNCTION_START();

    if(unicodeString.empty())
    {
        return "";
    }

    // Check for the dicom charset's name
    ///////////////////////////////////////////////////////////
    if(charsets.empty())
    {
        std::unique_ptr<defaultCharsetConversion> localCharsetConversion(new defaultCharsetConversion("ISO_IR 6"));
        std::string returnString =  localCharsetConversion->fromUnicode(unicodeString);
        if(returnString.empty())
        {
            IMEBRA_THROW(CharsetConversionCannotConvert, "Cannot convert from unicode using only the charset 'ISO_IR 6'");
        }
        return returnString;
    }

    // Setup the conversion objects
    ///////////////////////////////////////////////////////////
    std::unique_ptr<defaultCharsetConversion> localCharsetConversion(new defaultCharsetConversion(charsets.front()));

    // Returned string
    ///////////////////////////////////////////////////////////
    std::string rawString;
    rawString.reserve(unicodeString.size());

    // Convert all the chars. Each char is tested with the
    //  active charset first, then with other charsets if
    //  the active one doesn't work
    ///////////////////////////////////////////////////////////
    for(size_t scanString = 0; scanString != unicodeString.size(); ++scanString)
    {
        // Get the UNICODE char. On windows & Android the code may
        //  be spread across 2 16 bit wide codes.
        ///////////////////////////////////////////////////////////
        std::wstring code(1u, unicodeString[scanString]);

        // Check UTF-16 extension (Windows & Android)
        ///////////////////////////////////////////////////////////
        if(sizeof(wchar_t) == 2u)
        {
            if(code[0] >= 0xd800 && code[0] <=0xdfff && scanString < (unicodeString.size() - 1))
            {
                code += unicodeString[++scanString];
            }
        }

        // Check composed chars extension (diacritical marks)
        ///////////////////////////////////////////////////////////
        while(scanString < (unicodeString.size() - 1) && unicodeString[scanString + 1] >= 0x0300 && unicodeString[scanString + 1] <= 0x036f)
        {
            code += unicodeString[++scanString];
        }

        // Remember the return string size so we can check if we
        //  added something to it
        ///////////////////////////////////////////////////////////
        size_t currentRawSize(rawString.size());
        rawString += localCharsetConversion->fromUnicode(code);
        if(rawString.size() != currentRawSize)
        {
            // The conversion succeeded: continue with the next char
            ///////////////////////////////////////////////////////////
            continue;
        }

        std::string convertedSequence;
        for(const std::string& dicomCharset: charsets)
        {
            try
            {
                std::unique_ptr<defaultCharsetConversion> testEscapeSequence(new defaultCharsetConversion(dicomCharset));
                std::string convertedChar(testEscapeSequence->fromUnicode(code));
                if(!convertedChar.empty())
                {
                    convertedSequence = testEscapeSequence->getDictionary().getCharsetInformation(dicomCharset).m_escapeSequence;
                    convertedSequence += convertedChar;
                    localCharsetConversion.reset(testEscapeSequence.release());
                    break;
                }
            }
            catch(CharsetConversionNoSupportedTableError)
            {
                continue;
            }
        }
        if(convertedSequence.empty())
        {
            std::ostringstream charsetsList;
            for(const std::string& charset: charsets)
            {
                charsetsList << " '" << charset << "'";
            }
            IMEBRA_THROW(CharsetConversionCannotConvert, "Cannot convert from unicode using the following charsets:" << charsetsList.str());
        }
        rawString += convertedSequence;
    }

    return rawString;

    IMEBRA_FUNCTION_END();

}

std::wstring dicomConversion::convertToUnicode(const std::string& value, const charsetsList_t& charsets)
{
    IMEBRA_FUNCTION_START();

    if(value.empty())
    {
        return L"";
    }

    // Should we take care of the escape sequences...?
    ///////////////////////////////////////////////////////////
    if(charsets.empty())
    {
        std::unique_ptr<defaultCharsetConversion> localCharsetConversion(new defaultCharsetConversion("ISO_IR 6"));
        return localCharsetConversion->toUnicode(value);
    }

    // Initialize the conversion engine with the default
    //  charset
    ///////////////////////////////////////////////////////////
    std::unique_ptr<defaultCharsetConversion> localCharsetConversion(new defaultCharsetConversion(charsets.front()));

    // Only one charset is present: we don't need to check
    //  the escape sequences
    ///////////////////////////////////////////////////////////
    if(charsets.size() == 1)
    {
        return localCharsetConversion->toUnicode(value);
    }

    // Here we store the value to be returned
    ///////////////////////////////////////////////////////////
    std::wstring returnString;
    returnString.reserve(value.size());

    // Get the escape sequences from the unicode conversion
    //  engine
    ///////////////////////////////////////////////////////////
    const charsetDictionary::escapeSequences_t& escapeSequences(localCharsetConversion->getDictionary().getEscapeSequences());

    // Position and properties of the next escape sequence
    ///////////////////////////////////////////////////////////
    size_t escapePosition = std::string::npos;
    std::string escapeString;
    std::string isoTable;

    // Scan all the string and look for valid escape sequences.
    // The partial strings are converted using the dicom
    //  charset specified by the escape sequences.
    ///////////////////////////////////////////////////////////
    for(size_t scanString = 0; scanString < value.size(); /* empty */)
    {
        // Find the position of the next escape sequence
        ///////////////////////////////////////////////////////////
        if(escapePosition == std::string::npos)
        {
            escapePosition = value.size();
            for(charsetDictionary::escapeSequences_t::const_iterator scanEscapes(escapeSequences.begin()), endEscapes(escapeSequences.end());
                scanEscapes != endEscapes;
                ++scanEscapes)
            {
                size_t findEscape = value.find(scanEscapes->first, scanString);
                if(findEscape != std::string::npos && findEscape < escapePosition)
                {
                    escapePosition = findEscape;
                    escapeString = scanEscapes->first;
                    isoTable = scanEscapes->second;
                }
            }
        }

        // No more escape sequences. Just convert everything
        ///////////////////////////////////////////////////////////
        if(escapePosition == value.size())
        {
            std::wstring conversion = localCharsetConversion->toUnicode(value.substr(scanString));
            if(conversion.empty())
            {
                IMEBRA_THROW(CharsetConversionCannotConvert, "Cannot convert to unicode");
            }
            return returnString + conversion;
        }

        // The escape sequence can wait, now we are still in the
        //  already activated charset
        ///////////////////////////////////////////////////////////
        if(escapePosition > scanString)
        {
            std::wstring conversion = localCharsetConversion->toUnicode(value.substr(scanString, escapePosition - scanString));
            if(conversion.empty())
            {
                IMEBRA_THROW(CharsetConversionCannotConvert, "Cannot convert to unicode");
            }
            returnString += conversion;
            scanString = escapePosition;
            continue;
        }

        // Move the char pointer to the next char that has to be
        //  analyzed
        ///////////////////////////////////////////////////////////
        scanString = escapePosition + escapeString.length();
        escapePosition = std::string::npos;

        // An iso table is coupled to the found escape sequence.
        ///////////////////////////////////////////////////////////
        localCharsetConversion.reset(new defaultCharsetConversion(isoTable));
    }

    return returnString;

    IMEBRA_FUNCTION_END();

}

}

}


