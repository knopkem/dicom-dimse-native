/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dicomDictionary.cpp
    \brief Implementation of the class DicomDictionary class.

*/

#include "../include/imebra/dicomDictionary.h"
#include "../include/imebra/definitions.h"
#include "../include/imebra/tagId.h"
#include "../implementation/dicomDictImpl.h"
#include "../implementation/charsetConversionImpl.h"
#include "../implementation/exceptionImpl.h"
#include <memory>

namespace imebra
{

std::wstring DicomDictionary::getUnicodeTagDescription(const TagId& id)
{
    IMEBRA_FUNCTION_START();

    return implementation::dicomDictionary::getDicomDictionary()->getTagDescription(id.getGroupId(), id.getTagId());

    IMEBRA_FUNCTION_END_LOG();
}

std::string DicomDictionary::getTagDescription(const TagId& id)
{
    IMEBRA_FUNCTION_START();

    std::wstring name = implementation::dicomDictionary::getDicomDictionary()->getTagDescription(id.getGroupId(), id.getTagId());
    charsetsList_t charsets;
    charsets.push_back("ISO 2022 IR 6");
    return implementation::dicomConversion::convertFromUnicode(name, charsets);

    IMEBRA_FUNCTION_END_LOG();
}

tagVR_t DicomDictionary::getTagType(const TagId& id)
{
    IMEBRA_FUNCTION_START();

    return implementation::dicomDictionary::getDicomDictionary()->getTagType(id.getGroupId(), id.getTagId());

    IMEBRA_FUNCTION_END_LOG();
}

std::uint32_t DicomDictionary::getMultiplicityMin(const TagId& id)
{
    IMEBRA_FUNCTION_START();

    return implementation::dicomDictionary::getDicomDictionary()->getTagMultiplicityMin(id.getGroupId(), id.getTagId());

    IMEBRA_FUNCTION_END_LOG();
}

std::uint32_t DicomDictionary::getMultiplicityMax(const TagId& id)
{
    IMEBRA_FUNCTION_START();

    return implementation::dicomDictionary::getDicomDictionary()->getTagMultiplicityMax(id.getGroupId(), id.getTagId());

    IMEBRA_FUNCTION_END_LOG();
}

std::uint32_t DicomDictionary::getMultiplicityStep(const TagId& id)
{
    IMEBRA_FUNCTION_START();

    return implementation::dicomDictionary::getDicomDictionary()->getTagMultiplicityStep(id.getGroupId(), id.getTagId());

    IMEBRA_FUNCTION_END_LOG();
}

std::uint32_t DicomDictionary::getWordSize(tagVR_t dataType)
{
    IMEBRA_FUNCTION_START();

    return implementation::dicomDictionary::getDicomDictionary()->getWordSize(dataType);

    IMEBRA_FUNCTION_END_LOG();
}

std::uint32_t DicomDictionary::getMaxSize(tagVR_t dataType)
{
    IMEBRA_FUNCTION_START();

    return implementation::dicomDictionary::getDicomDictionary()->getMaxSize(dataType);

    IMEBRA_FUNCTION_END_LOG();
}

}

