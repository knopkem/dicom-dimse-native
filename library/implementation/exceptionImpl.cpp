/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file exception.cpp
    \brief Implementation of the exception classes.

*/

#include "exceptionImpl.h"
#include "charsetConversionImpl.h"

namespace imebra
{

namespace implementation
{

logTrace::logTrace(const std::string& functionName):
    m_functionName(functionName)
{
    IMEBRA_LOG_TRACE("Entering " << m_functionName);
}

logTrace::~logTrace()
{
    IMEBRA_LOG_TRACE("Exiting " << m_functionName);
}


///////////////////////////////////////////////////////////
// Return the message info for the current thread
///////////////////////////////////////////////////////////
std::string exceptionsManager::getMessage()
{
	tExceptionInfoList infoList;
	exceptionsManager::getExceptionInfo(&infoList);

    std::string message;
	for(tExceptionInfoList::iterator scanInfo = infoList.begin(); 
		scanInfo != infoList.end(); 
		++scanInfo)
	{
		message += scanInfo->getMessage();
        message += "\n\n";
	}

	return message;
}


///////////////////////////////////////////////////////////
// Return the info objects for the specified thread
///////////////////////////////////////////////////////////
void exceptionsManager::getExceptionInfo(tExceptionInfoList* pList)
{
    for(tExceptionInfoList::iterator scanInformation = m_information.begin();
        scanInformation != m_information.end();
		++scanInformation)
	{
		pList->push_back(*scanInformation);
	}
    m_information.clear();
}


///////////////////////////////////////////////////////////
// Add an info object to the current thread
///////////////////////////////////////////////////////////
void exceptionsManager::startExceptionInfo(const exceptionInfo& info)
{
    m_information.clear();
    m_information.push_back(info);
}


///////////////////////////////////////////////////////////
// Add an info object to the current thread
///////////////////////////////////////////////////////////
void exceptionsManager::addExceptionInfo(const exceptionInfo& info)
{
    m_information.push_back(info);
}


///////////////////////////////////////////////////////////
// Construct the exceptionInfo object
///////////////////////////////////////////////////////////
exceptionInfo::exceptionInfo(const std::string& functionName, const std::string& fileName, const long lineNumber, const std::string& exceptionType, const std::string& exceptionMessage):
	m_functionName(functionName), 
	m_fileName(fileName),
	m_lineNumber(lineNumber),
	m_exceptionType(exceptionType),
	m_exceptionMessage(exceptionMessage)
{
}

///////////////////////////////////////////////////////////
// Copy constructor
///////////////////////////////////////////////////////////
exceptionInfo::exceptionInfo(const exceptionInfo& right):
			m_functionName(right.m_functionName), 
			m_fileName(right.m_fileName),
			m_lineNumber(right.m_lineNumber),
			m_exceptionType(right.m_exceptionType),
			m_exceptionMessage(right.m_exceptionMessage)
{}

///////////////////////////////////////////////////////////
// Return the exceptionInfo content in a string
///////////////////////////////////////////////////////////
std::string exceptionInfo::getMessage()
{
    std::ostringstream message;
	message << "[" << m_functionName << "]" << "\n";
    message << " file: " << m_fileName << "  line: " << m_lineNumber << "\n";
    message << " exception type: " << m_exceptionType << "\n";
    message << " exception message: " << m_exceptionMessage << "\n";
	return message.str();
}



exceptionsManagerGetter::exceptionsManagerGetter()
{
#ifdef __APPLE__
    ::pthread_key_create(&m_key, &exceptionsManagerGetter::deleteExceptionsManager);
#endif
}

exceptionsManagerGetter::~exceptionsManagerGetter()
{
#ifdef __APPLE__
    ::pthread_key_delete(m_key);
#endif
}

exceptionsManagerGetter& exceptionsManagerGetter::getExceptionsManagerGetter()
{
    static exceptionsManagerGetter getter;
    return getter;
}

#ifndef __APPLE__
thread_local std::unique_ptr<exceptionsManager> exceptionsManagerGetter::m_pManager = std::unique_ptr<exceptionsManager>();
#endif

exceptionsManager& exceptionsManagerGetter::getExceptionsManager()
{
    IMEBRA_FUNCTION_START();

#ifdef __APPLE__
    exceptionsManager* pManager = (exceptionsManager*)pthread_getspecific(m_key);
    if(pManager == 0)
    {
        pManager = new exceptionsManager();
        pthread_setspecific(m_key, pManager);
    }
    return *pManager;
#else
    if(m_pManager.get() == 0)
    {
        m_pManager.reset(new exceptionsManager());
    }
    return *(m_pManager.get());
#endif

    IMEBRA_FUNCTION_END();
}

#ifdef __APPLE__
void exceptionsManagerGetter::deleteExceptionsManager(void* pManager)
{
    delete (exceptionsManager*)pManager;
}
#endif

} // namespace implementation

} // namespace imebra
