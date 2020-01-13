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

#include "../include/imebra/writingDataHandler.h"
#include "../include/imebra/date.h"
#include "../include/imebra/age.h"
#include "../include/imebra/patientName.h"
#include "../implementation/dataHandlerImpl.h"
#include "../implementation/dataHandlerNumericImpl.h"
#include <cstring>

namespace imebra
{

WritingDataHandler::~WritingDataHandler()
{
}

WritingDataHandler::WritingDataHandler(const std::shared_ptr<implementation::handlers::writingDataHandler>& pDataHandler): m_pDataHandler(pDataHandler)
{}

WritingDataHandler::WritingDataHandler(const WritingDataHandler& source): m_pDataHandler(getWritingDataHandlerImplementation(source))
{
}

const std::shared_ptr<implementation::handlers::writingDataHandler>& getWritingDataHandlerImplementation(const WritingDataHandler& writingHandler)
{
    return writingHandler.m_pDataHandler;
}

void WritingDataHandler::setSize(size_t elementsNumber)
{
    IMEBRA_FUNCTION_START();

    m_pDataHandler->setSize(elementsNumber);

    IMEBRA_FUNCTION_END_LOG();
}

size_t WritingDataHandler::getSize() const
{
    IMEBRA_FUNCTION_START();

    return m_pDataHandler->getSize();

    IMEBRA_FUNCTION_END_LOG();
}

tagVR_t WritingDataHandler::getDataType() const
{
    IMEBRA_FUNCTION_START();

    return m_pDataHandler->getDataType();

    IMEBRA_FUNCTION_END_LOG();
}


void WritingDataHandler::setDate(size_t index, const Date& date)
{
    IMEBRA_FUNCTION_START();

    m_pDataHandler->setDate((std::uint32_t)index, getDateImplementation(date));

    IMEBRA_FUNCTION_END_LOG();
}

void WritingDataHandler::setAge(size_t index, const Age& age)
{
    IMEBRA_FUNCTION_START();

    m_pDataHandler->setAge(index, getAgeImplementation(age));

    IMEBRA_FUNCTION_END_LOG();
}

void WritingDataHandler::setSignedLong(size_t index, std::int32_t value)
{
    IMEBRA_FUNCTION_START();

    m_pDataHandler->setSignedLong(index, value);

    IMEBRA_FUNCTION_END_LOG();
}

void WritingDataHandler::setUnsignedLong(size_t index, std::uint32_t value)
{
    IMEBRA_FUNCTION_START();

    m_pDataHandler->setUnsignedLong(index, value);

    IMEBRA_FUNCTION_END_LOG();
}

void WritingDataHandler::setDouble(size_t index, double value)
{
    IMEBRA_FUNCTION_START();

    m_pDataHandler->setDouble(index, value);

    IMEBRA_FUNCTION_END_LOG();
}

void WritingDataHandler::setString(size_t index, const std::string& value)
{
    IMEBRA_FUNCTION_START();

    m_pDataHandler->setString(index, value);

    IMEBRA_FUNCTION_END_LOG();
}

void WritingDataHandler::setUnicodeString(size_t index, const std::wstring& value)
{
    IMEBRA_FUNCTION_START();

    m_pDataHandler->setUnicodeString(index, value);

    IMEBRA_FUNCTION_END_LOG();
}

void WritingDataHandler::setPatientName(size_t index, const PatientName& patientName)
{
    IMEBRA_FUNCTION_START();

    m_pDataHandler->setPatientName(index, getPatientNameImplementation(patientName));

    IMEBRA_FUNCTION_END_LOG();
}

void WritingDataHandler::setUnicodePatientName(size_t index, const UnicodePatientName& patientName)
{
    IMEBRA_FUNCTION_START();

    m_pDataHandler->setUnicodePatientName(index, getUnicodePatientNameImplementation(patientName));

    IMEBRA_FUNCTION_END_LOG();
}

}
