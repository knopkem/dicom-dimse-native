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
    \brief Implementation of the the class used to read the streams.

*/

#include "../include/imebra/streamReader.h"
#include "../include/imebra/baseStreamInput.h"
#include "../include/imebra/memory.h"
#include "../implementation/streamReaderImpl.h"
#include "../implementation/memoryImpl.h"

namespace imebra
{

StreamReader::StreamReader(const std::shared_ptr<implementation::streamReader>& pReader): m_pReader(pReader)
{
}

StreamReader::StreamReader(const BaseStreamInput& stream): m_pReader(std::make_shared<implementation::streamReader>(getBaseStreamInputImplementation(stream)))
{
}

StreamReader::StreamReader(const BaseStreamInput& stream, size_t virtualStart, size_t virtualLength): m_pReader(std::make_shared<implementation::streamReader>(getBaseStreamInputImplementation(stream), virtualStart, virtualLength))
{
}

StreamReader::StreamReader(const StreamReader& source): m_pReader(getStreamReaderImplementation(source))
{
}

const std::shared_ptr<implementation::streamReader>& getStreamReaderImplementation(const StreamReader& streamReader)
{
    return streamReader.m_pReader;
}

StreamReader StreamReader::getVirtualStream(size_t virtualStreamLength)
{
    IMEBRA_FUNCTION_START();

    return StreamReader(m_pReader->getReader(virtualStreamLength));

    IMEBRA_FUNCTION_END_LOG();
}

void StreamReader::read(char* destination, size_t destinationSize)
{
    IMEBRA_FUNCTION_START();

    m_pReader->read((std::uint8_t*)destination, destinationSize);

    IMEBRA_FUNCTION_END_LOG();
}

size_t StreamReader::readSome(char* destination, size_t destinationSize)
{
    IMEBRA_FUNCTION_START();

    return m_pReader->readSome((std::uint8_t*)destination, destinationSize);

    IMEBRA_FUNCTION_END_LOG();
}

Memory StreamReader::read(size_t readSize)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::memory> readMemory(std::make_shared<implementation::memory> (readSize));
    m_pReader->read(readMemory->data(), readSize);
    return Memory(readMemory);

    IMEBRA_FUNCTION_END_LOG();
}

Memory StreamReader::readSome(size_t readSize)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::memory> readMemory(std::make_shared<implementation::memory> (readSize));
    size_t actuallyRead = m_pReader->readSome(readMemory->data(), readSize);
    readMemory->resize(actuallyRead);
    return Memory(readMemory);

    IMEBRA_FUNCTION_END_LOG();
}

void StreamReader::terminate()
{
    IMEBRA_FUNCTION_START();

    m_pReader->terminate();

    IMEBRA_FUNCTION_END_LOG();
}

StreamReader::~StreamReader()
{
}

}
