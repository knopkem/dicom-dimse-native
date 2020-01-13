/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#include "../implementation/dataImpl.h"
#include "../implementation/dataHandlerNumericImpl.h"
#include "../implementation/fileStreamImpl.h"

#include "../include/imebra/tag.h"
#include "../include/imebra/readingDataHandlerNumeric.h"
#include "../include/imebra/writingDataHandlerNumeric.h"
#include "../include/imebra/definitions.h"
#include "../include/imebra/streamReader.h"
#include "../include/imebra/streamWriter.h"
#include "../include/imebra/dataSet.h"
#include "../include/imebra/baseStreamInput.h"
#include "../include/imebra/fileStreamInput.h"

namespace imebra
{

Tag::Tag(const Tag& source): m_pData(getTagImplementation(source))
{
}

const std::shared_ptr<imebra::implementation::data>& getTagImplementation(const Tag& tag)
{
    return tag.m_pData;
}

Tag::~Tag()
{
}

Tag::Tag(const std::shared_ptr<imebra::implementation::data>& pData): m_pData(pData)
{
}

size_t Tag::getBuffersCount() const
{
    IMEBRA_FUNCTION_START();

    return m_pData->getBuffersCount();

    IMEBRA_FUNCTION_END_LOG();
}

bool Tag::bufferExists(size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return m_pData->bufferExists(bufferId);

    IMEBRA_FUNCTION_END_LOG();
}

size_t Tag::getBufferSize(size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return m_pData->getBufferSize(bufferId);

    IMEBRA_FUNCTION_END_LOG();
}

ReadingDataHandler Tag::getReadingDataHandler(size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return ReadingDataHandler(m_pData->getReadingDataHandler(bufferId));

    IMEBRA_FUNCTION_END_LOG();
}

ReadingDataHandlerNumeric Tag::getReadingDataHandlerNumeric(size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::handlers::readingDataHandlerNumericBase> numericHandler =
            std::dynamic_pointer_cast<implementation::handlers::readingDataHandlerNumericBase>(m_pData->getReadingDataHandler(bufferId));
    if(numericHandler.get() == 0)
    {
        throw std::bad_cast();
    }
    return ReadingDataHandlerNumeric(numericHandler);

    IMEBRA_FUNCTION_END_LOG();
}

ReadingDataHandlerNumeric Tag::getReadingDataHandlerRaw(size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::handlers::readingDataHandlerNumericBase> numericHandler = m_pData->getReadingDataHandlerRaw(bufferId);
    return ReadingDataHandlerNumeric(numericHandler);

    IMEBRA_FUNCTION_END_LOG();
}

StreamReader Tag::getStreamReader(size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return StreamReader(m_pData->getStreamReader(bufferId));

    IMEBRA_FUNCTION_END_LOG();
}

const DataSet Tag::getSequenceItem(size_t dataSetId) const
{
    IMEBRA_FUNCTION_START();

    return DataSet(m_pData->getSequenceItem(dataSetId));

    IMEBRA_FUNCTION_END_LOG();
}

bool Tag::sequenceItemExists(size_t dataSetId) const
{
    IMEBRA_FUNCTION_START();

    return m_pData->dataSetExists(dataSetId);

    IMEBRA_FUNCTION_END_LOG();
}

tagVR_t Tag::getDataType() const
{
    IMEBRA_FUNCTION_START();

    return m_pData->getDataType();

    IMEBRA_FUNCTION_END_LOG();
}

MutableTag::MutableTag(const std::shared_ptr<imebra::implementation::data>& pData): Tag(pData)
{
}

MutableTag::MutableTag(const MutableTag& source): Tag(source)
{
}

MutableTag::~MutableTag()
{
}

WritingDataHandler MutableTag::getWritingDataHandler(size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    return WritingDataHandler(getTagImplementation(*this)->getWritingDataHandler(bufferId));

    IMEBRA_FUNCTION_END_LOG();
}

WritingDataHandlerNumeric MutableTag::getWritingDataHandlerNumeric(size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase> numericHandler =
            std::dynamic_pointer_cast<implementation::handlers::writingDataHandlerNumericBase>(getTagImplementation(*this)->getWritingDataHandler(bufferId));
    if(numericHandler.get() == 0)
    {
        throw std::bad_cast();
    }
    return WritingDataHandlerNumeric(numericHandler);

    IMEBRA_FUNCTION_END_LOG();
}

WritingDataHandlerNumeric MutableTag::getWritingDataHandlerRaw(size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase> numericHandler = getTagImplementation(*this)->getWritingDataHandlerRaw(bufferId);
    return WritingDataHandlerNumeric(numericHandler);

    IMEBRA_FUNCTION_END_LOG();
}

StreamWriter MutableTag::getStreamWriter(size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    return StreamWriter(getTagImplementation(*this)->getStreamWriter(bufferId));

    IMEBRA_FUNCTION_END_LOG();
}

MutableDataSet MutableTag::appendSequenceItem()
{
    IMEBRA_FUNCTION_START();

    return MutableDataSet(getTagImplementation(*this)->appendSequenceItem());

    IMEBRA_FUNCTION_END_LOG();
}

void MutableTag::setStream(size_t bufferId, FileStreamInput& streamInput)
{
    IMEBRA_FUNCTION_START();

    getTagImplementation(*this)->setExternalStream(bufferId, std::dynamic_pointer_cast<implementation::fileStreamInput>(getBaseStreamInputImplementation(streamInput)));

    IMEBRA_FUNCTION_END_LOG();
}

}
