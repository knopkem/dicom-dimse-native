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

#include "../include/imebra/readingDataHandler.h"
#include "../implementation/dataHandlerImpl.h"
#include "../implementation/dataHandlerNumericImpl.h"
#include "../implementation/exceptionImpl.h"
#include "../include/imebra/age.h"
#include "../include/imebra/patientName.h"
#include <cstring>

namespace imebra
{

ReadingDataHandler::ReadingDataHandler(const std::shared_ptr<implementation::handlers::readingDataHandler>& pDataHandler): m_pDataHandler(pDataHandler)
{
}

ReadingDataHandler::ReadingDataHandler(const ReadingDataHandler& source): m_pDataHandler(getReadingDataHandlerImplementation(source))
{
}

ReadingDataHandler::~ReadingDataHandler()
{
}

const std::shared_ptr<implementation::handlers::readingDataHandler>& getReadingDataHandlerImplementation(const ReadingDataHandler& readingDataHandler)
{
    return readingDataHandler.m_pDataHandler;
}

size_t ReadingDataHandler::getSize() const
{
    IMEBRA_FUNCTION_START();

    return m_pDataHandler->getSize();

    IMEBRA_FUNCTION_END_LOG();
}

tagVR_t ReadingDataHandler::getDataType() const
{
    IMEBRA_FUNCTION_START();

    return m_pDataHandler->getDataType();

    IMEBRA_FUNCTION_END_LOG();
}

std::int32_t ReadingDataHandler::getSignedLong(size_t index) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataHandler->getSignedLong(index);

    IMEBRA_FUNCTION_END_LOG();
}

std::uint32_t ReadingDataHandler::getUnsignedLong(size_t index) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataHandler->getUnsignedLong(index);

    IMEBRA_FUNCTION_END_LOG();
}

double ReadingDataHandler::getDouble(size_t index) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataHandler->getDouble(index);

    IMEBRA_FUNCTION_END_LOG();
}

std::string ReadingDataHandler::getString(size_t index) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataHandler->getString(index);

    IMEBRA_FUNCTION_END_LOG();
}

std::wstring ReadingDataHandler::getUnicodeString(size_t index) const
{
    IMEBRA_FUNCTION_START();

    return m_pDataHandler->getUnicodeString(index);

    IMEBRA_FUNCTION_END_LOG();
}

const Date ReadingDataHandler::getDate(size_t index) const
{
    IMEBRA_FUNCTION_START();

    return Date(m_pDataHandler->getDate(index));

    IMEBRA_FUNCTION_END_LOG();
}

const Age ReadingDataHandler::getAge(size_t index) const
{
    IMEBRA_FUNCTION_START();

    return Age(m_pDataHandler->getAge(index));

    IMEBRA_FUNCTION_END_LOG();
}

const PatientName ReadingDataHandler::getPatientName(size_t index) const
{
    IMEBRA_FUNCTION_START();

    return PatientName(m_pDataHandler->getPatientName(index));

    IMEBRA_FUNCTION_END_LOG();
}

const UnicodePatientName ReadingDataHandler::getUnicodePatientName(size_t index) const
{
    IMEBRA_FUNCTION_START();

    return UnicodePatientName(m_pDataHandler->getUnicodePatientName(index));

    IMEBRA_FUNCTION_END_LOG();
}



}
