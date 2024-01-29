# Minimum CMake version required
cmake_minimum_required(VERSION 3.0.0...3.27.3 FATAL_ERROR)

message("CMAKE version used: ${CMAKE_VERSION}")
set(CMAKE_CXX_STANDARD 11)

# Declare project
project(DCMTK)


#-------------------------------------DCMTK-----------------------------------

set(BUILD_APPS "OFF")

if(MSVC)
    set(DCMTK_OVERWRITE_WIN32_COMPILER_FLAGS "OFF")
endif()

if(DEFINED ENV{LIB_ICONV})
    message("LIB_ICONV set to: $ENV{LIB_ICONV}")
    set(WITH_LIBICONVINC $ENV{LIB_ICONV})
    set(DCMTK_WITH_ICONV "ON")
    if(UNIX)
        set(LIBCHARSET_INCLUDE_DIR $ENV{LIB_ICONV}/include) 
        set(LIBCHARSET_LIBRARY $ENV{LIB_ICONV}/lib/libcharset.a)
        set(LIBICONV_INCLUDE_DIR $ENV{LIB_ICONV}/include)
        set(LIBICONV_LIBDIR $ENV{LIB_ICONV}/lib)
        set(Iconv_INCLUDE_DIR $ENV{LIB_ICONV}/include)
        set(Iconv_LIBRARY $ENV{LIB_ICONV}/lib/libiconv.a)
    endif()
    set(DCMTK_ENABLE_CHARSET_CONVERSION "libiconv" CACHE STRING "${DCMTK_ENABLE_CHARSET_CONVERSION_DOCSTRING}")
endif()

# Check the build system
include(CMake/dcmtkPrepare.cmake NO_POLICY_SCOPE)

set(DCMTK_MODULES ofstd oflog dcmdata dcmnet dcmjpeg dcmjpls dcmimage dcmimgle dcmqrdb dcmj2k oficonv dcmtls
  CACHE STRING "List of modules that should be built.")
  
# Provide an interface target to elegantly include the config directory
add_library(config INTERFACE)
target_include_directories(config INTERFACE
  $<BUILD_INTERFACE:${DCMTK_BINARY_DIR}/config/include>
  $<INSTALL_INTERFACE:include>
)
install(TARGETS config EXPORT DCMTKTargets)

#-----------------------------------------------------------------------------
# Include directories
#-----------------------------------------------------------------------------

set(DCMTK_INCLUDE_DIR "${DCMTK_BINARY_DIR}/config/include")
foreach(inc ${DCMTK_MODULES})
  list(APPEND DCMTK_INCLUDE_DIR "${DCMTK_SOURCE_DIR}/${inc}/include")
endforeach()

include_directories(${DCMTK_INCLUDE_DIR})

#-----------------------------------------------------------------------------
# Prepare osconfig.h
#-----------------------------------------------------------------------------

# add the osconfig.h.in file
configure_file("${DCMTK_SOURCE_DIR}/CMake/osconfig.h.in"
               "${DCMTK_BINARY_DIR}/config/include/dcmtk/config/osconfig.h")

#-----------------------------------------------------------------------------
# Prepare arith.h
#-----------------------------------------------------------------------------

INSPECT_FUNDAMENTAL_ARITHMETIC_TYPES()

if(MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
  # Generate node.lib
  execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
endif()

# Add flags specific to the compiler
#-----------------------------------
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    message(STATUS "MSVC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
else()
    message(STATUS "GNU or Clang detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wconversion -Wfloat-equal -fPIC -std=c++11")
    set (CMAKE_POSITION_INDEPENDENT_CODE TRUE)
endif()

#-----------------------------------------------------------------------------
# Start actual compilation tasks
#-----------------------------------------------------------------------------

# Recurse into subdirectories
foreach(module config ${DCMTK_MODULES})
  add_subdirectory(${module})
endforeach()
include(CMake/dcmtkAfterModules.cmake NO_POLICY_SCOPE)


# Build a shared library named after the project from the files in `src/`
file(GLOB SOURCE_FILES "src/*.cc" "src/*.c" "src/*.h")
add_library(${PROJECT_NAME} SHARED  ${SOURCE_FILES})

# Define dependency libraries
#----------------------------
target_link_libraries(${PROJECT_NAME} ${DCMTK_MODULES})

# Gives our library file a .node extension without any "lib" prefix
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
IF (WIN32)
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/DELAYLOAD:node.exe" )
ENDIF(WIN32)

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
add_definitions(-DOPJ_STATIC)
# Essential library files to link to a node addon
# You should add this line in every CMake.js based project
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})

# define NPI_VERSION
add_definitions(-DNAPI_VERSION=6)


