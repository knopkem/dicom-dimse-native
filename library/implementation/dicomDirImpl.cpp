/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dicomDir.cpp
    \brief Implementation of the classes dicomDir and directoryRecord.

*/

#include "dicomDirImpl.h"
#include "dataSetImpl.h"
#include "dicomStreamCodecImpl.h"
#include "dataHandlerNumericImpl.h"
#include "nullStreamImpl.h"
#include "streamWriterImpl.h"
#include "dataHandlerImpl.h"
#include "../include/imebra/exceptions.h"
#include <map>
#include <string>

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
// directoryRecord
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
directoryRecord::directoryRecord(std::shared_ptr<dataSet> pDataSet):
    m_pDataSet(pDataSet)
{
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the record's dataset
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<dataSet> directoryRecord::getRecordDataSet() const
{
    return m_pDataSet;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the next sibling record
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<directoryRecord> directoryRecord::getNextRecord() const
{
    return m_pNextRecord;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the first child record
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<directoryRecord> directoryRecord::getFirstChildRecord() const
{
    return m_pFirstChildRecord;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Sets the next sibling record
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void directoryRecord::setNextRecord(std::shared_ptr<directoryRecord> pNextRecord)
{
    IMEBRA_FUNCTION_START();

    if(pNextRecord != 0)
    {
        pNextRecord->checkCircularReference(this);
    }
    m_pNextRecord = pNextRecord;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the first child record
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void directoryRecord::setFirstChildRecord(std::shared_ptr<directoryRecord> pFirstChildRecord)
{
    IMEBRA_FUNCTION_START();

    if(pFirstChildRecord != 0)
    {
        pFirstChildRecord->checkCircularReference(this);
    }
    m_pFirstChildRecord = pFirstChildRecord;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns a part of the full file path
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
fileParts_t directoryRecord::getFileParts() const
{
    IMEBRA_FUNCTION_START();

    fileParts_t fileParts;

    std::shared_ptr<handlers::readingDataHandler> filePartsHandler = getRecordDataSet()->getReadingDataHandler(0x0004, 0, 0x1500, 0);

    for(size_t scanParts(0), endParts(filePartsHandler->getSize()); scanParts != endParts; ++scanParts)
    {
        fileParts.push_back(filePartsHandler->getString(scanParts));
    }

    return fileParts;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set a part of the full file path
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void directoryRecord::setFileParts(const fileParts_t& fileParts)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<handlers::writingDataHandler> filePartsHandler = getRecordDataSet()->getWritingDataHandler(0x0004, 0, 0x1500, 0);
    filePartsHandler->setSize(fileParts.size());

    size_t bufferIndex(0);
    for(fileParts_t::const_iterator scanParts(fileParts.begin()), endParts(fileParts.end()); scanParts != endParts; ++scanParts)
    {
        filePartsHandler->setString(bufferIndex++, *scanParts);
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the record's type (string)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string directoryRecord::getTypeString() const
{
    IMEBRA_FUNCTION_START();

    return getRecordDataSet()->getString(0x0004, 0, 0x1430, 0, 0);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the item's type
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void directoryRecord::setTypeString(const std::string& recordType)
{
    IMEBRA_FUNCTION_START();

    getRecordDataSet()->setString(0x0004, 0, 0x1430, 0, recordType);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Update the dataSet's offsets.
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void directoryRecord::updateOffsets()
{
    IMEBRA_FUNCTION_START();

    // Update offset for the next record
    ///////////////////////////////////////////////////////////
    if(m_pNextRecord == 0)
    {
        getRecordDataSet()->setUnsignedLong(0x0004, 0, 0x1400, 0, 0);
    }
    else
    {
        getRecordDataSet()->setUnsignedLong(0x0004, 0, 0x1400, 0, m_pNextRecord->getRecordDataSet()->getItemOffset());
        m_pNextRecord->updateOffsets();
    }

    // Update offset for the first child record
    ///////////////////////////////////////////////////////////
    if(m_pFirstChildRecord == 0)
    {
        getRecordDataSet()->setUnsignedLong(0x0004, 0, 0x1420, 0, 0);
    }
    else
    {
        getRecordDataSet()->setUnsignedLong(0x0004, 0, 0x1420, 0, m_pFirstChildRecord->getRecordDataSet()->getItemOffset());
        m_pFirstChildRecord->updateOffsets();
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Check circular (recursive) references between the
//  dicomdir's items
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void directoryRecord::checkCircularReference(directoryRecord* pStartRecord)
{
    IMEBRA_FUNCTION_START();

    if(this == pStartRecord)
    {
        IMEBRA_THROW(DicomDirCircularReferenceError, "Circular reference detected");
    }

    if(m_pNextRecord != 0)
    {
        m_pNextRecord->checkCircularReference(pStartRecord);
    }

    if(m_pFirstChildRecord != 0)
    {
        m_pFirstChildRecord->checkCircularReference(pStartRecord);
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// dicomDir class
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

dicomDir::dicomDir(): m_pDataSet(std::make_shared<dataSet>("1.2.840.10008.1.2.1", charsetsList_t()))
{
    // Adjust the version
    /////////////////////
    {
        std::shared_ptr<handlers::writingDataHandlerRaw> versionHandler(m_pDataSet->getWritingDataHandlerRaw(0x2, 0, 0x1, 0, tagVR_t::OB));
        versionHandler->setSize(2);
        versionHandler->setUnsignedLong(0, 0);
        versionHandler->setUnsignedLong(1, 1);
    }

    // Adjust the SOP class UID
    ///////////////////////////
    m_pDataSet->setString(0x2, 0, 0x2, 0, "1.2.840.10008.1.3.10");

    charsetsList_t list;
    list.push_back("ISO 2022 IR 6");
    m_pDataSet->setCharsetsList(list);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
dicomDir::dicomDir(std::shared_ptr<dataSet> pDataSet):
    m_pDataSet(pDataSet)
{
    IMEBRA_FUNCTION_START();

    if(m_pDataSet.get() == 0)
    {
        m_pDataSet = std::make_shared<dataSet>("1.2.840.10008.1.2.1", charsetsList_t());
    }

    // Get the DICOMDIR sequence
    ///////////////////////////////////////////////////////////
    typedef std::map<std::uint32_t, std::shared_ptr<directoryRecord> > tOffsetsToRecords;
    tOffsetsToRecords offsetsToRecords;
    for(std::uint32_t scanItems(0); ; ++scanItems)
    {
        try
        {
            std::shared_ptr<dataSet> pDataSet(m_pDataSet->getSequenceItem(0x0004, 0, 0x1220, scanItems));
            std::shared_ptr<directoryRecord> newRecord(std::make_shared<directoryRecord>(pDataSet));
            offsetsToRecords[pDataSet->getItemOffset()] = newRecord;
            m_recordsList.push_back(newRecord);
        }
        catch(const MissingDataElementError&)
        {
            break; // Out of sequence items
        }
    }

    // Scan all the records and update the pointers
    ///////////////////////////////////////////////////////////
    for(tOffsetsToRecords::iterator scanRecords(offsetsToRecords.begin()); scanRecords != offsetsToRecords.end(); ++scanRecords)
    {
        try
        {
            std::uint32_t nextRecordOffset(scanRecords->second->getRecordDataSet()->getUnsignedLong(0x0004, 0, 0x1400, 0, 0));
            tOffsetsToRecords::iterator findNextRecord(offsetsToRecords.find(nextRecordOffset));
            if(findNextRecord != offsetsToRecords.end())
            {
                scanRecords->second->setNextRecord(findNextRecord->second);
            }
        }
        catch(const MissingDataElementError&)
        {
            // Nothing to do
        }

        try
        {
            std::uint32_t childRecordOffset(scanRecords->second->getRecordDataSet()->getUnsignedLong(0x0004, 0, 0x1420, 0, 0));
            tOffsetsToRecords::iterator findChildRecord(offsetsToRecords.find(childRecordOffset));
            if(findChildRecord != offsetsToRecords.end())
            {
                scanRecords->second->setFirstChildRecord(findChildRecord->second);
            }
        }
        catch(const MissingDataElementError&)
        {
            // Nothing to do
        }
    }

    // Get the position of the first record
    ///////////////////////////////////////////////////////////
    try
    {
        tOffsetsToRecords::iterator findRecord(offsetsToRecords.find(m_pDataSet->getUnsignedLong(0x0004, 0, 0x1200, 0, 0)));
        if(findRecord == offsetsToRecords.end())
        {
            setFirstRootRecord(std::shared_ptr<directoryRecord>());
        }
        else
        {
            setFirstRootRecord(findRecord->second);
        }
    }
    catch(const MissingDataElementError&)
    {
        // Nothing to do
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Destructor, make sure that the records are released
//  after the root record.
// This resolves a stack overflow when a large number
//  sibling records is present
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
dicomDir::~dicomDir()
{
    m_pFirstRootRecord.reset();
    m_recordsList.clear();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the dataSet
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<dataSet> dicomDir::getDirectoryDataSet() const
{
    return m_pDataSet;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns a new record that can be inserted in the
//  folder
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<directoryRecord> dicomDir::getNewRecord(const std::string& recordType)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<data> recordsTag(m_pDataSet->getTagCreate(0x0004, 0, 0x1220));
    std::shared_ptr<dataSet> recordDataSet = recordsTag->appendSequenceItem();
    std::shared_ptr<directoryRecord> newRecord(std::make_shared<directoryRecord>(recordDataSet));
    newRecord->setTypeString(recordType);
    m_recordsList.push_back(newRecord);

    return newRecord;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Build the a dicom dataset that contain the directory's
//  information.
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<dataSet> dicomDir::buildDataSet()
{
    IMEBRA_FUNCTION_START();

    // Allocate offset fields
    ///////////////////////////////////////////////////////////
    if(m_pFirstRootRecord != 0)
    {
        m_pFirstRootRecord->updateOffsets();
    }
    m_pDataSet->setUnsignedLong(0x0004, 0, 0x1200, 0, 0);


    // Save to a null stream in order to update the offsets
    ///////////////////////////////////////////////////////////
    std::shared_ptr<nullStreamWriter> saveStream(std::make_shared<nullStreamWriter>());
    std::shared_ptr<streamWriter> writer(std::make_shared<streamWriter>(saveStream));
    std::shared_ptr<codecs::dicomStreamCodec> writerCodec(std::make_shared<codecs::dicomStreamCodec>());
    writerCodec->write(writer, m_pDataSet);

    // Scan all the records and update the pointers
    ///////////////////////////////////////////////////////////
    if(m_pFirstRootRecord != 0)
    {
        m_pFirstRootRecord->updateOffsets();
        m_pDataSet->setUnsignedLong(0x0004, 0, 0x1200, 0, m_pFirstRootRecord->getRecordDataSet()->getItemOffset());
    }

    return m_pDataSet;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the first root record
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<directoryRecord> dicomDir::getFirstRootRecord() const
{
    return m_pFirstRootRecord;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the first root record
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomDir::setFirstRootRecord(std::shared_ptr<directoryRecord> pFirstRootRecord)
{
    m_pFirstRootRecord = pFirstRootRecord;
}



} // namespace implementation

} // namespace imebra
