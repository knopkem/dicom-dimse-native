/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerString.cpp
    \brief Implementation of the base class for the string handlers.

*/

#include <sstream>
#include <iomanip>

#include "exceptionImpl.h"
#include "dataHandlerStringImpl.h"
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
// dataHandlerString
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
// Constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
readingDataHandlerString::readingDataHandlerString(const memory &parseMemory, tagVR_t dataType, const char separator, const uint8_t paddingByte):
    readingDataHandler(dataType)
{
    IMEBRA_FUNCTION_START();

    std::string parseString(reinterpret_cast<const char*>(parseMemory.data()), parseMemory.size());
    while(!parseString.empty() && (parseString.back() == static_cast<const char>(paddingByte) || parseString.back() == 0))
    {
        parseString.pop_back();
    }

    if(separator == 0)
    {
        m_strings.push_back(parseString);
        return;
    }

    for(size_t firstPosition(0); ;)
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


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Destructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
readingDataHandler::~readingDataHandler()
{
}

// Get the data element as a signed long
///////////////////////////////////////////////////////////
std::int32_t readingDataHandlerString::getSignedLong(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    std::istringstream conversion(getString(index));
    std::int32_t value;
    if(!(conversion >> value))
    {
        IMEBRA_THROW(DataHandlerConversionError, "Cannot convert " << m_strings.at(index) << " to a number");
    }
    return value;

    IMEBRA_FUNCTION_END();
}

// Get the data element as an unsigned long
///////////////////////////////////////////////////////////
std::uint32_t readingDataHandlerString::getUnsignedLong(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    std::istringstream conversion(getString(index));
    std::uint32_t value;
    if(!(conversion >> value))
    {
        IMEBRA_THROW(DataHandlerConversionError, "Cannot convert " << m_strings.at(index) << " to a number");
    }
    return value;

    IMEBRA_FUNCTION_END();
}

// Get the data element as a double
///////////////////////////////////////////////////////////
double readingDataHandlerString::getDouble(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    std::istringstream conversion(getString(index));
    double value;
    if(!(conversion >> value))
    {
        IMEBRA_THROW(DataHandlerConversionError, "Cannot convert " << m_strings.at(index) << " to a number");
    }
    return value;

    IMEBRA_FUNCTION_END();
}

// Get the data element as a string
///////////////////////////////////////////////////////////
std::string readingDataHandlerString::getString(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    if(index >= getSize())
    {
        IMEBRA_THROW(MissingItemError, "Missing item " << index);
    }

    return m_strings.at(index);

    IMEBRA_FUNCTION_END();
}

// Get the data element as an unicode string
///////////////////////////////////////////////////////////
std::wstring readingDataHandlerString::getUnicodeString(const size_t index) const
{
    IMEBRA_FUNCTION_START();

    charsetsList_t charsets;
    charsets.push_back("ISO 2022 IR 6");
    return dicomConversion::convertToUnicode(getString(index), charsets);

    IMEBRA_FUNCTION_END();
}

// Retrieve the data element as a string
///////////////////////////////////////////////////////////
size_t readingDataHandlerString::getSize() const
{
    return m_strings.size();
}

writingDataHandlerString::writingDataHandlerString(const std::shared_ptr<buffer> &pBuffer, tagVR_t dataType, const char separator, const size_t unitSize, const size_t maxSize):
    writingDataHandler(pBuffer, dataType), m_separator(separator), m_unitSize(unitSize), m_maxSize(maxSize)
{
}

writingDataHandlerString::~writingDataHandlerString()
{
    std::string completeString;
    for(size_t stringsIterator(0); stringsIterator != m_strings.size(); ++stringsIterator)
    {
        if(stringsIterator != 0)
        {
            completeString += m_separator;
        }
        completeString += m_strings.at(stringsIterator);
    }

    std::shared_ptr<memory> commitMemory = std::make_shared<memory>(completeString.size());
    commitMemory->assign(reinterpret_cast<const std::uint8_t*>(completeString.data()), completeString.size());

    m_buffer->commit(commitMemory);
}

// Set the data element as a signed long
///////////////////////////////////////////////////////////
void writingDataHandlerString::setSignedLong(const size_t index, const std::int32_t value)
{
    IMEBRA_FUNCTION_START();

    std::ostringstream conversion;
    conversion << value;
    setString(index, conversion.str());

    IMEBRA_FUNCTION_END();
}

// Set the data element as an unsigned long
///////////////////////////////////////////////////////////
void writingDataHandlerString::setUnsignedLong(const size_t index, const std::uint32_t value)
{
    IMEBRA_FUNCTION_START();

    std::ostringstream conversion;
    conversion << value;
    setString(index, conversion.str());

    IMEBRA_FUNCTION_END();
}

// Set the data element as a double
///////////////////////////////////////////////////////////
void writingDataHandlerString::setDouble(const size_t index, const double value)
{
    IMEBRA_FUNCTION_START();

    std::ostringstream conversion;
    conversion << value;
    setString(index, conversion.str());

    IMEBRA_FUNCTION_END();
}

// Set the buffer's size, in data elements
///////////////////////////////////////////////////////////
void writingDataHandlerString::setSize(const size_t elementsNumber)
{
    IMEBRA_FUNCTION_START();

    m_strings.resize(elementsNumber);

    IMEBRA_FUNCTION_END();
}

size_t writingDataHandlerString::getSize() const
{
    return m_strings.size();
}

void writingDataHandlerString::setString(const size_t index, const std::string& value)
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

    IMEBRA_FUNCTION_END();
}

void writingDataHandlerString::setUnicodeString(const size_t index, const std::wstring& value)
{
    IMEBRA_FUNCTION_START();

    charsetsList_t charsets;
    charsets.push_back("ISO_IR 6");
    setString(index, dicomConversion::convertFromUnicode(value, charsets));

    IMEBRA_FUNCTION_END();
}

void writingDataHandlerString::validate() const
{
    IMEBRA_FUNCTION_START();

    validateStringContainer(m_strings, m_maxSize, m_unitSize, m_separator != 0);

    IMEBRA_FUNCTION_END();
}


} // namespace handlers

} // namespace implementation

} // namespace imebra
