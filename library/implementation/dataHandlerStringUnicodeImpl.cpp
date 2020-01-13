/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerStringUnicode.cpp
    \brief Implementation of the base class used by the string handlers that need
            to handle several charsets.

*/

#include "exceptionImpl.h"
#include "dataHandlerStringUnicodeImpl.h"
#include "memoryImpl.h"
#include "bufferImpl.h"

namespace imebra
{

namespace implementation
{

namespace handlers
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// dataHandlerStringUnicode
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

readingDataHandlerStringUnicode::readingDataHandlerStringUnicode(const memory& parseMemory, const std::shared_ptr<const charsetsList_t>& pCharsets, tagVR_t dataType, const wchar_t separator, const std::uint8_t paddingByte):
    readingDataHandler(dataType)
{
    IMEBRA_FUNCTION_START();

    std::string asciiString((const char*)parseMemory.data(), parseMemory.size());
    std::wstring parseString(dicomConversion::convertToUnicode(asciiString, *pCharsets));

    while(!parseString.empty() && parseString.back() == (wchar_t)paddingByte)
    {
        parseString.pop_back();
    }

    if(separator == 0)
    {
        m_strings.push_back(parseString);
        return;
    }

    for(size_t firstPosition(0); ; )
    {
        size_t nextPosition = parseString.find(separator, firstPosition);
        if(nextPosition == std::string::npos)
        {
            m_strings.push_back(parseString.substr(firstPosition));
            return;
        }
        m_strings.push_back(parseString.substr(firstPosition, nextPosition - firstPosition));
        firstPosition = ++nextPosition;
    }

    IMEBRA_FUNCTION_END();

}

// Get the data element as a signed long
///////////////////////////////////////////////////////////
std::int32_t readingDataHandlerStringUnicode::getSignedLong(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    std::wistringstream conversion(getUnicodeString(index));
    std::int32_t value;
    if(!(conversion >> value))
    {
        IMEBRA_THROW(DataHandlerConversionError, "The string is not a number");
    }
    return value;

    IMEBRA_FUNCTION_END();
}

// Get the data element as an unsigned long
///////////////////////////////////////////////////////////
std::uint32_t readingDataHandlerStringUnicode::getUnsignedLong(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    std::wistringstream conversion(getUnicodeString(index));
    std::uint32_t value;
    if(!(conversion >> value))
    {
        IMEBRA_THROW(DataHandlerConversionError, "The string is not a number");
    }
    return value;

    IMEBRA_FUNCTION_END();
}

// Get the data element as a double
///////////////////////////////////////////////////////////
double readingDataHandlerStringUnicode::getDouble(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    std::wistringstream conversion(getUnicodeString(index));
    double value;
    if(!(conversion >> value))
    {
        IMEBRA_THROW(DataHandlerConversionError, "The string is not a number");
    }
    return value;

    IMEBRA_FUNCTION_END();
}

// Get the data element as a string
///////////////////////////////////////////////////////////
std::string readingDataHandlerStringUnicode::getString(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    charsetsList_t charsets;
    charsets.push_back("ISO_IR 192");
    return dicomConversion::convertFromUnicode(getUnicodeString(index), charsets);

    IMEBRA_FUNCTION_END();
}

// Get the data element as an unicode string
///////////////////////////////////////////////////////////
std::wstring readingDataHandlerStringUnicode::getUnicodeString(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    if(index >= getSize())
    {
        IMEBRA_THROW(MissingItemError, "Missing item " << index);
    }
    return m_strings.at(index);

    IMEBRA_FUNCTION_END();
}

// Retrieve the data element as a string
///////////////////////////////////////////////////////////
size_t readingDataHandlerStringUnicode::getSize() const
{
    return m_strings.size();
}

writingDataHandlerStringUnicode::writingDataHandlerStringUnicode(const std::shared_ptr<buffer> &pBuffer, const std::shared_ptr<const charsetsList_t>& pCharsets, tagVR_t dataType, const wchar_t separator, const size_t unitSize, const size_t maxSize):
    writingDataHandler(pBuffer, dataType),
    m_commitMemory(std::make_shared<memory>()),
    m_pCharsets(pCharsets), m_separator(separator), m_unitSize(unitSize), m_maxSize(maxSize)
{
}

writingDataHandlerStringUnicode::~writingDataHandlerStringUnicode()
{
    m_buffer->commit(m_commitMemory);
}

// Set the data element as a signed long
///////////////////////////////////////////////////////////
void writingDataHandlerStringUnicode::setSignedLong(const size_t index, const std::int32_t value)
{
    IMEBRA_FUNCTION_START();

    std::wostringstream conversion;
    conversion << value;
    setUnicodeString(index, conversion.str());

    IMEBRA_FUNCTION_END();
}

// Set the data element as an unsigned long
///////////////////////////////////////////////////////////
void writingDataHandlerStringUnicode::setUnsignedLong(const size_t index, const std::uint32_t value)
{
    IMEBRA_FUNCTION_START();

    std::wostringstream conversion;
    conversion << value;
    setUnicodeString(index, conversion.str());

    IMEBRA_FUNCTION_END();
}

// Set the data element as a double
///////////////////////////////////////////////////////////
void writingDataHandlerStringUnicode::setDouble(const size_t index, const double value)
{
    IMEBRA_FUNCTION_START();

    std::wostringstream conversion;
    conversion << value;
    setUnicodeString(index, conversion.str());

    IMEBRA_FUNCTION_END();
}

// Set the buffer's size, in data elements
///////////////////////////////////////////////////////////
void writingDataHandlerStringUnicode::setSize(const size_t elementsNumber)
{
    IMEBRA_FUNCTION_START();

    m_strings.resize(elementsNumber);

    buildCommitMemory();

    IMEBRA_FUNCTION_END();
}

size_t writingDataHandlerStringUnicode::getSize() const
{
    return m_strings.size();
}

void writingDataHandlerStringUnicode::setString(const size_t index, const std::string& value)
{
    IMEBRA_FUNCTION_START();

    charsetsList_t charsets;
    charsets.push_back("ISO_IR 192");
    setUnicodeString(index, dicomConversion::convertToUnicode(value, charsets));

    IMEBRA_FUNCTION_END();
}

void writingDataHandlerStringUnicode::setUnicodeString(const size_t index, const std::wstring& value)
{
    IMEBRA_FUNCTION_START();

    if(m_separator == 0 && index != 0)
    {
        IMEBRA_THROW(DataHandlerInvalidDataError, "Cannot insert more than one item in this string tag");
    }
    if(index >= getSize())
    {
        setSize(index + 1);
    }
    m_strings[index] = value;

    validate();

    buildCommitMemory();

    IMEBRA_FUNCTION_END();
}

void writingDataHandlerStringUnicode::validate() const
{
    IMEBRA_FUNCTION_START();

    validateStringContainer(m_strings, m_maxSize, m_unitSize, m_separator != 0);

    IMEBRA_FUNCTION_END();
}

void writingDataHandlerStringUnicode::buildCommitMemory()
{
    IMEBRA_FUNCTION_START();

    // Build the raw bytes
    ///////////////////////////////////////////////////////////
    std::wstring completeString;
    for(size_t stringsIterator(0); stringsIterator != m_strings.size(); ++stringsIterator)
    {
        if(stringsIterator != 0)
        {
            completeString += m_separator;
        }
        completeString += m_strings.at(stringsIterator);
    }

    std::string asciiString = dicomConversion::convertFromUnicode(completeString, *m_pCharsets);

    m_commitMemory = std::make_shared<memory>(asciiString.size());
    m_commitMemory->assign((const std::uint8_t*)asciiString.data(), asciiString.size());

    IMEBRA_FUNCTION_END();
}




} // namespace handlers

} // namespace implementation

} // namespace imebra
