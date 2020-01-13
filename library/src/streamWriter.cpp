/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file streamWriter.cpp
    \brief Implementation of the the class used to write the streams.

*/

#include "../include/imebra/streamWriter.h"
#include "../include/imebra/baseStreamOutput.h"
#include "../include/imebra/memory.h"
#include "../implementation/streamWriterImpl.h"
#include "../implementation/memoryImpl.h"
#include "../implementation/exceptionImpl.h"

namespace imebra
{

StreamWriter::StreamWriter(const std::shared_ptr<implementation::streamWriter>& pWriter): m_pWriter(pWriter)
{
}

StreamWriter::StreamWriter(const BaseStreamOutput& stream): m_pWriter(std::make_shared<implementation::streamWriter>(getBaseStreamOutputImplementation(stream)))
{
}

StreamWriter::StreamWriter(const BaseStreamOutput& stream, size_t virtualStart, size_t virtualLength): m_pWriter(std::make_shared<implementation::streamWriter>(getBaseStreamOutputImplementation(stream), virtualStart, virtualLength))
{
}

StreamWriter::StreamWriter(const StreamWriter& source): m_pWriter(getStreamWriterImplementation(source))
{
}

void StreamWriter::write(const char* data, size_t dataSize)
{
    IMEBRA_FUNCTION_START();

    m_pWriter->write((const std::uint8_t*)data, dataSize);

    IMEBRA_FUNCTION_END_LOG();
}

void StreamWriter::write(const Memory& memory)
{
    IMEBRA_FUNCTION_START();

    const std::uint8_t* data(getMemoryImplementation(memory)->data());
    const size_t dataSize(getMemoryImplementation(memory)->size());
    m_pWriter->write(data, dataSize);

    IMEBRA_FUNCTION_END_LOG();
}

void StreamWriter::flush()
{
    IMEBRA_FUNCTION_START();

    m_pWriter->flushDataBuffer();

    IMEBRA_FUNCTION_END_LOG();
}

const std::shared_ptr<implementation::streamWriter>& getStreamWriterImplementation(const StreamWriter& streamWriter)
{
    return streamWriter.m_pWriter;
}

StreamWriter::~StreamWriter()
{
}

}
