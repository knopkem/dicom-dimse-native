/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandler.cpp
    \brief Implementation of the classes ReadingDataHandler & WritingDataHandler.
*/

#include "../include/imebra/writingDataHandlerNumeric.h"
#include "../include/imebra/readingDataHandlerNumeric.h"
#include "../implementation/dataHandlerImpl.h"
#include "../implementation/dataHandlerNumericImpl.h"
#include <cstring>

namespace imebra
{

WritingDataHandlerNumeric::WritingDataHandlerNumeric(const std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase>& pDataHandler):
    WritingDataHandler(pDataHandler)
{
}

WritingDataHandlerNumeric::WritingDataHandlerNumeric(const WritingDataHandlerNumeric& source): WritingDataHandler(source)
{
}

WritingDataHandlerNumeric::~WritingDataHandlerNumeric()
{
}

MutableMemory WritingDataHandlerNumeric::getMemory() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase> numericDataHandler =
            std::dynamic_pointer_cast<implementation::handlers::writingDataHandlerNumericBase>(getWritingDataHandlerImplementation(*this));
    return MutableMemory(numericDataHandler->getMemory());

    IMEBRA_FUNCTION_END_LOG();
}

void WritingDataHandlerNumeric::assign(const char* source, size_t sourceSize)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase> numericDataHandler =
            std::dynamic_pointer_cast<implementation::handlers::writingDataHandlerNumericBase>(getWritingDataHandlerImplementation(*this));
    numericDataHandler->getMemory()->assign((std::uint8_t*) source, sourceSize);

    IMEBRA_FUNCTION_END_LOG();
}

char* WritingDataHandlerNumeric::data(size_t* pDataSize) const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase> numericDataHandler =
            std::dynamic_pointer_cast<implementation::handlers::writingDataHandlerNumericBase>(getWritingDataHandlerImplementation(*this));
    *pDataSize = numericDataHandler->getMemorySize();
    return (char*)numericDataHandler->getMemoryBuffer();

    IMEBRA_FUNCTION_END_LOG();
}

size_t WritingDataHandlerNumeric::data(char* destination, size_t destinationSize) const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase> numericDataHandler =
            std::dynamic_pointer_cast<implementation::handlers::writingDataHandlerNumericBase>(getWritingDataHandlerImplementation(*this));
    size_t memorySize = numericDataHandler->getMemorySize();
    if(destination != 0 && destinationSize >= memorySize && memorySize != 0)
    {
        ::memcpy(destination, numericDataHandler->getMemoryBuffer(), memorySize);
    }
    return memorySize;

    IMEBRA_FUNCTION_END_LOG();
}

size_t WritingDataHandlerNumeric::getUnitSize() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase> numericDataHandler =
            std::dynamic_pointer_cast<implementation::handlers::writingDataHandlerNumericBase>(getWritingDataHandlerImplementation(*this));
    return numericDataHandler->getUnitSize();

    IMEBRA_FUNCTION_END_LOG();
}

bool WritingDataHandlerNumeric::isSigned() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase> numericDataHandler =
            std::dynamic_pointer_cast<implementation::handlers::writingDataHandlerNumericBase>(getWritingDataHandlerImplementation(*this));
    return numericDataHandler->isSigned();

    IMEBRA_FUNCTION_END_LOG();
}

bool WritingDataHandlerNumeric::isFloat() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase> numericDataHandler =
            std::dynamic_pointer_cast<implementation::handlers::writingDataHandlerNumericBase>(getWritingDataHandlerImplementation(*this));
    return numericDataHandler->isFloat();

    IMEBRA_FUNCTION_END_LOG();
}

void WritingDataHandlerNumeric::copyFrom(const ReadingDataHandlerNumeric& source)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase> numericDataHandler =
            std::dynamic_pointer_cast<implementation::handlers::writingDataHandlerNumericBase>(getWritingDataHandlerImplementation(*this));
    return numericDataHandler->copyFrom(std::dynamic_pointer_cast<implementation::handlers::readingDataHandlerNumericBase>(getReadingDataHandlerImplementation(source)));

    IMEBRA_FUNCTION_END_LOG();
}

}
