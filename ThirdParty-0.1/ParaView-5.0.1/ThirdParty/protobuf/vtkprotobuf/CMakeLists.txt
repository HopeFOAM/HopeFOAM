cmake_minimum_required(VERSION 2.6)
project(protobuf)

set (PROTOBUF_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (PROTOBUF_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

IF (NOT PROTOBUF_INSTALL_BIN_DIR)
  SET (PROTOBUF_INSTALL_BIN_DIR bin)
ENDIF()
IF (NOT PROTOBUF_INSTALL_LIB_DIR)
  SET (PROTOBUF_INSTALL_LIB_DIR lib)
ENDIF()
IF (NOT PROTOBUF_INSTALL_EXPORT_NAME)
  SET (PROTOBUF_INSTALL_EXPORT_NAME ProtobufTargets)
ENDIF()

if (NOT WIN32)
  OPTION(BUILD_SHARED_LIBS "Build shared protobuf library" ON)
endif()

#disable compiler warnings from the protobuf library
option(PROTOBUF_DISABLE_COMPILER_WARNINGS "Disable compiler warnings" ON)
mark_as_advanced(PROTOBUF_DISABLE_COMPILER_WARNINGS)
if(PROTOBUF_DISABLE_COMPILER_WARNINGS)
  if(WIN32)
    if (MSVC)
      string(REGEX REPLACE "(^| )([/-])W[0-9]( |$)" " "
          CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
      set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} /w")

      string(REGEX REPLACE "(^| )([/-])W[0-9]( |$)" " "
          CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
    endif(MSVC)
  else(WIN32)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
  endif(WIN32)
endif(PROTOBUF_DISABLE_COMPILER_WARNINGS)

# PROTOBUF_USE_DLLS is added on per target basis (see src/CMakeLists.txt). If we
# add this globally, then we get errors when building the static executable
# "protoc".
#if (WIN32 AND BUILD_SHARED_LIBS)
# add_definitions(-DPROTOBUF_USE_DLLS)
#endif (WIN32 AND BUILD_SHARED_LIBS)

# Direct all binary outputs to one directory
if(NOT LIBRARY_OUTPUT_PATH)
  set(LIBRARY_OUTPUT_PATH    ${CMAKE_CURRENT_BINARY_DIR}/bin CACHE INTERNAL "")
endif(NOT LIBRARY_OUTPUT_PATH)

if(NOT EXECUTABLE_OUTPUT_PATH)
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin CACHE INTERNAL "")
endif(NOT EXECUTABLE_OUTPUT_PATH)

# Set up protobuf include directories, and the export file
set(PROTOBUF_INCLUDE_DIRS
  "${PROTOBUF_SOURCE_DIR}/src"
  "${PROTOBUF_BINARY_DIR}/src")
set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES "${protobuf_INCLUDE_DIRS}")
if (NOT DEFINED PROTOBUF_EXPORTS_FILE)
  set(PROTOBUF_EXPORTS_FILE "${PROTOBUF_BINARY_DIR}/PROTOBUF_EXPORTS.cmake")
  file(REMOVE "${PROTOBUF_EXPORTS_FILE}")
endif()

INCLUDE (CheckFunctionExists)
INCLUDE (CheckIncludeFile)
INCLUDE (CheckIncludeFileCXX)
INCLUDE (CheckIncludeFiles)
INCLUDE (CheckLibraryExists)
INCLUDE (CheckSymbolExists)
INCLUDE (CheckTypeSize)

find_package(Threads)
set(THREAD_LINK_LIB "")
if (Threads_FOUND)
 set(HAVE_PTHREAD 1)
 set(THREAD_LINK_LIB ${CMAKE_THREAD_LIBS_INIT})
endif ()

# Enable classes using zlib compression
set(PROTOBUF_USE_ZLIB 0)
if (PROTOBUF_ENABLE_ZLIB)
  find_package(ZLIB)
  set(ZLIB_LINK_LIB "")
  if (ZLIB_FOUND)
    set(HAVE_ZLIB 1)
    set(ZLIB_LINK_LIB ${ZLIB_LIBRARIES})
    set(PROTOBUF_USE_ZLIB 1)
  endif (ZLIB_FOUND)
endif()

# Source code for protobuf and its test cases
add_subdirectory(src)

# Create the configure file for protobuf
configure_file("${PROTOBUF_SOURCE_DIR}/protobufConfig.cmake.in"
  "${PROTOBUF_BINARY_DIR}/protobufConfig.cmake" @ONLY)
