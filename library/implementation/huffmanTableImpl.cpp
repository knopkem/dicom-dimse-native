/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file huffmanTable.cpp
    \brief Implementation of the huffman codec.

*/

#include "exceptionImpl.h"
#include "huffmanTableImpl.h"
#include "streamReaderImpl.h"
#include "streamWriterImpl.h"
#include "jpegCodecBaseImpl.h"
#include "../include/imebra/exceptions.h"

#include <list>
#include <string.h>
#include <stdexcept>

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
// huffmanTable
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
huffmanTable::huffmanTable(std::uint32_t maxValueLength)
{
    IMEBRA_FUNCTION_START();

    m_numValues = (std::uint32_t(1)<<(maxValueLength)) + 1;
    reset();

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Reset the class' attributes
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void huffmanTable::reset()
{
    IMEBRA_FUNCTION_START();

    m_valuesFreq.clear();
    m_valuesFreq.resize(m_numValues);

    m_orderedValues.resize(m_numValues);
    ::memset(&(m_orderedValues[0]), 0, m_numValues*sizeof(m_orderedValues[0]));

    m_valuesToHuffman.resize(m_numValues);
    ::memset(&(m_valuesToHuffman[0]), 0, m_numValues*sizeof(m_valuesToHuffman[0]));

    m_valuesToHuffmanLength.resize(m_numValues);
    ::memset(&(m_valuesToHuffmanLength[0]), 0, m_numValues*sizeof(m_valuesToHuffmanLength[0]));

    m_valuesPerLength.fill(0);
    m_firstValidLength = 0;
    m_firstMinValue = 0xffffffff;
    m_firstMaxValue = 0xffffffff;
    m_firstValuesPerLength = 0;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Increase the frequency of one value
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void huffmanTable::incValueFreq(const std::uint32_t value)
{
    ++(m_valuesFreq[value].m_freq);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Remove the last huffman code from the lengths table
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void huffmanTable::removeLastCode()
{
    IMEBRA_FUNCTION_START();

    // Find the number of codes
    ///////////////////////////////////////////////////////////
    std::uint32_t codes = 0;
    std::uint32_t lastLength = 0;
    for(std::uint32_t scanLengths = 0; scanLengths < m_valuesPerLength.size(); ++scanLengths)
    {
        if(m_valuesPerLength[scanLengths] == 0)
        {
            continue;
        }
        codes += m_valuesPerLength[scanLengths];
        lastLength = scanLengths;
    }
    if(lastLength == 0)
    {
        return;
    }
    m_valuesPerLength[lastLength]--;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the number of values for a specific length
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void huffmanTable::setValuesPerLength(std::uint32_t length, std::uint32_t numValues)
{
    IMEBRA_FUNCTION_START();

    if(length >= m_valuesPerLength.size())
    {
        IMEBRA_THROW(CodecCorruptedFileError, "Huffman code length too big");
    }
    m_valuesPerLength[length] = numValues;

    IMEBRA_FUNCTION_END();
}

void huffmanTable::addOrderedValue(size_t index, std::uint32_t value)
{
    IMEBRA_FUNCTION_START();

    if(index >= m_orderedValues.size())
    {
        IMEBRA_THROW(CodecCorruptedFileError, "Too many values in the huffman table");
    }
    m_orderedValues[index] = value;

    IMEBRA_FUNCTION_END();
}

std::uint32_t huffmanTable::getValuesPerLength(std::uint32_t length)
{
    IMEBRA_FUNCTION_START();

    return m_valuesPerLength[length];

    IMEBRA_FUNCTION_END();
}

std::uint32_t huffmanTable::getOrderedValue(size_t index)
{
    return m_orderedValues[index];
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Calculate the codes length
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void huffmanTable::calcHuffmanCodesLength(const std::uint32_t maxCodeLength)
{
    IMEBRA_FUNCTION_START();

    // Order the values by their frequency
    typedef std::map<huffmanTable::freqValue, bool, huffmanTable::freqValueCompare> tFreqOrderedMap;
    tFreqOrderedMap freqOrderedValues;

    for(size_t scanValues = 0; scanValues < m_valuesFreq.size(); ++scanValues)
    {
        if(m_valuesFreq[scanValues].m_freq != 0)
        {
            huffmanTable::freqValue key(m_valuesFreq[scanValues].m_freq, (std::uint32_t)scanValues);
            freqOrderedValues[key] = true;
        }
    }

    while(freqOrderedValues.size() > 1)
    {
        huffmanTable::freqValue key0(freqOrderedValues.begin()->first);
        freqOrderedValues.erase(freqOrderedValues.begin());
        huffmanTable::freqValue key1(freqOrderedValues.begin()->first);
        freqOrderedValues.erase(freqOrderedValues.begin());

        key0.m_freq += key1.m_freq;
        m_valuesFreq[key0.m_value].m_freq = key0.m_freq;
        m_valuesFreq[key1.m_value].m_freq = 0;
        m_valuesFreq[key0.m_value].m_codeLength++;

        freqOrderedValues[key0] = true;

        std::uint32_t chainedValue;
        for(chainedValue = key0.m_value; m_valuesFreq[chainedValue].m_nextCode != std::numeric_limits<std::uint32_t>::max(); /* empty */)
        {
            chainedValue = (std::uint32_t)m_valuesFreq[chainedValue].m_nextCode;
            m_valuesFreq[chainedValue].m_codeLength++;
        }
        m_valuesFreq[chainedValue].m_nextCode = key1.m_value;
        while(m_valuesFreq[chainedValue].m_nextCode != std::numeric_limits<std::uint32_t>::max())
        {
            chainedValue = (std::uint32_t)m_valuesFreq[chainedValue].m_nextCode;
            m_valuesFreq[chainedValue].m_codeLength++;
        }
    }

    typedef std::map<huffmanTable::lengthValue, bool, huffmanTable::lengthValueCompare> tLengthOrderedMap;
    tLengthOrderedMap lengthOrderedValues ;
    for(size_t findValuesPerLength = 0; findValuesPerLength < m_valuesFreq.size(); ++findValuesPerLength)
    {
        if(m_valuesFreq[findValuesPerLength].m_codeLength != 0)
        {
            huffmanTable::lengthValue key(m_valuesFreq[findValuesPerLength].m_codeLength, (std::uint32_t)findValuesPerLength);
            lengthOrderedValues[key] = true;
            m_valuesPerLength[m_valuesFreq[findValuesPerLength].m_codeLength]++;
        }
    }

    size_t insertPosition(0);
    for(tLengthOrderedMap::iterator scanLengths = lengthOrderedValues.begin(); scanLengths != lengthOrderedValues.end(); ++scanLengths)
    {
        m_orderedValues[insertPosition++] = scanLengths->first.m_value;
    }

    // Reduce the size of the codes' lengths
    for(size_t reduceLengths = m_valuesPerLength.size() - 1; reduceLengths>maxCodeLength; --reduceLengths)
    {
        while(m_valuesPerLength[reduceLengths] != 0)
        {
            size_t reduceLengths1;
            for(reduceLengths1 = reduceLengths - 2; reduceLengths1 < m_valuesPerLength.size() && m_valuesPerLength[reduceLengths1] == 0; --reduceLengths1)
            {}

            if(reduceLengths1 >= m_valuesPerLength.size())
            {
                break;
            }
            m_valuesPerLength[reduceLengths]-=2;
            m_valuesPerLength[reduceLengths-1]++;
            m_valuesPerLength[reduceLengths1+1]+=2;
            m_valuesPerLength[reduceLengths1]--;
        }
    }

    // Find the first available length
    for(m_firstValidLength = 1; m_firstValidLength != m_valuesPerLength.size(); ++m_firstValidLength)
    {
        if(m_valuesPerLength[m_firstValidLength] != 0)
        {
            break;
        }
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Calculate the huffman codes from the codes length
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void huffmanTable::calcHuffmanTables()
{
    IMEBRA_FUNCTION_START();

    std::uint32_t huffmanCode = 0;

    std::uint32_t valueIndex = 0;

    ::memset(m_minValuePerLength, 0xff, sizeof(m_minValuePerLength));
    ::memset(m_maxValuePerLength, 0xff, sizeof(m_maxValuePerLength));
    m_firstValuesPerLength = 0;
    m_firstMinValue = 0xffffffff;
    m_firstMaxValue = 0xffffffff;
    m_firstValidLength = 0;
    for(size_t codeLength = 1; codeLength != m_valuesPerLength.size(); ++codeLength)
    {
        if(m_valuesPerLength[codeLength] != 0 && m_firstValidLength == 0)
        {
            m_firstValidLength = codeLength;
        }
        for(std::uint32_t generateCodes = 0; generateCodes<m_valuesPerLength[codeLength]; ++generateCodes)
        {
            if(generateCodes == 0)
            {
                m_minValuePerLength[codeLength]=huffmanCode;
            }
            m_maxValuePerLength[codeLength]=huffmanCode;
            m_valuesToHuffman[m_orderedValues[valueIndex]]=huffmanCode;
            m_valuesToHuffmanLength[m_orderedValues[valueIndex]] = codeLength;
            ++valueIndex;
            ++huffmanCode;
        }

        huffmanCode<<=1;

    }

    m_firstMinValue = m_minValuePerLength[m_firstValidLength];
    m_firstMaxValue = m_maxValuePerLength[m_firstValidLength];
    m_firstValuesPerLength = m_valuesPerLength[m_firstValidLength];

    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read an Huffman code
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t huffmanTable::readHuffmanCode(codecs::jpegStreamReader& stream)
{
    IMEBRA_FUNCTION_START();

    // Read initial number of bits
    std::uint32_t readBuffer(stream.readBits(m_firstValidLength));

    // Validate the current Huffman code. If it's OK, then
    //  return the ordered value
    ///////////////////////////////////////////////////////////
    if(readBuffer <= m_firstMaxValue)
    {
        return m_orderedValues[readBuffer - m_firstMinValue];
    }

    std::uint32_t orderedValue(m_firstValuesPerLength);

    // Scan all the codes sizes
    ///////////////////////////////////////////////////////////
    for(size_t scanSize(m_firstValidLength + 1), missingBits(0); scanSize != m_valuesPerLength.size(); ++scanSize)
    {
        ++missingBits;

        // If the active length is empty, then continue the loop
        ///////////////////////////////////////////////////////////
        if(m_valuesPerLength[scanSize] == 0)
        {
            continue;
        }

        readBuffer <<= missingBits;
        if(missingBits == 1)
        {
            readBuffer |= stream.readBit();
        }
        else
        {
            readBuffer |= stream.readBits(missingBits);
        }

        // Validate the current Huffman code. If it's OK, then
        //  return the ordered value
        ///////////////////////////////////////////////////////////
        if(readBuffer<=m_maxValuePerLength[scanSize])
        {
            return m_orderedValues[orderedValue + readBuffer - m_minValuePerLength[scanSize]];
        }

        orderedValue += m_valuesPerLength[scanSize];

        // Reset the number of bits to read in one go
        ///////////////////////////////////////////////////////////
        missingBits = 0;

    }

    IMEBRA_THROW(CodecCorruptedFileError, "Invalid huffman code found while reading from a stream");

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write an Huffman code
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void huffmanTable::writeHuffmanCode(const std::uint32_t code, streamWriter* pStream)
{
    IMEBRA_FUNCTION_START();

    if(m_valuesToHuffmanLength[code] == 0)
    {
        IMEBRA_THROW(std::logic_error, "Trying to write an invalid huffman code");
    }
    pStream->writeBits(m_valuesToHuffman[code], m_valuesToHuffmanLength[code]);

    IMEBRA_FUNCTION_END();
}

} // namespace implementation

} // namespace imebra

