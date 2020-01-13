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
    \brief Implementation of the classes that parse/create a DICOMDIR
        structure (DicomDir and DicomDirEntry).

*/

#include "../include/imebra/dicomDirEntry.h"
#include "../include/imebra/dataSet.h"
#include "../implementation/dicomDirImpl.h"
#include "../implementation/exceptionImpl.h"
#include "../include/imebra/exceptions.h"

namespace imebra
{

DicomDirEntry::DicomDirEntry(const DicomDirEntry &source): m_pDirectoryRecord(getDicomDirEntryImplementation(source))
{
}

DicomDirEntry::DicomDirEntry(const std::shared_ptr<imebra::implementation::directoryRecord>& pDirectoryRecord): m_pDirectoryRecord(pDirectoryRecord)
{
}

const std::shared_ptr<implementation::directoryRecord>& getDicomDirEntryImplementation(const DicomDirEntry& dicomDirEntry)
{
    return dicomDirEntry.m_pDirectoryRecord;
}

DicomDirEntry::~DicomDirEntry()
{
}

DataSet DicomDirEntry::getEntryDataSet() const
{
    IMEBRA_FUNCTION_START();

    return DataSet(m_pDirectoryRecord->getRecordDataSet());

    IMEBRA_FUNCTION_END_LOG();
}

bool DicomDirEntry::hasNextEntry() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::directoryRecord> pNextEntry(m_pDirectoryRecord->getNextRecord());
    return pNextEntry != 0;

    IMEBRA_FUNCTION_END_LOG();
}

DicomDirEntry DicomDirEntry::getNextEntry() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::directoryRecord> pNextEntry(m_pDirectoryRecord->getNextRecord());
    if(pNextEntry == 0)
    {
        throw DicomDirNoEntryError("Missing sibling entry");;
    }
    return DicomDirEntry(pNextEntry);

    IMEBRA_FUNCTION_END_LOG();
}

bool DicomDirEntry::hasChildren() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::directoryRecord> pChildEntry(m_pDirectoryRecord->getFirstChildRecord());
    return pChildEntry != 0;

    IMEBRA_FUNCTION_END_LOG();
}

DicomDirEntry DicomDirEntry::getFirstChildEntry() const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<implementation::directoryRecord> pChildEntry(m_pDirectoryRecord->getFirstChildRecord());
    if(pChildEntry == 0)
    {
        throw DicomDirNoEntryError("There isn't any child entry");
    }
    return DicomDirEntry(pChildEntry);

    IMEBRA_FUNCTION_END_LOG();
}

fileParts_t DicomDirEntry::getFileParts() const
{
    IMEBRA_FUNCTION_START();

    return m_pDirectoryRecord->getFileParts();

    IMEBRA_FUNCTION_END_LOG();
}

std::string DicomDirEntry::getTypeString() const
{
    IMEBRA_FUNCTION_START();

    return m_pDirectoryRecord->getTypeString();

    IMEBRA_FUNCTION_END_LOG();
}

MutableDicomDirEntry::MutableDicomDirEntry(const MutableDicomDirEntry &source): DicomDirEntry(getDicomDirEntryImplementation(source))
{
}

MutableDicomDirEntry::~MutableDicomDirEntry()
{
}

MutableDicomDirEntry::MutableDicomDirEntry(const std::shared_ptr<imebra::implementation::directoryRecord>& pDirectoryRecord): DicomDirEntry(pDirectoryRecord)
{
}

MutableDataSet MutableDicomDirEntry::getEntryDataSet()
{
    IMEBRA_FUNCTION_START();

    return MutableDataSet(getDicomDirEntryImplementation(*this)->getRecordDataSet());

    IMEBRA_FUNCTION_END_LOG();

}

void MutableDicomDirEntry::setNextEntry(const DicomDirEntry& nextEntry)
{
    IMEBRA_FUNCTION_START();

    getDicomDirEntryImplementation(*this)->setNextRecord(getDicomDirEntryImplementation(nextEntry));

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDicomDirEntry::setFirstChildEntry(const DicomDirEntry& firstChildEntry)
{
    IMEBRA_FUNCTION_START();

    getDicomDirEntryImplementation(*this)->setFirstChildRecord(getDicomDirEntryImplementation(firstChildEntry));

    IMEBRA_FUNCTION_END_LOG();
}

void MutableDicomDirEntry::setFileParts(const fileParts_t& fileParts)
{
    IMEBRA_FUNCTION_START();

    getDicomDirEntryImplementation(*this)->setFileParts(fileParts);

    IMEBRA_FUNCTION_END_LOG();
}

}
