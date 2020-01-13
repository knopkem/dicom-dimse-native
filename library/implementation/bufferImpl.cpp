/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file buffer.cpp
    \brief Implementation of the buffer class.

*/

#include "exceptionImpl.h"
#include "streamReaderImpl.h"
#include "streamWriterImpl.h"
#include "bufferImpl.h"
#include "bufferStreamImpl.h"
#include "dataHandlerImpl.h"
#include "dataHandlerNumericImpl.h"
#include "dataHandlerStringAEImpl.h"
#include "dataHandlerStringASImpl.h"
#include "dataHandlerStringCSImpl.h"
#include "dataHandlerStringDSImpl.h"
#include "dataHandlerStringISImpl.h"
#include "dataHandlerStringLOImpl.h"
#include "dataHandlerStringLTImpl.h"
#include "dataHandlerStringPNImpl.h"
#include "dataHandlerStringSHImpl.h"
#include "dataHandlerStringSTImpl.h"
#include "dataHandlerStringUCImpl.h"
#include "dataHandlerStringUIImpl.h"
#include "dataHandlerStringURImpl.h"
#include "dataHandlerStringUTImpl.h"
#include "dataHandlerDateImpl.h"
#include "dataHandlerDateTimeImpl.h"
#include "dataHandlerTimeImpl.h"
#include "dicomDictImpl.h"
#include "../include/imebra/exceptions.h"
#include "../include/imebra/definitions.h"

#include <vector>
#include <string.h>


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
// imebraBuffer
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
// Buffer's constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
buffer::buffer(const std::shared_ptr<const charsetsList_t>& pCharsets, streamController::tByteOrdering endianType):
    m_byteOrdering(endianType),
    m_originalBufferPosition(0),
    m_originalBufferLength(0),
    m_originalWordLength(1),
    m_pCharsetsList(pCharsets)
{
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Buffer's constructor (on demand content)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
buffer::buffer(
        const std::shared_ptr<baseStreamInput>& originalStream,
        size_t bufferPosition,
        size_t bufferLength,
        size_t wordLength,
        streamController::tByteOrdering endianType,
        const std::shared_ptr<const charsetsList_t>& pCharsets):
        m_byteOrdering(endianType),
        m_originalStream(originalStream),
        m_originalBufferPosition(bufferPosition),
        m_originalBufferLength(bufferLength),
        m_originalWordLength(wordLength),
        m_pCharsetsList(pCharsets)
{
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
buffer::~buffer()
{
}


std::shared_ptr<const memory> buffer::getLocalMemory() const
{
    IMEBRA_FUNCTION_START();

    // If the object must be loaded from the original stream,
    //  then load it...
    ///////////////////////////////////////////////////////////
    if(m_originalStream != nullptr)
    {
        std::shared_ptr<memory> localMemory(std::make_shared<memory>(m_originalBufferLength));
        if(m_originalBufferLength != 0)
        {
            std::shared_ptr<streamReader> reader(std::make_shared<streamReader>(m_originalStream, m_originalBufferPosition, m_originalBufferLength));
            std::vector<std::uint8_t> localBuffer;
            localBuffer.resize(m_originalBufferLength);
            reader->read(&localBuffer[0], m_originalBufferLength);
            if(m_originalWordLength != 0)
            {
                reader->adjustEndian(&localBuffer[0], m_originalWordLength, m_byteOrdering, m_originalBufferLength/m_originalWordLength);
            }
            localMemory->assign(&localBuffer[0], m_originalBufferLength);
        }
        return localMemory;
    }

    return joinMemory();

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<const memory> buffer::joinMemory() const
{
    IMEBRA_FUNCTION_START();

    if(m_memory.empty())
    {
        return std::make_shared<memory>();
    }

    if(m_memory.size() == 1)
    {
        return m_memory.front();
    }

    size_t totalSize(0);
    for(std::list<std::shared_ptr<const memory> >::const_iterator scanMemory(m_memory.begin()), endMemory(m_memory.end());
        scanMemory != endMemory;
        ++scanMemory)
    {
        totalSize += (*scanMemory)->size();
    }

    std::shared_ptr<memory> newMemory(std::make_shared<memory>(totalSize));

    size_t copyIndex(0);
    for(const std::shared_ptr<const memory>& scanMemory: m_memory)
    {
        ::memcpy(newMemory->data() + copyIndex, scanMemory->data(), scanMemory->size());
        copyIndex += scanMemory->size();
    }

    return newMemory;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Create a data handler and connect it to the buffer
// (raw or normal)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<handlers::readingDataHandler> buffer::getReadingDataHandler(tagVR_t tagVR) const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    std::shared_ptr<const memory> localMemory(getLocalMemory());

    switch(tagVR)
    {
    case tagVR_t::AE:
        return std::make_shared<handlers::readingDataHandlerStringAE>(*localMemory);

    case tagVR_t::AS:
        return std::make_shared<handlers::readingDataHandlerStringAS>(*localMemory);

    case tagVR_t::CS:
        return std::make_shared<handlers::readingDataHandlerStringCS>(*localMemory);

    case tagVR_t::DS:
        return std::make_shared<handlers::readingDataHandlerStringDS>(*localMemory);

    case tagVR_t::IS:
        return std::make_shared<handlers::readingDataHandlerStringIS>(*localMemory);

    case tagVR_t::LO:
        return std::make_shared<handlers::readingDataHandlerStringLO>(*localMemory, m_pCharsetsList);

    case tagVR_t::LT:
        return std::make_shared<handlers::readingDataHandlerStringLT>(*localMemory, m_pCharsetsList);

    case tagVR_t::PN:
        return std::make_shared<handlers::readingDataHandlerStringPN>(*localMemory, m_pCharsetsList);

    case tagVR_t::SH:
        return std::make_shared<handlers::readingDataHandlerStringSH>(*localMemory, m_pCharsetsList);

    case tagVR_t::ST:
        return std::make_shared<handlers::readingDataHandlerStringST>(*localMemory, m_pCharsetsList);

    case tagVR_t::UC:
        return std::make_shared<handlers::readingDataHandlerStringUC>(*localMemory, m_pCharsetsList);

    case tagVR_t::UI:
        return std::make_shared<handlers::readingDataHandlerStringUI>(*localMemory);

    case tagVR_t::UR:
        return std::make_shared<handlers::readingDataHandlerStringUR>(*localMemory);

    case tagVR_t::UT:
        return std::make_shared< handlers::readingDataHandlerStringUT>(*localMemory, m_pCharsetsList);

    case tagVR_t::OB:
        return std::make_shared<handlers::readingDataHandlerNumeric<std::uint8_t> >(localMemory, tagVR);

    case tagVR_t::OL:
        return std::make_shared<handlers::readingDataHandlerNumeric<std::int32_t> >(localMemory, tagVR);

    case tagVR_t::SB:
        return std::make_shared<handlers::readingDataHandlerNumeric<std::int8_t> >(localMemory, tagVR);

    case tagVR_t::UN:
        return std::make_shared<handlers::readingDataHandlerNumeric<std::uint8_t> >(localMemory, tagVR);

    case tagVR_t::OW:
        return std::make_shared<handlers::readingDataHandlerNumeric<std::uint16_t> >(localMemory, tagVR);

    case tagVR_t::AT:
        return std::make_shared<handlers::readingDataHandlerNumeric<std::uint32_t> >(localMemory, tagVR);

    case tagVR_t::FL:
        return std::make_shared<handlers::readingDataHandlerNumeric<float> >(localMemory, tagVR);

    case tagVR_t::OF:
        return std::make_shared<handlers::readingDataHandlerNumeric<float> >(localMemory, tagVR);

    case tagVR_t::FD:
        return std::make_shared<handlers::readingDataHandlerNumeric<double> >(localMemory, tagVR);

    case tagVR_t::OD:
        return std::make_shared<handlers::readingDataHandlerNumeric<double> >(localMemory, tagVR);

    case tagVR_t::SL:
        return std::make_shared<handlers::readingDataHandlerNumeric<std::int32_t> >(localMemory, tagVR);

    case tagVR_t::SS:
        return std::make_shared<handlers::readingDataHandlerNumeric<std::int16_t> >(localMemory, tagVR);

    case tagVR_t::UL:
        return std::make_shared<handlers::readingDataHandlerNumeric<std::uint32_t> >(localMemory, tagVR);

    case tagVR_t::US:
        return std::make_shared<handlers::readingDataHandlerNumeric<std::uint16_t> >(localMemory, tagVR);

    case tagVR_t::DA:
        return std::make_shared<handlers::readingDataHandlerDate>(*localMemory);

    case tagVR_t::DT:
        return std::make_shared<handlers::readingDataHandlerDateTime>(*localMemory);

    case tagVR_t::TM:
        return std::make_shared<handlers::readingDataHandlerTime>(*localMemory);

    case tagVR_t::SQ:
        IMEBRA_THROW(std::logic_error, "Cannot retrieve a SQ data handler");

    default:
        IMEBRA_THROW(std::logic_error, "The buffer was created with an invalid buffer type");
    }

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<handlers::writingDataHandler> buffer::getWritingDataHandler(tagVR_t tagVR, std::uint32_t size)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    // Reset the pointer to the data handler
    ///////////////////////////////////////////////////////////
    std::shared_ptr<handlers::writingDataHandler> handler;

    switch(tagVR)
    {
    case tagVR_t::AE:
        return std::make_shared<handlers::writingDataHandlerStringAE>(shared_from_this());

    case tagVR_t::AS:
        return std::make_shared<handlers::writingDataHandlerStringAS>(shared_from_this());

    case tagVR_t::CS:
        return std::make_shared<handlers::writingDataHandlerStringCS>(shared_from_this());

    case tagVR_t::DS:
        return std::make_shared<handlers::writingDataHandlerStringDS>(shared_from_this());

    case tagVR_t::IS:
        return std::make_shared<handlers::writingDataHandlerStringIS>(shared_from_this());

    case tagVR_t::UR:
        return std::make_shared<handlers::writingDataHandlerStringUR>(shared_from_this());

    case tagVR_t::LO:
        return std::make_shared<handlers::writingDataHandlerStringLO>(shared_from_this(), m_pCharsetsList);

    case tagVR_t::LT:
        return std::make_shared<handlers::writingDataHandlerStringLT>(shared_from_this(), m_pCharsetsList);

    case tagVR_t::PN:
        return std::make_shared<handlers::writingDataHandlerStringPN>(shared_from_this(), m_pCharsetsList);

    case tagVR_t::SH:
        return std::make_shared<handlers::writingDataHandlerStringSH>(shared_from_this(), m_pCharsetsList);

    case tagVR_t::ST:
        return std::make_shared<handlers::writingDataHandlerStringST>(shared_from_this(), m_pCharsetsList);

    case tagVR_t::UC:
        return std::make_shared<handlers::writingDataHandlerStringUC>(shared_from_this(), m_pCharsetsList);

    case tagVR_t::UI:
        return std::make_shared<handlers::writingDataHandlerStringUI>(shared_from_this());

    case tagVR_t::UT:
        return std::make_shared< handlers::writingDataHandlerStringUT>(shared_from_this(), m_pCharsetsList);

    case tagVR_t::OB:
        return std::make_shared<handlers::writingDataHandlerNumeric<std::uint8_t> >(shared_from_this(), size, tagVR);

    case tagVR_t::OL:
        return std::make_shared<handlers::writingDataHandlerNumeric<std::int32_t> >(shared_from_this(), size, tagVR);

    case tagVR_t::SB:
        return std::make_shared<handlers::writingDataHandlerNumeric<std::int8_t> >(shared_from_this(), size, tagVR);

    case tagVR_t::UN:
        return std::make_shared<handlers::writingDataHandlerNumeric<std::uint8_t> >(shared_from_this(), size, tagVR);

    case tagVR_t::OW:
        return std::make_shared<handlers::writingDataHandlerNumeric<std::uint16_t> >(shared_from_this(), size, tagVR);

    case tagVR_t::AT:
        return std::make_shared<handlers::writingDataHandlerNumeric<std::uint32_t> >(shared_from_this(), size, tagVR);

    case tagVR_t::FL:
        return std::make_shared<handlers::writingDataHandlerNumeric<float> >(shared_from_this(), size, tagVR);

    case tagVR_t::OF:
        return std::make_shared<handlers::writingDataHandlerNumeric<float> >(shared_from_this(), size, tagVR);

    case tagVR_t::FD:
        return std::make_shared<handlers::writingDataHandlerNumeric<double> >(shared_from_this(), size, tagVR);

    case tagVR_t::OD:
        return std::make_shared<handlers::writingDataHandlerNumeric<double> >(shared_from_this(), size, tagVR);

    case tagVR_t::SL:
        return std::make_shared<handlers::writingDataHandlerNumeric<std::int32_t> >(shared_from_this(), size, tagVR);

    case tagVR_t::SS:
        return std::make_shared<handlers::writingDataHandlerNumeric<std::int16_t> >(shared_from_this(), size, tagVR);

    case tagVR_t::UL:
        return std::make_shared<handlers::writingDataHandlerNumeric<std::uint32_t> >(shared_from_this(), size, tagVR);

    case tagVR_t::US:
        return std::make_shared<handlers::writingDataHandlerNumeric<std::uint16_t> >(shared_from_this(), size, tagVR);

    case tagVR_t::DA:
        return std::make_shared<handlers::writingDataHandlerDate>(shared_from_this());

    case tagVR_t::DT:
        return std::make_shared<handlers::writingDataHandlerDateTime>(shared_from_this());

    case tagVR_t::TM:
        return std::make_shared<handlers::writingDataHandlerTime>(shared_from_this());

    case tagVR_t::SQ:
        IMEBRA_THROW(std::logic_error, "Cannot retrieve a SQ data handler");

    default:
        IMEBRA_THROW(std::logic_error, "The buffer was created with an invalid buffer type");
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Check if the buffer has an external stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool buffer::hasExternalStream() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_originalStream != nullptr;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a reading stream for the buffer
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<streamReader> buffer::getStreamReader()
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    // If the object must be loaded from the original stream,
    //  then return the original stream
    ///////////////////////////////////////////////////////////
    if(m_originalStream != nullptr && (m_originalWordLength <= 1u || m_byteOrdering == streamReader::getPlatformEndian()))
    {
        std::shared_ptr<streamReader> reader(std::make_shared<streamReader>(m_originalStream, m_originalBufferPosition, m_originalBufferLength));
        return reader;
    }

    // Build a stream from the buffer's memory
    ///////////////////////////////////////////////////////////
    std::shared_ptr<streamReader> reader;
    std::shared_ptr<memoryStreamInput> memoryStream = std::make_shared<memoryStreamInput>(getLocalMemory());
    reader = std::shared_ptr<streamReader>(std::make_shared<streamReader>(memoryStream));

    return reader;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a writing stream for the buffer
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<streamWriter> buffer::getStreamWriter(tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<handlers::writingDataHandlerRaw> tempHandlerRaw = getWritingDataHandlerRaw(tagVR);
    return std::make_shared<streamWriter>(std::make_shared<bufferStreamOutput>(tempHandlerRaw));

    IMEBRA_FUNCTION_END();
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Create a raw data handler and connect it to the buffer
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<handlers::readingDataHandlerRaw> buffer::getReadingDataHandlerRaw(tagVR_t tagVR) const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    return std::make_shared<handlers::readingDataHandlerRaw>(getLocalMemory(), tagVR);

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<handlers::writingDataHandlerRaw> buffer::getWritingDataHandlerRaw(tagVR_t tagVR, std::uint32_t size)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    return std::make_shared<handlers::writingDataHandlerRaw>(shared_from_this(), size, tagVR);

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<handlers::readingDataHandlerNumericBase> buffer::getReadingDataHandlerNumeric(tagVR_t tagVR) const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<handlers::readingDataHandler> handler = getReadingDataHandler(tagVR);
    std::shared_ptr<handlers::readingDataHandlerNumericBase> numericHandler = std::dynamic_pointer_cast<handlers::readingDataHandlerNumericBase>(handler);
    if(numericHandler == nullptr)
    {
        IMEBRA_THROW(DataHandlerConversionError, "The data handler does not handle numeric data");
    }

    return numericHandler;

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<handlers::writingDataHandlerNumericBase> buffer::getWritingDataHandlerNumeric(tagVR_t tagVR, std::uint32_t size)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<handlers::writingDataHandler> handler = getWritingDataHandler(tagVR, size);
    std::shared_ptr<handlers::writingDataHandlerNumericBase> numericHandler = std::dynamic_pointer_cast<handlers::writingDataHandlerNumericBase>(handler);
    if(numericHandler == nullptr)
    {
        IMEBRA_THROW(DataHandlerConversionError, "The data handler does not handle numeric data");
    }

    return numericHandler;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Append a block of memory
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void buffer::appendMemory(std::shared_ptr<const memory> pMemory)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    m_memory.push_back(pMemory);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Return the buffer's size in bytes
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
size_t buffer::getBufferSizeBytes() const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    // The buffer has not been loaded yet
    ///////////////////////////////////////////////////////////
    if(m_originalStream != nullptr)
    {
        return m_originalBufferLength;
    }

    size_t totalSize(0);
    for(std::list<std::shared_ptr<const memory> >::const_iterator scanMemory(m_memory.begin()), endMemory(m_memory.end()); scanMemory != endMemory; ++scanMemory)
    {
        totalSize += (*scanMemory)->size();
    }

    return totalSize;

    IMEBRA_FUNCTION_END();
}


streamController::tByteOrdering buffer::getEndianType() const
{
    return m_byteOrdering;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Commit the changes made by copyBack
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void buffer::commit(std::shared_ptr<memory> newMemory)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    m_memory.clear();
    m_memory.push_back(newMemory);
    m_originalStream.reset();

    IMEBRA_FUNCTION_END();
}


} // namespace implementation

} // namespace imebra

