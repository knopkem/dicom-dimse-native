/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#include "../include/imebra/dataSet.h"
#include "../include/imebra/tag.h"
#include "../include/imebra/lut.h"
#include "../include/imebra/date.h"
#include "../include/imebra/age.h"
#include "../include/imebra/patientName.h"
#include "../include/imebra/VOIDescription.h"
#include "../include/imebra/streamReader.h"
#include "../include/imebra/overlay.h"
#include "../implementation/dataSetImpl.h"
#include "../include/imebra/streamWriter.h"
#include "../include/imebra/uidsEnumeration.h"
#include "../implementation/dataHandlerNumericImpl.h"
#include "../implementation/charsetConversionBaseImpl.h"
#include "../implementation/streamWriterImpl.h"
#include "../implementation/streamReaderImpl.h"
#include <typeinfo>
#include <memory>

namespace imebra
{

DataSet::DataSet(const DataSet& source): m_pDataSet(getDataSetImplementation(source))
{
}

DataSet::DataSet(const std::shared_ptr<imebra::implementation::dataSet>& pDataSet): m_pDataSet(pDataSet)
{
}

DataSet::~DataSet()
{
}

const std::shared_ptr<implementation::dataSet>& getDataSetImplementation(const DataSet& dataSet)
{
    return dataSet.m_pDataSet;
}

tagsIds_t DataSet::getTags() const
{
    IMEBRA_FUNCTION_START();

    tagsIds_t returnTags;

    implementation::dataSet::tGroupsIds groups = m_pDataSet->getGroups();
    for(implementation::dataSet::tGroupsIds::const_iterator scanGroups(groups.begin()), endGroups(groups.end());
        scanGroups != endGroups;
        ++scanGroups)
    {
        std::uint32_t orders = m_pDataSet->getGroupsNumber(*scanGroups);
        for(std::uint32_t scanOrders(0); scanOrders != orders; ++scanOrders)
        {
            const implementation::dataSet::tTags tags = m_pDataSet->getGroupTags(*scanGroups, scanOrders);
            for(implementation::dataSet::tTags::const_iterator scanTags(tags.begin()), endTags(tags.end());
                scanTags != endTags;
                ++scanTags)
            {
                returnTags.push_back(TagId(*scanGroups, scanOrders, scanTags->first));
            }
        }
    }

    return returnTags;

    IMEBRA_FUNCTION_END_LOG();
}

const Tag DataSet::getTag(const TagId& tagId) const
{
    IMEBRA_FUNCTION_START();

    return Tag(m_pDataSet->getTag(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId()));

    IMEBRA_FUNCTION_END_LOG();
}

const Image DataSet::getImage(size_t frameNumber) const
{
    IMEBRA_FUNCTION_START();

    return Image(m_pDataSet->getImage(static_cast<std::uint32_t>(frameNumber)));

    IMEBRA_FUNCTION_END_LOG();
}

const Overlay DataSet::getOverlay(size_t overlayNumber) const
{
    IMEBRA_FUNCTION_START();

    return Overlay(m_pDataSet->getOverlay(static_cast<std::uint32_t>(overlayNumber)));

    IMEBRA_FUNCTION_END();
}

const Image DataSet::getImageApplyModalityTransform(size_t frameNumber) const
{
    IMEBRA_FUNCTION_START();

    return Image(m_pDataSet->getModalityImage((std::uint32_t)frameNumber));

    IMEBRA_FUNCTION_END_LOG();
}

StreamReader DataSet::getStreamReader(const TagId& tagId, size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return StreamReader(m_pDataSet->getStreamReader(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId));

    IMEBRA_FUNCTION_END_LOG();
}

const DataSet DataSet::getSequenceItem(const TagId& tagId, size_t itemId) const
{
    IMEBRA_FUNCTION_START();

    return DataSet(m_pDataSet->getSequenceItem(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), itemId));

    IMEBRA_FUNCTION_END_LOG();
}

const LUT DataSet::getLUT(const TagId &tagId, size_t itemId) const
{
    IMEBRA_FUNCTION_START();

    return LUT(m_pDataSet->getLut(tagId.getGroupId(), tagId.getTagId(), itemId));

    IMEBRA_FUNCTION_END_LOG();
}

vois_t DataSet::getVOIs() const
{
    IMEBRA_FUNCTION_START();

    vois_t vois;
    for(const std::shared_ptr<const implementation::VOIDescription>& voi: m_pDataSet->getVOIs())
    {
        vois.push_back(VOIDescription(voi));
    }

    return vois;

    IMEBRA_FUNCTION_END_LOG();
}


const DataSet DataSet::getFunctionalGroupDataSet(size_t frameNumber) const
{
    IMEBRA_FUNCTION_START();

    return DataSet(m_pDataSet->getFunctionalGroupDataSet(frameNumber));

    IMEBRA_FUNCTION_END_LOG();
}


ReadingDataHandler DataSet::getReadingDataHandler(const TagId& tagId, size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return ReadingDataHandler(m_pDataSet->getReadingDataHandler(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId));

    IMEBRA_FUNCTION_END_LOG();
}

ReadingDataHandlerNumeric DataSet::getReadingDataHandlerNumeric(const TagId& tagId, size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return ReadingDataHandlerNumeric(m_pDataSet->getReadingDataHandlerNumeric(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId));

    IMEBRA_FUNCTION_END_LOG();
}

ReadingDataHandlerNumeric DataSet::getReadingDataHandlerRaw(const TagId& tagId, size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::handlers::readingDataHandlerNumericBase> numericHandler = m_pDataSet->getReadingDataHandlerRaw(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId);
    return ReadingDataHandlerNumeric(numericHandler);

    IMEBRA_FUNCTION_END_LOG();
}

bool DataSet::bufferExists(const TagId& tagId, size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataSet->bufferExists(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId);

    IMEBRA_FUNCTION_END_LOG();
}

std::int32_t DataSet::getSignedLong(const TagId& tagId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataSet->getSignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber);

    IMEBRA_FUNCTION_END_LOG();
}

std::int32_t DataSet::getSignedLong(const TagId& tagId, size_t elementNumber, std::int32_t defaultValue) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataSet->getSignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, defaultValue);

    IMEBRA_FUNCTION_END_LOG();
}

std::uint32_t DataSet::getUnsignedLong(const TagId& tagId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataSet->getUnsignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber);

    IMEBRA_FUNCTION_END_LOG();
}

std::uint32_t DataSet::getUnsignedLong(const TagId& tagId, size_t elementNumber, std::uint32_t defaultValue) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataSet->getUnsignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, defaultValue);

    IMEBRA_FUNCTION_END_LOG();
}

double DataSet::getDouble(const TagId& tagId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataSet->getDouble(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber);

    IMEBRA_FUNCTION_END_LOG();
}

double DataSet::getDouble(const TagId& tagId, size_t elementNumber, double defaultValue) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataSet->getDouble(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, defaultValue);

    IMEBRA_FUNCTION_END_LOG();
}

std::string DataSet::getString(const TagId& tagId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataSet->getString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber);

    IMEBRA_FUNCTION_END_LOG();
}

std::string DataSet::getString(const TagId& tagId, size_t elementNumber, const std::string& defaultValue) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataSet->getString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, defaultValue);

    IMEBRA_FUNCTION_END_LOG();
}

std::wstring DataSet::getUnicodeString(const TagId& tagId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataSet->getUnicodeString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber);

    IMEBRA_FUNCTION_END_LOG();
}

std::wstring DataSet::getUnicodeString(const TagId& tagId, size_t elementNumber, const std::wstring& defaultValue) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataSet->getUnicodeString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, defaultValue);

    IMEBRA_FUNCTION_END_LOG();
}

const Age DataSet::getAge(const TagId& tagId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return Age(m_pDataSet->getAge(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber));

    IMEBRA_FUNCTION_END_LOG();
}

const Age DataSet::getAge(const TagId& tagId, size_t elementNumber, const Age& defaultValue) const
{
    IMEBRA_FUNCTION_START();

    return Age(m_pDataSet->getAge(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, getAgeImplementation(defaultValue)));

    IMEBRA_FUNCTION_END_LOG();
}

const Date DataSet::getDate(const TagId& tagId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return Date(m_pDataSet->getDate(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber));

    IMEBRA_FUNCTION_END_LOG();
}

const Date DataSet::getDate(const TagId& tagId, size_t elementNumber, const Date& defaultValue) const
{
    IMEBRA_FUNCTION_START();

    return Date(m_pDataSet->getDate(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, getDateImplementation(defaultValue)));

    IMEBRA_FUNCTION_END_LOG();
}

const PatientName DataSet::getPatientName(const TagId& tagId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return PatientName(m_pDataSet->getPatientName(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber));

    IMEBRA_FUNCTION_END_LOG();
}

const PatientName DataSet::getPatientName(const TagId& tagId, size_t elementNumber, const PatientName& defaultValue) const
{
    IMEBRA_FUNCTION_START();

    return PatientName(m_pDataSet->getPatientName(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, getPatientNameImplementation(defaultValue)));

    IMEBRA_FUNCTION_END_LOG();
}

const UnicodePatientName DataSet::getUnicodePatientName(const TagId& tagId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return UnicodePatientName(m_pDataSet->getUnicodePatientName(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber));

    IMEBRA_FUNCTION_END_LOG();
}

const UnicodePatientName DataSet::getUnicodePatientName(const TagId& tagId, size_t elementNumber, const UnicodePatientName& defaultValue) const
{
    IMEBRA_FUNCTION_START();

    return UnicodePatientName(m_pDataSet->getUnicodePatientName(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, getUnicodePatientNameImplementation(defaultValue)));

    IMEBRA_FUNCTION_END_LOG();
}

tagVR_t DataSet::getDataType(const TagId& tagId) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataSet->getDataType(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId());

    IMEBRA_FUNCTION_END_LOG();
}

MutableDataSet::MutableDataSet(const MutableDataSet &source): DataSet(source)
{
}

MutableDataSet::MutableDataSet(): DataSet(std::make_shared<imebra::implementation::dataSet>(uidImplicitVRLittleEndian_1_2_840_10008_1_2, charsetsList_t()))
{
}

MutableDataSet::MutableDataSet(const std::shared_ptr<implementation::dataSet>& pDataSet): DataSet(pDataSet)
{
}

MutableDataSet::MutableDataSet(const std::string &transferSyntax): DataSet(std::make_shared<imebra::implementation::dataSet>(transferSyntax, charsetsList_t()))
{
}

MutableDataSet::MutableDataSet(const std::string &transferSyntax, const charsetsList_t &charsets):
    DataSet(std::make_shared<imebra::implementation::dataSet>(transferSyntax, charsets))
{
}

MutableDataSet::~MutableDataSet()
{
}

MutableTag MutableDataSet::getTagCreate(const TagId& tagId, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    return MutableTag(getDataSetImplementation(*this)->getTagCreate(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), tagVR));

    IMEBRA_FUNCTION_END_LOG();
}

MutableTag MutableDataSet::getTagCreate(const TagId& tagId)
{
    IMEBRA_FUNCTION_START();

    return MutableTag(getDataSetImplementation(*this)->getTagCreate(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId()));

    IMEBRA_FUNCTION_END_LOG();
}

WritingDataHandler MutableDataSet::getWritingDataHandler(const TagId& tagId, size_t bufferId, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    return WritingDataHandler(getDataSetImplementation(*this)->getWritingDataHandler(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId, tagVR));

    IMEBRA_FUNCTION_END_LOG();
}

WritingDataHandler MutableDataSet::getWritingDataHandler(const TagId& tagId, size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    return WritingDataHandler(getDataSetImplementation(*this)->getWritingDataHandler(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId));

    IMEBRA_FUNCTION_END_LOG();
}

WritingDataHandlerNumeric MutableDataSet::getWritingDataHandlerNumeric(const TagId& tagId, size_t bufferId, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    return WritingDataHandlerNumeric(getDataSetImplementation(*this)->getWritingDataHandlerNumeric(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId, tagVR));

    IMEBRA_FUNCTION_END_LOG();
}

WritingDataHandlerNumeric MutableDataSet::getWritingDataHandlerNumeric(const TagId& tagId, size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    return WritingDataHandlerNumeric(getDataSetImplementation(*this)->getWritingDataHandlerNumeric(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId));

    IMEBRA_FUNCTION_END_LOG();
}

WritingDataHandlerNumeric MutableDataSet::getWritingDataHandlerRaw(const TagId& tagId, size_t bufferId, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase> numericHandler = getDataSetImplementation(*this)->getWritingDataHandlerRaw(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId, tagVR);
    return WritingDataHandlerNumeric(numericHandler);

    IMEBRA_FUNCTION_END_LOG();
}

WritingDataHandlerNumeric MutableDataSet::getWritingDataHandlerRaw(const TagId& tagId, size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase> numericHandler = getDataSetImplementation(*this)->getWritingDataHandlerRaw(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId);
    return WritingDataHandlerNumeric(numericHandler);

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setImage(size_t frameNumber, const Image& image, imageQuality_t quality)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setImage(static_cast<std::uint32_t>(frameNumber), getImageImplementation(image), quality);

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setOverlay(size_t overlayNumber, const Overlay& overlay)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setOverlay(static_cast<std::uint32_t>(overlayNumber), getOverlayImplementation(overlay));

    IMEBRA_FUNCTION_END();
}

StreamWriter MutableDataSet::getStreamWriter(const TagId& tagId, size_t bufferId, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    return StreamWriter(getDataSetImplementation(*this)->getStreamWriter(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId, tagVR));

    IMEBRA_FUNCTION_END_LOG();
}

StreamWriter MutableDataSet::getStreamWriter(const TagId& tagId, size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    return StreamWriter(getDataSetImplementation(*this)->getStreamWriter(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId));

    IMEBRA_FUNCTION_END_LOG();
}

MutableDataSet MutableDataSet::appendSequenceItem(const TagId &tagId)
{
    IMEBRA_FUNCTION_START();

    return MutableDataSet(getDataSetImplementation(*this)->appendSequenceItem(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId()));

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setSignedLong(const TagId& tagId, std::int32_t newValue, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setSignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newValue, tagVR);

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setSignedLong(const TagId& tagId, std::int32_t newValue)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setSignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newValue);

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setUnsignedLong(const TagId& tagId, std::uint32_t newValue, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setUnsignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newValue, tagVR);

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setUnsignedLong(const TagId& tagId, std::uint32_t newValue)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setUnsignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newValue);

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setDouble(const TagId& tagId, double newValue, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setDouble(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newValue, tagVR);

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setDouble(const TagId& tagId, double newValue)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setDouble(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newValue);

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setString(const TagId& tagId, const std::string& newString, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newString, tagVR);

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setString(const TagId& tagId, const std::string& newString)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newString);

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setUnicodeString(const TagId& tagId, const std::wstring& newString, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setUnicodeString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newString, tagVR);

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setUnicodeString(const TagId& tagId, const std::wstring& newString)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setUnicodeString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newString);

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setAge(const TagId& tagId, const Age& age)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setAge(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, getAgeImplementation(age));

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setDate(const TagId& tagId, const Date& date, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setDate(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, getDateImplementation(date), tagVR);

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setDate(const TagId& tagId, const Date& date)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setDate(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, getDateImplementation(date));

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setPatientName(const TagId& tagId, const PatientName& patientName)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setPatientName(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, getPatientNameImplementation(patientName));

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDataSet::setUnicodePatientName(const TagId& tagId, const UnicodePatientName& patientName)
{
    IMEBRA_FUNCTION_START();

    getDataSetImplementation(*this)->setUnicodePatientName(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, getUnicodePatientNameImplementation(patientName));

    IMEBRA_FUNCTION_END_LOG();
}

} // namespace imebra
