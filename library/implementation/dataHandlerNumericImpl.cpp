/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerNumeric.cpp
    \brief Implementation of the handler for the numeric tags.

*/

#include "dataHandlerNumericImpl.h"
#include "memoryImpl.h"
#include "bufferImpl.h"

namespace imebra
{

namespace implementation
{

namespace handlers
{

readingDataHandlerNumericBase::readingDataHandlerNumericBase(const std::shared_ptr<const memory>& parseMemory, tagVR_t dataType):
    readingDataHandler(dataType), m_pMemory(parseMemory)
{
}

const std::uint8_t* readingDataHandlerNumericBase::getMemoryBuffer() const
{
    IMEBRA_FUNCTION_START();

    return m_pMemory->data();

    IMEBRA_FUNCTION_END();
}

size_t readingDataHandlerNumericBase::getMemorySize() const
{
    IMEBRA_FUNCTION_START();

    return m_pMemory->size();

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<const memory> readingDataHandlerNumericBase::getMemory() const
{
    return m_pMemory;
}

void readingDataHandlerNumericBase::copyTo(std::shared_ptr<writingDataHandlerNumericBase> pDestination)
{
    IMEBRA_FUNCTION_START();

    imebra::implementation::handlers::writingDataHandlerNumericBase* pHandler(pDestination.get());
    if(typeid(*pHandler) == typeid(imebra::implementation::handlers::writingDataHandlerNumeric<std::uint8_t>) ||
        dynamic_cast<imebra::implementation::handlers::writingDataHandlerNumeric<std::uint8_t>* >(pHandler) != 0)
    {
        copyTo((std::uint8_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else if(typeid(*pHandler) == typeid(imebra::implementation::handlers::writingDataHandlerNumeric<std::int8_t>))
    {
        copyTo((std::int8_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else if(typeid(*pHandler) == typeid(imebra::implementation::handlers::writingDataHandlerNumeric<std::uint16_t>))
    {
        copyTo((std::uint16_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else if(typeid(*pHandler) == typeid(imebra::implementation::handlers::writingDataHandlerNumeric<std::int16_t>))
    {
        copyTo((std::int16_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else if(typeid(*pHandler) == typeid(imebra::implementation::handlers::writingDataHandlerNumeric<std::uint32_t>))
    {
        copyTo((std::uint32_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else if(typeid(*pHandler) == typeid(imebra::implementation::handlers::writingDataHandlerNumeric<std::int32_t>))
    {
        copyTo((std::int32_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else if(typeid(*pHandler) == typeid(imebra::implementation::handlers::writingDataHandlerNumeric<float>))
    {
        copyTo((float*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else if(typeid(*pHandler) == typeid(imebra::implementation::handlers::writingDataHandlerNumeric<double>))
    {
        copyTo((double*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else
    {
        IMEBRA_THROW(std::runtime_error, "Data type not valid");
    }

    IMEBRA_FUNCTION_END();
}


writingDataHandlerNumericBase::writingDataHandlerNumericBase(const std::shared_ptr<buffer> &pBuffer, const size_t initialSize, tagVR_t dataType, size_t unitSize):
    writingDataHandler(pBuffer, dataType), m_pMemory(std::make_shared<memory>(initialSize * unitSize))
{
}

size_t writingDataHandlerNumericBase::getSize() const
{
    IMEBRA_FUNCTION_START();

    return m_pMemory->size() / getUnitSize();

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<memory> writingDataHandlerNumericBase::getMemory() const
{
    return m_pMemory;
}

// Set the buffer's size, in data elements
///////////////////////////////////////////////////////////
void writingDataHandlerNumericBase::setSize(const size_t elementsNumber)
{
    IMEBRA_FUNCTION_START();

    m_pMemory->resize(elementsNumber * getUnitSize());

    IMEBRA_FUNCTION_END();
}


writingDataHandlerNumericBase::~writingDataHandlerNumericBase()
{
    if(m_buffer != 0)
    {
        m_buffer->commit(m_pMemory);
    }
}

std::uint8_t* writingDataHandlerNumericBase::getMemoryBuffer() const
{
    IMEBRA_FUNCTION_START();

    return m_pMemory->data();

    IMEBRA_FUNCTION_END();
}

size_t writingDataHandlerNumericBase::getMemorySize() const
{
    IMEBRA_FUNCTION_START();

    return m_pMemory->size();

    IMEBRA_FUNCTION_END();
}

// Copy the data from another handler
///////////////////////////////////////////////////////////
void writingDataHandlerNumericBase::copyFrom(std::shared_ptr<readingDataHandlerNumericBase> pSource)
{
    IMEBRA_FUNCTION_START();

    imebra::implementation::handlers::readingDataHandlerNumericBase* pHandler(pSource.get());
    if(typeid(*pHandler) == typeid(imebra::implementation::handlers::readingDataHandlerNumeric<std::uint8_t>) ||
        dynamic_cast<imebra::implementation::handlers::readingDataHandlerNumeric<std::uint8_t>* >(pHandler) != 0)
    {
        copyFrom((std::uint8_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else if(typeid(*pHandler) == typeid(imebra::implementation::handlers::readingDataHandlerNumeric<std::int8_t>))
    {
        copyFrom((std::int8_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else if(typeid(*pHandler) == typeid(imebra::implementation::handlers::readingDataHandlerNumeric<std::uint16_t>))
    {
        copyFrom((std::uint16_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else if(typeid(*pHandler) == typeid(imebra::implementation::handlers::readingDataHandlerNumeric<std::int16_t>))
    {
        copyFrom((std::int16_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else if(typeid(*pHandler) == typeid(imebra::implementation::handlers::readingDataHandlerNumeric<std::uint32_t>))
    {
        copyFrom((std::uint32_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else if(typeid(*pHandler) == typeid(imebra::implementation::handlers::readingDataHandlerNumeric<std::int32_t>))
    {
        copyFrom((std::int32_t*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else if(typeid(*pHandler) == typeid(imebra::implementation::handlers::readingDataHandlerNumeric<float>))
    {
        copyFrom((float*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else if(typeid(*pHandler) == typeid(imebra::implementation::handlers::readingDataHandlerNumeric<double>))
    {
        copyFrom((double*)pHandler->getMemoryBuffer(), pHandler->getSize());
    }
    else
    {
        IMEBRA_THROW(std::runtime_error, "Data type not valid");
    }

    IMEBRA_FUNCTION_END();

}


}

}

}
