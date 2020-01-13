/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file logging.h
    \brief Defines the classes and the macros used to handle logging.

*/

#if !defined(CImbxLogging_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_)
#define CImbxLogging_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_

#include "../include/imebra/definitions.h"
#include "configurationImpl.h"
#include <iostream>
#include <sstream>
#include <string>


// Logging levels
#define IMEBRA_LOG_LEVEL_DISABLED_VALUE 0
#define IMEBRA_LOG_LEVEL_FATAL_VALUE    1
#define IMEBRA_LOG_LEVEL_ERROR_VALUE    2
#define IMEBRA_LOG_LEVEL_WARNING_VALUE  3
#define IMEBRA_LOG_LEVEL_INFO_VALUE     4
#define IMEBRA_LOG_LEVEL_DEBUG_DEBUG    5
#define IMEBRA_LOG_LEVEL_TRACE_VALUE    6

// Define IMEBRA_LOG_LEVEL to one of the previous values
// to enable logging
#if !defined(IMEBRA_LOG_LEVEL)

#if defined(IMEBRA_LOG_LEVEL_TRACE)
#define IMEBRA_LOG_LEVEL IMEBRA_LOG_LEVEL_TRACE_VALUE
#elif defined(IMEBRA_LOG_LEVEL_DEBUG)
#define IMEBRA_LOG_LEVEL IMEBRA_LOG_LEVEL_DEBUG_VALUE
#elif defined(IMEBRA_LOG_LEVEL_INFO)
#define IMEBRA_LOG_LEVEL IMEBRA_LOG_LEVEL_INFO_VALUE
#elif defined(IMEBRA_LOG_LEVEL_WARNING)
#define IMEBRA_LOG_LEVEL IMEBRA_LOG_LEVEL_WARNING_VALUE
#elif defined(IMEBRA_LOG_LEVEL_ERROR)
#define IMEBRA_LOG_LEVEL IMEBRA_LOG_LEVEL_ERROR_VALUE
#elif defined(IMEBRA_LOG_LEVEL_FATAL)
#define IMEBRA_LOG_LEVEL IMEBRA_LOG_LEVEL_FATAL_VALUE
#else
#define IMEBRA_LOG_LEVEL IMEBRA_LOG_LEVEL_DISABLED_VALUE
#endif

#endif


#if !defined(IMEBRA_LOG_FATAL_IMPLEMENTATION)

#if !defined(IMEBRA_LOGGING_LOG4CXX) && !defined(IMEBRA_LOGGING_ANDROID) && !defined(IMEBRA_LOGGING_APPLE) && !defined(IMEBRA_LOGGING_SYSLOG) && !defined(IMEBRA_LOGGING_COUT)

  #if defined(IMEBRA_ANDROID)
    #define IMEBRA_LOGGING_ANDROID
    #if defined(NDEBUG)
      #undef IMEBRA_LOG_LEVEL
      #define IMEBRA_LOG_LEVEL IMEBRA_LOG_LEVEL_DISABLED_VALUE
    #endif
  #elif defined(__APPLE__)
    #define IMEBRA_LOGGING_APPLE
  #elif defined(IMEBRA_POSIX)
    #define IMEBRA_LOGGING_SYSLOG
  #else
    #define IMEBRA_LOGGING_COUT
  #endif

#endif

#if defined(IMEBRA_LOGGING_LOG4CXX)
  #include <log4cxx/logger.h>

  #define IMEBRA_LOG_FATAL_IMPLEMENTATION(message) LOG4CXX_FATAL(::log4cxx::Logger::getRootLogger(), message.c_str())
  #define IMEBRA_LOG_ERROR_IMPLEMENTATION(message) LOG4CXX_ERROR(::log4cxx::Logger::getRootLogger(), message.c_str())
  #define IMEBRA_LOG_WARNING_IMPLEMENTATION(message) LOG4CXX_WARN(::log4cxx::Logger::getRootLogger(), message.c_str())
  #define IMEBRA_LOG_INFO_IMPLEMENTATION(message) LOG4CXX_INFO(::log4cxx::Logger::getRootLogger(), message.c_str())
  #define IMEBRA_LOG_DEBUG_IMPLEMENTATION(message) LOG4CXX_DEBUG(::log4cxx::Logger::getRootLogger(), message.c_str())
  #define IMEBRA_LOG_TRACE_IMPLEMENTATION(message) LOG4CXX_TRACE(::log4cxx::Logger::getRootLogger(), message.c_str())

#elif defined(IMEBRA_LOGGING_ANDROID)
  #include <android/log.h>

  #define IMEBRA_LOG_FATAL_IMPLEMENTATION(message) ::__android_log_write(ANDROID_LOG_ASSERT, "Imebra", message.c_str())
  #define IMEBRA_LOG_ERROR_IMPLEMENTATION(message) ::__android_log_write(ANDROID_LOG_ERROR, "Imebra", message.c_str())
  #define IMEBRA_LOG_WARNING_IMPLEMENTATION(message) ::__android_log_write(ANDROID_LOG_WARN, "Imebra", message.c_str())
  #define IMEBRA_LOG_INFO_IMPLEMENTATION(message) ::__android_log_write(ANDROID_LOG_INFO, "Imebra", message.c_str())
  #define IMEBRA_LOG_DEBUG_IMPLEMENTATION(message) ::__android_log_write(ANDROID_LOG_DEBUG, "Imebra", message.c_str())
  #define IMEBRA_LOG_TRACE_IMPLEMENTATION(message) ::__android_log_write(ANDROID_LOG_VERBOSE, "Imebra", message.c_str())

#elif defined(IMEBRA_LOGGING_APPLE)

  namespace imebra
  {

  namespace implementation
  {
  void appleLog(const std::string& format, const std::string& message);
  } // namespace implementation

  } // namespace imebra

  #define IMEBRA_LOG_FATAL_IMPLEMENTATION(message) ::imebra::implementation::appleLog("FATAL: %@", message.c_str())
  #define IMEBRA_LOG_ERROR_IMPLEMENTATION(message) ::imebra::implementation::appleLog("ERROR: %@", message.c_str())
  #define IMEBRA_LOG_WARNING_IMPLEMENTATION(message) ::imebra::implementation::appleLog("WARNING: %@", message.c_str())
  #define IMEBRA_LOG_INFO_IMPLEMENTATION(message) ::imebra::implementation::appleLog("INFO: %@", message.c_str())
  #define IMEBRA_LOG_DEBUG_IMPLEMENTATION(message) ::imebra::implementation::appleLog("DEBUG: %@", message.c_str())
  #define IMEBRA_LOG_TRACE_IMPLEMENTATION(message) ::imebra::implementation::appleLog("TRACE: %@", message.c_str())

#elif defined(IMEBRA_LOGGING_SYSLOG)
  #include <syslog.h>

  #define IMEBRA_LOG_FATAL_IMPLEMENTATION(message) ::syslog(LOG_CRIT, "%s", message.c_str())
  #define IMEBRA_LOG_ERROR_IMPLEMENTATION(message) ::syslog(LOG_ERR, "%s", message.c_str())
  #define IMEBRA_LOG_WARNING_IMPLEMENTATION(message) ::syslog(LOG_WARNING, "%s", message.c_str())
  #define IMEBRA_LOG_INFO_IMPLEMENTATION(message) ::syslog(LOG_INFO, "%s", message.c_str())
  #define IMEBRA_LOG_DEBUG_IMPLEMENTATION(message) ::syslog(LOG_DEBUG, "%s", message.c_str())
  #define IMEBRA_LOG_TRACE_IMPLEMENTATION(message) ::syslog(LOG_DEBUG, "%s", message.c_str())

#else

  namespace imebra
  {

  namespace implementation
  {

  ///
  /// \brief Returns a string with the current time.
  ///
  /// \return the current time
  ///
  ///////////////////////////////////////////////////////////
  std::string getCurrentTime();

  } // namespace implementation

  } // namespace imebra

  #define IMEBRA_LOG_FATAL_IMPLEMENTATION(message) std::cout << "Imebra FATAL " << ::imebra::implementation::getCurrentTime() << " " << message << std::endl;
  #define IMEBRA_LOG_ERROR_IMPLEMENTATION(message) std::cout << "Imebra ERROR " << ::imebra::implementation::getCurrentTime() << " " << message << std::endl;
  #define IMEBRA_LOG_WARNING_IMPLEMENTATION(message) std::cout << "Imebra WARNING " << ::imebra::implementation::getCurrentTime() << " " << message << std::endl;
  #define IMEBRA_LOG_INFO_IMPLEMENTATION(message) std::cout << "Imebra INFO " << ::imebra::implementation::getCurrentTime() << " " << message << std::endl;
  #define IMEBRA_LOG_DEBUG_IMPLEMENTATION(message) std::cout << "Imebra DEBUG " << ::imebra::implementation::getCurrentTime() << " " << message << std::endl;
  #define IMEBRA_LOG_TRACE_IMPLEMENTATION(message) std::cout << "Imebra TRACE " << ::imebra::implementation::getCurrentTime() << " " << message << std::endl;

#endif

#endif // !defined(IMEBRA_LOG_FATAL_IMPLEMENTATION)

// Logging macros used by Imebra functions
//
///////////////////////////////////////////////////////////

#if IMEBRA_LOG_LEVEL >= IMEBRA_LOG_LEVEL_FATAL_VALUE
#define IMEBRA_LOG_FATAL(message) \
{ \
    std::ostringstream buildMessage; \
    buildMessage << message; \
    IMEBRA_LOG_FATAL_IMPLEMENTATION(buildMessage.str()); \
}
#else
#define IMEBRA_LOG_FATAL(message)
#endif

#if IMEBRA_LOG_LEVEL >= IMEBRA_LOG_LEVEL_ERROR_VALUE
#define IMEBRA_LOG_ERROR(message) \
{ \
    std::ostringstream buildMessage; \
    buildMessage << message; \
    IMEBRA_LOG_ERROR_IMPLEMENTATION(buildMessage.str()); \
}
#else
#define IMEBRA_LOG_ERROR(message)
#endif

#if IMEBRA_LOG_LEVEL >= IMEBRA_LOG_LEVEL_WARNING_VALUE
#define IMEBRA_LOG_WARNING(message) \
{ \
    std::ostringstream buildMessage; \
    buildMessage << message; \
    IMEBRA_LOG_WARNING_IMPLEMENTATION(buildMessage.str()); \
}
#else
#define IMEBRA_LOG_WARNING(message)
#endif

#if IMEBRA_LOG_LEVEL >= IMEBRA_LOG_LEVEL_INFO_VALUE
#define IMEBRA_LOG_INFO(message) \
{ \
    std::ostringstream buildMessage; \
    buildMessage << message; \
    IMEBRA_LOG_INFO_IMPLEMENTATION(buildMessage.str()); \
}
#else
#define IMEBRA_LOG_INFO(message)
#endif

#if IMEBRA_LOG_LEVEL >= IMEBRA_LOG_LEVEL_DEBUG_DEBUG
#define IMEBRA_LOG_DEBUG(message) \
{ \
    std::ostringstream buildMessage; \
    buildMessage << message; \
    IMEBRA_LOG_DEBUG_IMPLEMENTATION(buildMessage.str()); \
}
#else
#define IMEBRA_LOG_DEBUG(message)
#endif

#if IMEBRA_LOG_LEVEL >= IMEBRA_LOG_LEVEL_TRACE_VALUE
#define IMEBRA_LOG_TRACE(message) \
{ \
    std::ostringstream buildMessage; \
    buildMessage << message; \
    IMEBRA_LOG_TRACE_IMPLEMENTATION(buildMessage.str()); \
}
#else
#define IMEBRA_LOG_TRACE(message)
#endif


#endif // CImbxLogging_F1BAF067_21DE_466b_AEA1_6CC4F006FAFA__INCLUDED_
