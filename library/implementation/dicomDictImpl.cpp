/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dicomDict.cpp
    \brief Implementation of the class dicomDict.

*/


#include "exceptionImpl.h"
#include "dicomDictImpl.h"
#include "dicomDescriptions.h"
#include "../include/imebra/exceptions.h"

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
// dicomDictionary
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
// Constructor. Register all the known tags and VRs
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
dicomDictionary::dicomDictionary()
{
    IMEBRA_FUNCTION_START();

    registerVR(tagVR_t::AE, false, 0, 16);
    registerVR(tagVR_t::AS, false, 0, 0);
    registerVR(tagVR_t::AT, false, 4, 0);
    registerVR(tagVR_t::CS, false, 0, 16);
    registerVR(tagVR_t::DA, false, 0, 0);
    registerVR(tagVR_t::DS, false, 0, 16);
    registerVR(tagVR_t::DT, false, 0, 26);
    registerVR(tagVR_t::FL, false, 4, 0);
    registerVR(tagVR_t::FD, false, 8, 0);
    registerVR(tagVR_t::IS, false, 0, 12);
    registerVR(tagVR_t::LO, false, 0, 64);
    registerVR(tagVR_t::LT, false, 0, 10240);
    registerVR(tagVR_t::OB, true,  0, 0);
    registerVR(tagVR_t::SB, true,  0, 0); // Non standard. Used internally for signed bytes
    registerVR(tagVR_t::OD, true,  8, 0);
    registerVR(tagVR_t::OF, true,  4, 0);
    registerVR(tagVR_t::OL, true,  4, 0);
    registerVR(tagVR_t::OW, true,  2, 0);
    registerVR(tagVR_t::PN, false, 0, 64);
    registerVR(tagVR_t::SH, false, 0, 16);
    registerVR(tagVR_t::SL, false, 4, 0);
    registerVR(tagVR_t::SQ, true,  0, 0);
    registerVR(tagVR_t::SS, false, 2, 0);
    registerVR(tagVR_t::ST, false, 0, 1024);
    registerVR(tagVR_t::TM, false, 0, 16);
    registerVR(tagVR_t::UC, true, 0, 0);
    registerVR(tagVR_t::UI, false, 0, 64);
    registerVR(tagVR_t::UL, false, 4, 0);
    registerVR(tagVR_t::UN, true,  0, 0);
    registerVR(tagVR_t::UR, true, 0, 0);
    registerVR(tagVR_t::US, false, 2, 0);
    registerVR(tagVR_t::UT, true, 0, 0);

    for(size_t scanDescriptions(0); m_tagsDescription[scanDescriptions].m_tagId != (std::uint32_t)0xffffffff; ++scanDescriptions)
    {
        registerTag(m_tagsDescription[scanDescriptions].m_tagId,
                    m_tagsDescription[scanDescriptions].m_tagMask,
                    m_tagsDescription[scanDescriptions].m_tagDescription,
                    m_tagsDescription[scanDescriptions].m_tagKeyword,
                    m_tagsDescription[scanDescriptions].m_vr0, m_tagsDescription[scanDescriptions].m_vr1,
                    m_tagsDescription[scanDescriptions].m_multiplicityMin,
                    m_tagsDescription[scanDescriptions].m_multiplicityMax,
                    m_tagsDescription[scanDescriptions].m_multiplicityStep);

    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Register a tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomDictionary::registerTag(std::uint32_t tagId,
                                  std::uint32_t tagMask,
                                  const wchar_t* tagName,
                                  const char* tagKeyword,
                                  tagVR_t vr0, tagVR_t vr1,
                                  std::uint32_t multiplicityMin, std::uint32_t multiplicityMax, std::uint32_t multiplicityStep)
{
    IMEBRA_FUNCTION_START();

    std::uint32_t increaseValue = 1u;
    while(tagMask != 0xffffffff && (increaseValue & tagMask) != 0)
    {
        increaseValue <<= 1;
    }

    for(std::uint32_t registerTagId(tagId); (registerTagId & tagMask) == tagId; registerTagId += increaseValue)
    {
        imageDataDictionaryElement newElement;
        newElement.m_tagDescription = tagName;
        newElement.m_tagKeyword = tagKeyword;
        newElement.m_vr0 = vr0;
        newElement.m_vr1 = vr1;
        newElement.m_multiplicityMin = multiplicityMin;
        newElement.m_multiplicityMax = multiplicityMax;
        newElement.m_multiplicityStep = multiplicityStep;
        m_dicomDict[registerTagId] = newElement;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Register a VR
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomDictionary::registerVR(tagVR_t vr, bool bLongLength, std::uint32_t wordSize, std::uint32_t maxLength)
{
    IMEBRA_FUNCTION_START();

    if(m_vrDict.find(vr) != m_vrDict.end())
    {
        IMEBRA_THROW(std::logic_error, "VR registered twice");
    }
    validDataTypesStruct newElement;
    newElement.m_longLength = bLongLength;
    newElement.m_wordLength = wordSize;
    newElement.m_maxLength = maxLength;

    m_vrDict[vr] = newElement;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return an human readable description for the tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::wstring dicomDictionary::getTagDescription(std::uint16_t groupId, std::uint16_t tagId) const
{
    IMEBRA_FUNCTION_START();

    std::uint32_t tagDWordId=(((std::uint32_t)groupId)<<16) | (std::uint32_t)tagId;

    tDicomDictionary::const_iterator findIterator = m_dicomDict.find(tagDWordId);
    if(findIterator == m_dicomDict.end())
    {
        IMEBRA_THROW(DictionaryUnknownTagError, "Unknown tag " << std::hex << groupId << ", " << std::hex << tagId);
    }

    return findIterator->second.m_tagDescription;

    IMEBRA_FUNCTION_END();
}


std::uint32_t dicomDictionary::getTagMultiplicityMin(std::uint16_t groupId, std::uint16_t tagId) const
{
    IMEBRA_FUNCTION_START();

    std::uint32_t tagDWordId=(((std::uint32_t)groupId)<<16) | (std::uint32_t)tagId;

    tDicomDictionary::const_iterator findIterator = m_dicomDict.find(tagDWordId);
    if(findIterator == m_dicomDict.end())
    {
        IMEBRA_THROW(DictionaryUnknownTagError, "Unknown tag " << std::hex << groupId << ", " << std::hex << tagId);
    }

    return findIterator->second.m_multiplicityMin;

    IMEBRA_FUNCTION_END();
}


std::uint32_t dicomDictionary::getTagMultiplicityMax(std::uint16_t groupId, std::uint16_t tagId) const
{
    IMEBRA_FUNCTION_START();

    std::uint32_t tagDWordId=(((std::uint32_t)groupId)<<16) | (std::uint32_t)tagId;

    tDicomDictionary::const_iterator findIterator = m_dicomDict.find(tagDWordId);
    if(findIterator == m_dicomDict.end())
    {
        IMEBRA_THROW(DictionaryUnknownTagError, "Unknown tag " << std::hex << groupId << ", " << std::hex << tagId);
    }

    return findIterator->second.m_multiplicityMax;

    IMEBRA_FUNCTION_END();
}


std::uint32_t dicomDictionary::getTagMultiplicityStep(std::uint16_t groupId, std::uint16_t tagId) const
{
    IMEBRA_FUNCTION_START();

    std::uint32_t tagDWordId=(((std::uint32_t)groupId)<<16) | (std::uint32_t)tagId;

    tDicomDictionary::const_iterator findIterator = m_dicomDict.find(tagDWordId);
    if(findIterator == m_dicomDict.end())
    {
        IMEBRA_THROW(DictionaryUnknownTagError, "Unknown tag " << std::hex << groupId << ", " << std::hex << tagId);
    }

    return findIterator->second.m_multiplicityStep;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the default type for the specified tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
tagVR_t dicomDictionary::getTagType(std::uint16_t groupId, std::uint16_t tagId) const
{
    IMEBRA_FUNCTION_START();

    std::uint32_t tagDWordId=(((std::uint32_t)groupId)<<16) | (std::uint32_t)tagId;

    tDicomDictionary::const_iterator findIterator = m_dicomDict.find(tagDWordId);
    if(findIterator == m_dicomDict.end())
    {
        IMEBRA_THROW(DictionaryUnknownTagError, "Unknown tag " << std::hex << groupId << ", " << std::hex << tagId);
    }

    return findIterator->second.m_vr0;

    IMEBRA_FUNCTION_END();
}


bool dicomDictionary::isDataTypeValid(const std::string& dataType) const
{
    try
    {
        stringDataTypeToEnum(dataType);
        return true;
    }
    catch(const DictionaryUnknownDataTypeError&)
    {
        return false;
    }
}


tagVR_t dicomDictionary::stringDataTypeToEnum(const std::string& dataType) const
{
    std::uint16_t enumVR = MAKE_VR_ENUM(dataType);

    if(m_vrDict.find((tagVR_t)enumVR) == m_vrDict.end())
    {
        IMEBRA_THROW(DictionaryUnknownDataTypeError, "Unknown data type " << dataType);
    }

    return (tagVR_t)enumVR;
}


std::string dicomDictionary::enumDataTypeToString(tagVR_t dataType) const
{
    std::string returnType((size_t)2, ' ');
    returnType[0] = (char)(((std::uint16_t)dataType >> 8) & 0xff);
    returnType[1] = (char)((std::uint16_t)dataType & 0xff);

    return returnType;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return true if the specified data type must use a
//  long length descriptor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool dicomDictionary::getLongLength(tagVR_t dataType) const
{
    tVRDictionary::const_iterator findIterator = m_vrDict.find(dataType);

    if(findIterator == m_vrDict.end())
    {
        return false;
    }

    return findIterator->second.m_longLength;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the word size for the specified data type
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dicomDictionary::getWordSize(tagVR_t dataType) const
{
    IMEBRA_FUNCTION_START();

    tVRDictionary::const_iterator findIterator = m_vrDict.find(dataType);

    if(findIterator == m_vrDict.end())
    {
        IMEBRA_THROW(DictionaryUnknownDataTypeError, "Unregistered data type" << (std::uint16_t)dataType);
    }

    return findIterator->second.m_wordLength;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the max size in bytes for the specified data
//  type
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dicomDictionary::getMaxSize(tagVR_t dataType) const
{
    IMEBRA_FUNCTION_START();

    tVRDictionary::const_iterator findIterator = m_vrDict.find(dataType);

    if(findIterator == m_vrDict.end())
    {
        IMEBRA_THROW(DictionaryUnknownDataTypeError, "Unregistered data type " << (std::uint16_t)dataType);
    }

    return findIterator->second.m_maxLength;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return a pointer to the unique instance of
//  dicomDictionary
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
dicomDictionary* dicomDictionary::getDicomDictionary()
{
    IMEBRA_FUNCTION_START();

    static dicomDictionary m_imbxDicomDictionary;
    return &m_imbxDicomDictionary;

    IMEBRA_FUNCTION_END();
}


} // namespace implementation

} // namespace imebra
