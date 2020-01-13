/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file exception.h
    \brief Defines the classes and the macros used to handle the
            exceptions in the library.

*/

#if !defined(CImbxException_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_)
#define CImbxException_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_

#include "../include/imebra/definitions.h"
#include "configurationImpl.h"
#include "logging.h"
#include <memory>

#include <typeinfo>
#include <exception>
#include <stdexcept>
#include <map>
#include <list>
#include <string>
#include <sstream>
#include <thread>

namespace imebra
{

namespace implementation
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class is used to store the information
///         related to the points where the exceptions are
///         being caught&thrown.
///
/// The application can use the macros
///  IMEBRA_FUNCTION_START(),
///  IMEBRA_FUNCTION_END(),
///  IMEBRA_THROW() and
///  IMEBRA_RETHROW()
///  inside its function: the macros take care of logging
///  in the exceptions manager the source code's lines
///  travelled by an exception while it is being thrown
///  to the catch point.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class exceptionInfo
{
public:
    /// \brief Construct the exceptionInfo object.
    ///
    /// @param functionName  the name of the function where
    ///                       the catch&throw is happening
    /// @param fileName      the name of the file where the
    ///                       catch&throw is happening
    /// @param lineNumber    the line where the catch&throw
    ///                       is happening
    /// @param exceptionType the type of the exception being
    ///                       rethrown
    /// @param exceptionMessage the message embedded in the
    ///                       exception being rethrown
    ///
    ///////////////////////////////////////////////////////////
    exceptionInfo(const std::string& functionName, const std::string& fileName, const long lineNumber, const std::string& exceptionType, const std::string& exceptionMessage);

    exceptionInfo(const exceptionInfo& right);

    /// \brief Return the object's content formatted in
    ///         a string.
    ///
    /// @return the object's content formatted in a string
    ///
    ///////////////////////////////////////////////////////////
    std::string getMessage();

    std::string m_functionName;   ///< The name of the function where the catch&throw is happening
    std::string m_fileName;        ///< The name of the file where the catch&throw is happening
    long m_lineNumber;             ///< The number of the line where the catch&throw is happening
    std::string m_exceptionType;   ///< The type of the exception being rethrown
    std::string m_exceptionMessage;///< The message embedded in the exception
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief One statically allocated instance of this class
///         logs the stack positions while an exception is
///         being processed.
///
/// In order to log the stack position the application
///  must use the following macros inside its functions:
/// - IMEBRA_FUNCTION_START()
/// - IMEBRA_FUNCTION_END()
/// - IMEBRA_THROW()
///
/// The final catch block that processes the exception and
///  doesn't rethrow it should call
///  exceptionsManager::getMessage() in order to
///  retrieve the exception's history.
///
/// The retrieved message will contain the position of the
///  first throw statement and the positions of the
///  IMEBRA_FUNCTION_END() macros that rethrown the
///  exception.
///
/// All the catch blocks that don't rethrow the catched
///  exception and solve the exception's cause should call
///  exceptionsManager::getMessage() to empty the
///  messages stack.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class exceptionsManager
{
public:
    void startExceptionInfo(const exceptionInfo& info);

    /// \brief Add an exceptionInfo object to the active
    ///         thread's information list.
    ///
    /// This function is called by IMEBRA_FUNCTION_END() when
    ///  an uncaught exception is found.
    ///
    /// @param info the info object that must be added
    ///         to the active thread's information list
    ///
    ///////////////////////////////////////////////////////////
    void addExceptionInfo(const exceptionInfo& info);

    /// \brief Returns the same information returned by
    ///         getExceptionInfo(), but formatted in a text
    ///         message.
    ///
    /// The function also clears the information list for
    ///  the current thread.
    ///
    /// @return the information formatted as a text message
    ///
    ///////////////////////////////////////////////////////////
    std::string getMessage();

    /// \brief Defines a list of exceptionInfo objects.
    ///
    ///////////////////////////////////////////////////////////
    typedef std::list<exceptionInfo> tExceptionInfoList;

    /// \brief Fill the list with all the collected
    ///         exceptionInfo objects for the current thread.
    ///
    /// The function also clears the information list of
    ///  the current thread.
    ///
    /// @param pList a pointer to the list that will be filled
    ///               with a copy of the exceptionInfo objects
    ///               collected before the call.
    ///
    ///////////////////////////////////////////////////////////
    void getExceptionInfo(tExceptionInfoList* pList);

protected:
    tExceptionInfoList m_information;
};


class exceptionsManagerGetter
{
protected:
    exceptionsManagerGetter();
    ~exceptionsManagerGetter();

public:
    static exceptionsManagerGetter& getExceptionsManagerGetter();

    exceptionsManager& getExceptionsManager();

protected:

#ifdef __APPLE__
    static void deleteExceptionsManager(void* pMemoryPool);
    pthread_key_t m_key;
#endif

#ifndef __APPLE__
    thread_local static std::unique_ptr<exceptionsManager> m_pManager;
#endif
};



/// \def IMEBRA_FUNCTION_START(functionName)
///
/// \brief Initialize a try block. The try block must be
///         matched by a call to the
///         IMEBRA_FUNCTION_END() macro.
///
/// This macro should be placed at the very beginning
///  of a function.
///
/// All the exceptions not catched by the body of the
///  function are catched by IMEBRA_FUNCTION_END() and
///  rethrown, but before being rethrown the function's
///  name and the line number in the source file are
///  logged.
/// All the positions of the IMEBRA_FUNCTION_END() that
///  catch the same exception are logged togheter.
///
/// Exceptions thrown inside the function should be thrown
///  by IMEBRA_THROW(), but this is not necessary.
///
/// @param functionName the name of the function in which
///         the macro is placed.
///
///////////////////////////////////////////////////////////
#ifdef _MSC_VER
#define IMEBRA_METHOD_NAME() __FUNCTION__
#else
#define IMEBRA_METHOD_NAME() __PRETTY_FUNCTION__
#endif

///
/// \brief
///
/// Used in IMEBRA_FUNCTION_START to force to logging of
/// the entering and exiting events (trace log).
///
///////////////////////////////////////////////////////////
class logTrace
{
public:
    /// Constructor.
    ///
    /// Logs the function entering event.
    ///
    /// \param functionName the name of the function logging
    ///                     the trace events
    ///
    ///////////////////////////////////////////////////////////
    logTrace(const std::string& functionName);
    ~logTrace();

private:
    const std::string m_functionName;
};

#if IMEBRA_LOG_LEVEL >= IMEBRA_LOG_LEVEL_TRACE_VALUE
#define IMEBRA_FUNCTION_START() \
    try{ \
       const ::imebra::implementation::logTrace trace(IMEBRA_METHOD_NAME());
#else
#define IMEBRA_FUNCTION_START() \
    try{
#endif

/// \def IMEBRA_FUNCTION_END()
///
/// \brief Insert a catch block that rethrows the catched
///         exception and log the function's name and
///         the position in the source file in which the
///         exception has been catched and rethrown.
///
/// This function must be placed at the end of a function
///  if the IMEBRA_FUNCTION_START() has been used in
///  the function.
///
///////////////////////////////////////////////////////////
#define IMEBRA_FUNCTION_END() \
    }\
    catch(std::exception& e)\
    {\
        imebra::implementation::exceptionInfo info(IMEBRA_METHOD_NAME(), __FILE__, __LINE__, typeid(e).name(), e.what());\
        imebra::implementation::exceptionsManagerGetter::getExceptionsManagerGetter().getExceptionsManager().addExceptionInfo(info);\
        throw;\
    }\
    catch(...)\
    {\
        imebra::implementation::exceptionInfo info(IMEBRA_METHOD_NAME(), __FILE__, __LINE__, "unknown", "");\
        imebra::implementation::exceptionsManagerGetter::getExceptionsManagerGetter().getExceptionsManager().addExceptionInfo(info);\
        throw;\
    }

/// \def IMEBRA_FUNCTION_END_LOG()
///
/// \brief Insert a catch block that rethrows the catched
///         exception and log the function's name and
///         the position in the source file in which the
///         exception has been catched and rethrown.
///
/// The exception is also sent to the system log as
/// error.
///
/// This function must be placed at the end of a function
///  if the IMEBRA_FUNCTION_START() has been used in
///  the function.
///
///////////////////////////////////////////////////////////
#define IMEBRA_FUNCTION_END_LOG() \
    }\
    catch(std::exception& e)\
    {\
        imebra::implementation::exceptionInfo info(IMEBRA_METHOD_NAME(), __FILE__, __LINE__, typeid(e).name(), e.what());\
        imebra::implementation::exceptionsManagerGetter::getExceptionsManagerGetter().getExceptionsManager().addExceptionInfo(info);\
        IMEBRA_LOG_ERROR("Throwing exception " << typeid(e).name() << " with message \"" << e.what() << "\" in function " << IMEBRA_METHOD_NAME() << " line " << __LINE__ << " file " << __FILE__);\
        throw;\
    }\
    catch(...)\
    {\
        imebra::implementation::exceptionInfo info(IMEBRA_METHOD_NAME(), __FILE__, __LINE__, "unknown", "");\
        imebra::implementation::exceptionsManagerGetter::getExceptionsManagerGetter().getExceptionsManager().addExceptionInfo(info);\
        IMEBRA_LOG_ERROR("Throwing unknown exception in function " << IMEBRA_METHOD_NAME() << " line " << __LINE__ << " file " << __FILE__);\
        throw;\
    }

/// \def IMEBRA_FUNCTION_END_MODIFY()
///
/// \brief Insert a catch block that rethrows the catched
///         exception and log the function's name and
///         the position in the source file in which the
///         exception has been catched and rethrown.
///        Exception of the type specified in the first
///         parameter are replaced by the type specified
///         in the second parameter.
///
/// @param catchType the exception type that must be
///                  caught and replace by the exception in
///                  throwType
/// @param throwType the exception type to throw instead
///                  of catchType
///
///////////////////////////////////////////////////////////
#define IMEBRA_FUNCTION_END_MODIFY(catchType, throwType) \
    }\
    catch(catchType& e)\
    {\
        imebra::implementation::exceptionInfo info(IMEBRA_METHOD_NAME(), __FILE__, __LINE__, typeid(e).name(), e.what());\
        imebra::implementation::exceptionsManagerGetter::getExceptionsManagerGetter().getExceptionsManager().addExceptionInfo(info);\
        IMEBRA_THROW(throwType, e.what());\
    }\
    catch(std::exception& e)\
    {\
        imebra::implementation::exceptionInfo info(IMEBRA_METHOD_NAME(), __FILE__, __LINE__, typeid(e).name(), e.what());\
        imebra::implementation::exceptionsManagerGetter::getExceptionsManagerGetter().getExceptionsManager().addExceptionInfo(info);\
        throw;\
    }\
    catch(...)\
    {\
        imebra::implementation::exceptionInfo info(IMEBRA_METHOD_NAME(), __FILE__, __LINE__, "unknown", "");\
        imebra::implementation::exceptionsManagerGetter::getExceptionsManagerGetter().getExceptionsManager().addExceptionInfo(info);\
        throw;\
    }

/// \def IMEBRA_THROW(exceptionType, what)
///
/// \brief Throw an exception of the specified type and log
///         the function's name and the position in the
///         file on which the exception has been thrown.
///
/// This macro can be used only in the functions or blocks
///  that use the macros IMEBRA_FUNCTION_START() and
///  IMEBRA_FUNCTION_END().
///
/// @param exceptionType the type of exception to throw
/// @param what          a message to be associated with
///                       the exception
///
///////////////////////////////////////////////////////////
#define IMEBRA_THROW(exceptionType, message) \
    {\
        std::ostringstream buildMessage; \
        buildMessage << message; \
        exceptionType imebraTrackException(buildMessage.str());\
        imebra::implementation::exceptionInfo info(IMEBRA_METHOD_NAME(), __FILE__, __LINE__, typeid(imebraTrackException).name(), imebraTrackException.what());\
        imebra::implementation::exceptionsManagerGetter::getExceptionsManagerGetter().getExceptionsManager().startExceptionInfo(info);\
        throw imebraTrackException;\
    }

#define IMEBRA_THROW_ADDITIONAL_PARAM(exceptionType, message, additional) \
    {\
        std::ostringstream buildMessage; \
        buildMessage << message; \
        exceptionType imebraTrackException(buildMessage.str(), additional);\
        imebra::implementation::exceptionInfo info(IMEBRA_METHOD_NAME(), __FILE__, __LINE__, typeid(imebraTrackException).name(), imebraTrackException.what());\
        imebra::implementation::exceptionsManagerGetter::getExceptionsManagerGetter().getExceptionsManager().startExceptionInfo(info);\
        throw imebraTrackException;\
    }

/// \def IMEBRA_RETHROW(what)
///
/// \brief Rethrow an exception caught by a catch block
///         and add some descriptions to it.
///
/// This macro can be used only in the functions or blocks
///  that use the macros IMEBRA_FUNCTION_START() and
///  IMEBRA_FUNCTION_END().
///
/// @param what          a message to be associated with
///                       the exception
///
///////////////////////////////////////////////////////////
#define IMEBRA_RETHROW(what) \
    {\
        imebra::implementation::exceptionInfo info(IMEBRA_METHOD_NAME(), __FILE__, __LINE__, "rethrowing", what);\
        imebra::implementation::exceptionsManagerGetter::getExceptionsManagerGetter().getExceptionsManager().addExceptionInfo(info);\
        throw;\
    }

///@}

} // namespace implementation

} // namespace imebra


#endif // !defined(CImbxException_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_)
