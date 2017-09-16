set(PACKAGE           "protobuf")
set(VERSION           "2.3.0")
set(PACKAGE_BUGREPORT "protobuf@googlegroups.com")
set(PACKAGE_NAME      "Protocal Buffers")
set(PACKAGE_TARNAME   "protobuf")
set(PACKAGE_VERSION   "2.3.0")
set(PACKAGE_STRING    "Protocal Buffers 2.3.0")

#------------------------------------------------------------------------------
# Setup RPATH settings to ensure that the protoc exetuble can be run correctly
# in various build environments (linux/mac).
#------------------------------------------------------------------------------

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

#--------------------------------------------------------------------------

CHECK_INCLUDE_FILE("dlfcn.h"     HAVE_DLFCN_H)
CHECK_INCLUDE_FILE("inttypes.h"  HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE("memory.h"    HAVE_MEMORY_H)
CHECK_INCLUDE_FILE("stdint.h"    HAVE_STDINT_H)
CHECK_INCLUDE_FILE("stdlib.h"    HAVE_STDLIB_H)
CHECK_INCLUDE_FILE("strings.h"   HAVE_STRINGS_H)
CHECK_INCLUDE_FILE("string.h"    HAVE_STRING_H)
CHECK_INCLUDE_FILE("sys/stat.h"  HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE("sys/types.h" HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE("unistd.h"    HAVE_UNISTD_H)

# Enumerate all known and possible combinations of
# hash_map/hash_set header files location
# hash_map/hash_set namespace name and
# hash_map/hash_set class name
# Break on first successful compile and record names
function(determine_hash_namespace)
  if (DEFINED protobut_determine_hash_namespace_done)
    # BUG #14031: don't redo these tests each time.
    return()
  endif()
  # set this internal variable so we don't waste time doing try compiles each
  # time.
  set (protobut_determine_hash_namespace_done TRUE CACHE INTERNAL "")

  message(STATUS "Protobuf: doing try-compiles for hash map/set headers")
  set(NAMESPACE "")
  set(LOCATION_MAP "")
  set(CLASSNAME_MAP "")
  set(LOCATION_SET "")
  set(CLASSNAME_SET "")
  foreach(NAMESPACE_I std std::tr1 stdext __gnu_cxx "")
   # Try hash_map
   foreach(LOCATION_MAP_I hash_map tr1/unordered_map ext/hash_map)
    foreach(CLASSNAME_MAP_I unordered_map hash_map)
     configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMake/hash_map_test.cc.in
                    ${CMAKE_CURRENT_BINARY_DIR}/CMake/hash_map_test.cc
                    @ONLY)
     try_compile(SUCCEED_MAP
                 ${CMAKE_CURRENT_BINARY_DIR}/CMake
                 ${CMAKE_CURRENT_BINARY_DIR}/CMake/hash_map_test.cc)
     if (SUCCEED_MAP)
      set(CLASSNAME_MAP "${CLASSNAME_MAP_I}")
      break()
     endif()
    endforeach(CLASSNAME_MAP_I)
    if (SUCCEED_MAP)
     set(LOCATION_MAP "${LOCATION_MAP_I}")
     break()
    endif()
   endforeach(LOCATION_MAP_I)

   # Try hash_set
   foreach(LOCATION_SET_I hash_set tr1/unordered_set ext/hash_set)
    foreach(CLASSNAME_SET_I unordered_set hash_set)
     configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMake/hash_set_test.cc.in
                    ${CMAKE_CURRENT_BINARY_DIR}/CMake/hash_set_test.cc
                    @ONLY)
     try_compile(SUCCEED_SET
                 ${CMAKE_CURRENT_BINARY_DIR}/CMake
                 ${CMAKE_CURRENT_BINARY_DIR}/CMake/hash_set_test.cc)
     if (SUCCEED_SET)
      set(CLASSNAME_SET "${CLASSNAME_SET_I}")
      break()
     endif()
    endforeach(CLASSNAME_SET_I)
    if (SUCCEED_SET)
     set(LOCATION_SET "${LOCATION_SET_I}")
     break()
    endif()
   endforeach(LOCATION_SET_I)

   # See if we found a successful compile for both hash_map and hash_set
   if (SUCCEED_MAP AND SUCCEED_SET)
    # Namespace of hash_map/hash_set
    set(HASH_NAMESPACE "${NAMESPACE_I}" CACHE INTERNAL "")
    # Define if the compiler has hash_map
    set(HAVE_HASH_MAP 1 CACHE INTERNAL "")
    # Location of <hash_map> file
    set(HASH_MAP_H "${LOCATION_MAP}" CACHE INTERNAL "")
    # Class name of the <hash_map>
    set(HASH_MAP_CLASS "${CLASSNAME_MAP}" CACHE INTERNAL "")
    # Define if the compiler has hash_set
    set(HAVE_HASH_SET 1 CACHE INTERNAL "")
    # Location of <hash_set> file
    set(HASH_SET_H "${LOCATION_SET}" CACHE INTERNAL "")
    # Class name of <hash_set>
    set(HASH_SET_CLASS "${CLASSNAME_SET}" CACHE INTERNAL "")
    break()
   endif ()
  endforeach(NAMESPACE_I)
endfunction()

determine_hash_namespace()

# Define to the sub-directory in which libtool stores uninstalled libraries
# LT_OBJDIR

# Define to necessary symbol if this constant
# uses a non-standard name on your system
function(pthread_test var)
  if (DEFINED protobut_pthread_test_done)
    set (${var} "${pthread_test_result}" PARENT_SCOPE)
    return()
  endif()
  set (protobut_pthread_test_done TRUE CACHE INTERNAL "")

  message(STATUS "Protobuf: doing try-compiles for pthread test")

  set (_result "UNKNOWN")
  foreach(PTHREAD_TEST PTHREAD_CREATE_JOINABLE PTHREAD_CREATE_UNDETACHED)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMake/pthread_test.cc.in
                   ${CMAKE_CURRENT_BINARY_DIR}/CMake/pthread_test.cc
                   @ONLY)
    try_compile(SUCCEED
                ${CMAKE_CURRENT_BINARY_DIR}/CMake
                ${CMAKE_CURRENT_BINARY_DIR}/CMake/pthread_test.cc)
    if (SUCCEED)
      set(_result "${PTHREAD_TEST}")
      break()
    endif ()
  endforeach(PTHREAD_TEST)

  set (pthread_test_result "${_result}" CACHE INTERNAL "")
  set (${var} "${pthread_test_result}" PARENT_SCOPE)
endfunction()

set(ATTR_NAME "UNKNOWN")
pthread_test(ATTR_NAME)

if ("${ATTR_NAME}" STREQUAL "${PTHREAD_CREATE_JOINABLE}")
 set(PTHREAD_CREATE_JOINABLE "${ATTR_NAME}")
endif ()

# Define to 1 if you have the ANSI C header files
CHECK_INCLUDE_FILE("stddef.h" HAVE_STDDEF_H)
if (HAVE_STDLIB_H AND HAVE_STDDEF_H)
 set(STDC_HEADERS 1)
endif ()

# Config config.h.cmake.in
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# Include directories for this project
if (PROTOBUF_USE_ZLIB)
  include_directories(${ZLIB_INCLUDE_DIRS})
endif()
include_directories(
 ${CMAKE_CURRENT_BINARY_DIR}
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_SOURCE_DIR}
 ${CMAKE_SOURCE_DIR}/..)

# Targets ==============================================================
# protobuf library
set(LIBPROTOBUF_SOURCES
 google/protobuf/stubs/common.cc
 google/protobuf/stubs/once.cc
 google/protobuf/stubs/hash.cc
 google/protobuf/extension_set.cc
 google/protobuf/generated_message_util.cc
 google/protobuf/message_lite.cc
 google/protobuf/repeated_field.cc
 google/protobuf/wire_format_lite.cc
 google/protobuf/io/coded_stream.cc
 google/protobuf/io/zero_copy_stream.cc
 google/protobuf/io/zero_copy_stream_impl_lite.cc
 google/protobuf/stubs/strutil.cc
 google/protobuf/stubs/substitute.cc
 google/protobuf/stubs/structurally_valid.cc
 google/protobuf/descriptor.cc
 google/protobuf/descriptor.pb.cc
 google/protobuf/descriptor_database.cc
 google/protobuf/dynamic_message.cc
 google/protobuf/extension_set_heavy.cc
 google/protobuf/generated_message_reflection.cc
 google/protobuf/message.cc
 google/protobuf/reflection_ops.cc
 google/protobuf/service.cc
 google/protobuf/text_format.cc
 google/protobuf/unknown_field_set.cc
 google/protobuf/wire_format.cc
 google/protobuf/io/printer.cc
 google/protobuf/io/tokenizer.cc
 google/protobuf/io/zero_copy_stream_impl.cc
 google/protobuf/compiler/importer.cc
 google/protobuf/compiler/parser.cc)
if (PROTOBUF_USE_ZLIB)
    # Keeping the ZLIB dependent source file separate makes it possible to build
    # protoc executable without any zlib dependecies. Any extra depedencies on
    # protoc executable make it complicated to use the executable during build
    # steps for projects such as ParaView since they require the environment to
    # be set up correctly hence we avoid as such dependencies on protoc at all
    # costs!!!
    set (LIBPROTOBUF_SOURCES_EXTRA
      google/protobuf/io/gzip_stream.cc)
endif()
add_library(protobuf ${LIBPROTOBUF_SOURCES} ${LIBPROTOBUF_SOURCES_EXTRA})
if (WIN32 AND BUILD_SHARED_LIBS)
 set_target_properties(protobuf PROPERTIES
                       COMPILE_DEFINITIONS "LIBPROTOBUF_EXPORTS;PROTOBUF_USE_DLLS")
endif (WIN32 AND BUILD_SHARED_LIBS)
target_link_libraries(protobuf ${THREAD_LINK_LIB} ${ZLIB_LINK_LIB})

# protobuf-lite library
set(PROTOBUF_LITE_SOURCES
 google/protobuf/stubs/common.cc
 google/protobuf/stubs/once.cc
 google/protobuf/stubs/hash.cc
 google/protobuf/extension_set.cc
 google/protobuf/generated_message_util.cc
 google/protobuf/message_lite.cc
 google/protobuf/repeated_field.cc
 google/protobuf/wire_format_lite.cc
 google/protobuf/io/coded_stream.cc
 google/protobuf/io/zero_copy_stream.cc
 google/protobuf/io/zero_copy_stream_impl_lite.cc)
add_library(protobuf-lite ${PROTOBUF_LITE_SOURCES})
if (WIN32 AND BUILD_SHARED_LIBS)
 set_target_properties(protobuf-lite PROPERTIES
                       COMPILE_DEFINITIONS "LIBPROTOBUF_EXPORTS;PROTOBUF_USE_DLLS")
endif (WIN32 AND BUILD_SHARED_LIBS)
target_link_libraries(protobuf-lite ${THREAD_LINK_LIB})

# protoc library
set(LIBPROTOC_SOURCES
 google/protobuf/compiler/code_generator.cc
 google/protobuf/compiler/command_line_interface.cc
 google/protobuf/compiler/cpp/cpp_enum.cc
 google/protobuf/compiler/cpp/cpp_enum_field.cc
 google/protobuf/compiler/cpp/cpp_extension.cc
 google/protobuf/compiler/cpp/cpp_field.cc
 google/protobuf/compiler/cpp/cpp_file.cc
 google/protobuf/compiler/cpp/cpp_generator.cc
 google/protobuf/compiler/cpp/cpp_helpers.cc
 google/protobuf/compiler/cpp/cpp_message.cc
 google/protobuf/compiler/cpp/cpp_message_field.cc
 google/protobuf/compiler/cpp/cpp_primitive_field.cc
 google/protobuf/compiler/cpp/cpp_service.cc
 google/protobuf/compiler/cpp/cpp_string_field.cc
 google/protobuf/compiler/plugin.cc
 google/protobuf/compiler/plugin.pb.cc
 google/protobuf/compiler/subprocess.cc
 google/protobuf/compiler/zip_writer.cc
 )
link_directories(${LIBRARY_OUTPUT_PATH})
# Removing protoc lib since it's causing build issues with Ninja generator on
# Windows. protoc_compiler doesn't depend on protoc lib anyways. That dependency
# was removed a while ago to avoid shared library issues.
#add_library(protoc ${LIBPROTOC_SOURCES})
#if (WIN32 AND BUILD_SHARED_LIBS)
# set_target_properties(protoc PROPERTIES
#                       COMPILE_DEFINITIONS "LIBPROTOC_EXPORTS;PROTOBUF_USE_DLLS")
#endif (WIN32 AND BUILD_SHARED_LIBS)
#target_link_libraries(protoc ${THREAD_LINK_LIB} protobuf)

# protoc executable
link_directories(${LIBRARY_OUTPUT_PATH})
IF(NOT CMAKE_CROSSCOMPILING)
  # We build the protoc executable without any dependencies on libraries built
  # here viz. protobuf/protoc. This avoids unnecessary build-time
  # complications when using protoc to generate headers in projects.
   add_executable(protoc_compiler
      google/protobuf/compiler/main.cc
      ${LIBPROTOC_SOURCES}
      ${LIBPROTOBUF_SOURCES})
   set_target_properties(protoc_compiler PROPERTIES
                         OUTPUT_NAME     protoc)
   target_link_libraries(protoc_compiler
                         ${THREAD_LINK_LIB})
ENDIF(NOT CMAKE_CROSSCOMPILING)

IF(NOT CMAKE_CROSSCOMPILING)
  export(TARGETS protobuf protobuf-lite protoc_compiler
         APPEND FILE "${PROTOBUF_EXPORTS_FILE}")
ELSE(NOT CMAKE_CROSSCOMPILING)
  export(TARGETS protobuf protobuf-lite
         APPEND FILE "${PROTOBUF_EXPORTS_FILE}")
ENDIF(NOT CMAKE_CROSSCOMPILING)

install (TARGETS protobuf protobuf-lite
  EXPORT ${PROTOBUF_INSTALL_EXPORT_NAME}
  RUNTIME DESTINATION ${PROTOBUF_INSTALL_BIN_DIR} COMPONENT Runtime
  LIBRARY DESTINATION ${PROTOBUF_INSTALL_LIB_DIR} COMPONENT Runtime
  ARCHIVE DESTINATION ${PROTOBUF_INSTALL_LIB_DIR} COMPONENT Development)

IF(COMPILE_TOOLS_TARGET)
  ADD_DEPENDENCIES(${COMPILE_TOOLS_TARGET} protoc_compiler)
ENDIF(COMPILE_TOOLS_TARGET)
