/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerStringPN.cpp
    \brief Implementation of the class dataHandlerStringPN.

*/

#include "dataHandlerStringPNImpl.h"
#include "patientNameImpl.h"
#include "../include/imebra/exceptions.h"

namespace imebra
{

namespace implementation
{

namespace handlers
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// dataHandlerStringPN
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

readingDataHandlerStringPN::readingDataHandlerStringPN(const memory& parseMemory, const std::shared_ptr<const charsetsList_t>& pCharsets):
    readingDataHandlerStringUnicode(parseMemory, pCharsets, tagVR_t::PN, L'\\', 0x20)
{
}


std::shared_ptr<patientName> readingDataHandlerStringPN::getPatientName(const size_t index) const
{
    return returnPatientName<std::string, patientName, char>(getString(index), '=');
}

std::shared_ptr<unicodePatientName> readingDataHandlerStringPN::getUnicodePatientName(const size_t index) const
{
    return returnPatientName<std::wstring, unicodePatientName, wchar_t>(getUnicodeString(index), L'=');
}



writingDataHandlerStringPN::writingDataHandlerStringPN(const std::shared_ptr<buffer>& pBuffer, const std::shared_ptr<const charsetsList_t>& pCharsets):
    writingDataHandlerStringUnicode(pBuffer, pCharsets, tagVR_t::PN, L'\\', 0, 0)
{
}

void writingDataHandlerStringPN::setPatientName(const size_t index, const std::shared_ptr<const patientName>& pPatientName)
{
    setString(index, returnPatientName<std::string, patientName, char>(pPatientName, '='));
}

void writingDataHandlerStringPN::setUnicodePatientName(const size_t index, const std::shared_ptr<const unicodePatientName>& pPatientName)
{
    setUnicodeString(index, returnPatientName<std::wstring, unicodePatientName, wchar_t>(pPatientName, L'='));
}


} // namespace handlers

} // namespace implementation

} // namespace imebra
