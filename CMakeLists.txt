cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0042 NEW)
set (CMAKE_CXX_STANDARD 11)

# Name of the project (will be the name of the plugin)
project(imebra)

set(imebra_libraries "")

# Select the proper charset conversion libraries according to the charset conversion method
#------------------------------------------------------------------------------------------
if(IMEBRA_CHARSET_CONVERSION MATCHES ICONV)
    message("Using ICONV")
    add_definitions(-DIMEBRA_USE_ICONV)
    if(APPLE)
        set(imebra_libraries ${imebra_libraries} iconv)
    endif(APPLE)
elseif(IMEBRA_CHARSET_CONVERSION MATCHES ICU)
    message("Using ICU")
    add_definitions(-DIMEBRA_USE_ICU)
    set(imebra_libraries ${imebra_libraries} icuuc)
    set(imebra_libraries ${imebra_libraries} icuio)
elseif(IMEBRA_CHARSET_CONVERSION MATCHES JAVA)
    message("Using JAVA")
    add_definitions(-DIMEBRA_USE_JAVA)
elseif(IMEBRA_CHARSET_CONVERSION MATCHES WINDOWS)
    message("Using WINDOWS CHARSET CONVERSION")
    add_definitions(-DIMEBRA_USE_WINDOWS_CHARSET)
else(IMEBRA_CHARSET_CONVERSION MATCHES ICONV)
    if(WIN32)
        message("Using WINDOWS CHARSET CONVERSION (default)")
        add_definitions(-DIMEBRA_USE_WINDOWS_CHARSET)
    else(WIN32)
        message("Using ICONV (default)")
        add_definitions(-DIMEBRA_USE_ICONV)
        if(APPLE)
            set(imebra_libraries ${imebra_libraries} iconv)
        endif(APPLE)
    endif(WIN32)
endif(IMEBRA_CHARSET_CONVERSION MATCHES ICONV)

# Set the proper logging preprocessor macros
#-------------------------------------------
if(IMEBRA_LOG_FRAMEWORK MATCHES LOG4CXX)
    add_definitions(-DIMEBRA_LOGGING_LOG4CXX)
    set(imebra_libraries ${imebra_libraries} log4cxx) # Log4cxx needs additional libraries
elseif(IMEBRA_LOG_FRAMEWORK MATCHES SYSLOG)
    add_definitions(-DIMEBRA_LOGGING_SYSLOG)
elseif(IMEBRA_LOG_FRAMEWORK MATCHES COUT)
    add_definitions(-DIMEBRA_LOGGING_COUT)
elseif(IMEBRA_LOG_FRAMEWORK MATCHES APPLE)
    add_definitions(-DIMEBRA_LOGGING_APPLE)
elseif(IMEBRA_LOG_FRAMEWORK MATCHES ANDROID)
    add_definitions(-DIMEBRA_LOGGING_ANDROID)
endif(IMEBRA_LOG_FRAMEWORK MATCHES LOG4CXX)

if(IMEBRA_LOG_LEVEL)
    add_definitions(-DIMEBRA_LOG_LEVEL_${IMEBRA_LOG_LEVEL})
endif(IMEBRA_LOG_LEVEL)


##############################################
#
# LIBRARY
#
##############################################
add_definitions(-DIMEBRA_DLL)
add_definitions(-DIMEBRA_DLL_EXPORTS)
add_definitions(-DNOMINMAX)

file(GLOB imebra_interface "${CMAKE_CURRENT_SOURCE_DIR}/library/include/imebra/*.h")
file(GLOB imebra_include "${CMAKE_CURRENT_SOURCE_DIR}/library/src/*.h")
file(GLOB imebra_src "${CMAKE_CURRENT_SOURCE_DIR}/library/src/*.cpp")
file(GLOB imebra_implementation_src "${CMAKE_CURRENT_SOURCE_DIR}/library/implementation/*.cpp")
file(GLOB imebra_implementation_src_objc "${CMAKE_CURRENT_SOURCE_DIR}/library/implementation/*.mm")
file(GLOB imebra_implementation_include "${CMAKE_CURRENT_SOURCE_DIR}/library/implementation/*.h")
set(imebra_objc_src "")
set(imebra_objc_include "")
set(imebra_objc_include_dir "")
set(imebra_objc_gnustep_dir "")

# Build a shared library named after the project from the files in `src/`
file(GLOB SOURCE_FILES "src/*.cc" "src/*.h")
add_library(${PROJECT_NAME} SHARED 
	${SOURCE_FILES}
	${imebra_implementation_src}
    ${imebra_implementation_src_objc}
    ${imebra_implementation_include}
    ${imebra_objc_src}
    ${imebra_objc_include}
    ${imebra_include}
    ${imebra_src}
    ${imebra_interface}
	)
	
# Add flags specific to the compiler
#-----------------------------------
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

    message(STATUS "MSVC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    set(imebra_libraries ${imebra_libraries} Kernel32)

else()

    message(STATUS "GNU or Clang detected, adding compile flags")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wpedantic -Wconversion -Wfloat-equal ${HIDDEN_FLAGS}")
    set(imebra_libraries ${imebra_libraries} pthread)
    if("${EMBED_BITCODE}" STREQUAL "1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fembed-bitcode")
    endif("${EMBED_BITCODE}" STREQUAL "1")

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        if(CMAKE_BUILD_TYPE MATCHES DEBUG)
            message("Debug mode, enabling coverage")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
            set(imebra_libraries ${imebra_libraries} gcov)
        endif()
    endif()

endif()

# Define dependency libraries
#----------------------------
target_link_libraries(imebra ${imebra_libraries})

# Define Imebra public folder
#----------------------------
target_include_directories(imebra PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/library/include> $<INSTALL_INTERFACE:include>)
target_include_directories(imebra PUBLIC $<BUILD_INTERFACE:${imebra_objc_include_dir}> $<INSTALL_INTERFACE:include>)

# Set the library version from IMEBRA_MAJOR_VERSION, IMEBRA_MINOR_VERSION,
#  & IMEBRA_REVISION
#---------------------------------------------------------------------------------
if ("$ENV{IMEBRA_MAJOR_VERSION}" STREQUAL "")
    set(IMEBRA_MAJOR_VERSION "5")
else("$ENV{IMEBRA_MAJOR_VERSION}" STREQUAL "")
    set(IMEBRA_MAJOR_VERSION "$ENV{IMEBRA_MAJOR_VERSION}")
endif("$ENV{IMEBRA_MAJOR_VERSION}" STREQUAL "")

if ("$ENV{IMEBRA_MINOR_VERSION}" STREQUAL "")
    set(IMEBRA_MINOR_VERSION "0")
else("$ENV{IMEBRA_MINOR_VERSION}" STREQUAL "")
    set(IMEBRA_MINOR_VERSION "$ENV{IMEBRA_MINOR_VERSION}")
endif("$ENV{IMEBRA_MINOR_VERSION}" STREQUAL "")

if ("$ENV{IMEBRA_REVISION}" STREQUAL "")
    set(IMEBRA_REVISION "0")
else("$ENV{IMEBRA_REVISION}" STREQUAL "")
    set(IMEBRA_REVISION "$ENV{IMEBRA_REVISION}")
endif("$ENV{IMEBRA_REVISION}" STREQUAL "")

set(IMEBRA_VERSION "${IMEBRA_MAJOR_VERSION}.${IMEBRA_MINOR_VERSION}.${IMEBRA_REVISION}")

set_target_properties(imebra PROPERTIES VERSION ${IMEBRA_VERSION} SOVERSION ${IMEBRA_MAJOR_VERSION})


# Gives our library file a .node extension without any "lib" prefix
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# Include N-API wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})


# Essential include files to build a node addon,
# You should add this line in every CMake.js based project
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC} ${NODE_ADDON_API_DIR})

# Essential library files to link to a node addon
# You should add this line in every CMake.js based project
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})

# define NPI_VERSION
add_definitions(-DNAPI_VERSION=4)