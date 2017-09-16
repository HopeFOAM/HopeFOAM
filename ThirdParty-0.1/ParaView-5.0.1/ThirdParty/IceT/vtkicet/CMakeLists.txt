## Copyright 2003 Sandia Coporation
## Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
## the U.S. Government retains certain rights in this software.
##
## This source code is released under the New BSD License.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Turn off policy 0017, which makes modules packaged with CMake find
# any module rather than just others packaged with CMake.  This is
# necessary for backward compatibility when included from ParaView.
IF (POLICY CMP0017)
  CMAKE_POLICY(SET CMP0017 OLD)
ENDIF (POLICY CMP0017)

PROJECT(ICET C)

# Set the current IceT version.
SET(ICET_MAJOR_VERSION 2)
SET(ICET_MINOR_VERSION 2)
SET(ICET_PATCH_VERSION 0)
SET(ICET_VERSION "${ICET_MAJOR_VERSION}.${ICET_MINOR_VERSION}.${ICET_PATCH_VERSION}")

# CMake 2.8.12 and later supports an rpath mechanism on Mac OSX that allows
# build and install targets to point to dependent libraries that are not
# dependent on DYLD_LIBRARY_PATH. This variable turns that behavior on by
# default and also suppresses CMake policy warning 0042.
SET(CMAKE_MACOSX_RPATH ON)

# Set output paths.
SET(LIBRARY_OUTPUT_PATH ${ICET_BINARY_DIR}/lib CACHE PATH
  "Output directory for building all libraries.")
SET(EXECUTABLE_OUTPUT_PATH ${ICET_BINARY_DIR}/bin CACHE PATH
  "Output directory for building all executable.")
MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)
SET(ICET_LIBRARY_DIR ${LIBRARY_OUTPUT_PATH})
SET(ICET_EXECUTABLE_DIR ${EXECUTABLE_OUTPUT_PATH})

SET(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_SOURCE_DIR}/cmake
  )

# Turn shared libraries on or off.
OPTION(BUILD_SHARED_LIBS "Build IceT with shared libraries." OFF)
SET(ICET_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

# Options controlling support libraries
OPTION(ICET_USE_OPENGL "Build OpenGL support layer for IceT." ON)
OPTION(ICET_USE_OSMESA "Use OffScreen Mesa" OFF)
OPTION(ICET_USE_OFFSCREEN_EGL "Use OffScreen rendering through EGL" OFF)
OPTION(ICET_USE_MPI "Build MPI communication layer for IceT." ON)

# Option to set the preferred K value to use in the radix-k algorithm
SET(initial_magic_k 8)
IF ("${CMAKE_SYSTEM_NAME}" MATCHES "^BlueGene")
  SET(initial_magic_k 16)
ENDIF ("${CMAKE_SYSTEM_NAME}" MATCHES "^BlueGene")
IF ("$ENV{ICET_MAGIC_K}" GREATER 1)
  SET(initial_magic_k $ENV{ICET_MAGIC_K})
ENDIF ("$ENV{ICET_MAGIC_K}" GREATER 1)
SET(ICET_MAGIC_K ${initial_magic_k} CACHE STRING
  "Sets the preferred `k' value for the radix-k algorithm.  This is the amount of simultaneous messages each process receives.  A value of 8 is generally good on most architectures, but in others that have slower computation with respect to network (such as BlueGene), a larger value may give better performance."
  )
IF (NOT ${ICET_MAGIC_K} GREATER 1)
  MESSAGE(SEND_ERROR "ICET_MAGIC_K must be set to a number greater than 1.")
ENDIF (NOT ${ICET_MAGIC_K} GREATER 1)

# Option to set the preferred number of ways to break up an image.
SET(initial_max_image_split 512)
IF ("$ENV{ICET_MAX_IMAGE_SPLIT}" GREATER 0)
  SET(initial_max_image_split $ENV{ICET_MAX_IMAGE_SPLIT})
ENDIF ("$ENV{ICET_MAX_IMAGE_SPLIT}" GREATER 0)
SET(ICET_MAX_IMAGE_SPLIT ${initial_max_image_split} CACHE STRING
  "Sets the preferred number of times an image may be split.  Some image compositing algorithms prefer to partition the images such that each process gets a piece.  Too many partitions, though, and you end up spending more time collecting them than you save balancing the compositing."
  )

# Configure MPE support
IF (ICET_USE_MPI)
  OPTION(ICET_USE_MPE "Use MPE to trace MPI communications.  This is helpful for developers trying to measure the performance of parallel compositing algorithms." OFF)
  MARK_AS_ADVANCED(ICET_USE_MPE)
ENDIF (ICET_USE_MPI)

# Configure testing support.
INCLUDE(Dart)
IF (BUILD_TESTING)
  ENABLE_TESTING()
ENDIF (BUILD_TESTING)

IF (CMAKE_BUILD_TYPE MATCHES "Debug")
  ADD_DEFINITIONS(-DDEBUG)
ENDIF (CMAKE_BUILD_TYPE MATCHES "Debug")

IF (UNIX)
  LINK_LIBRARIES(m)
ENDIF (UNIX)

# Configure OpenGL support.
IF (ICET_USE_OPENGL)
  IF (ICET_USE_OSMESA)
    FIND_PACKAGE(OSMesa REQUIRED)
    INCLUDE_DIRECTORIES(${OSMESA_INCLUDE_DIR})
    SET(ICET_OPENGL_LIBRARIES ${OSMESA_LIBRARY})
  ELSEIF (ICET_USE_OFFSCREEN_EGL)
    FIND_PACKAGE(EGL REQUIRED)
    INCLUDE_DIRECTORIES(${EGL_INCLUDE_DIR})
    SET(ICET_OPENGL_LIBRARIES ${EGL_LIBRARIES})
  ELSE()
    FIND_PACKAGE(OpenGL REQUIRED)
    INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
    SET(ICET_OPENGL_LIBRARIES ${OPENGL_LIBRARIES})
  ENDIF()
ENDIF (ICET_USE_OPENGL)

# Configure MPI support.
IF (ICET_USE_MPI)
  FIND_PACKAGE(MPI REQUIRED)
  # Mark certain variables as non-advanced.

  IF (DEFINED MPI_C_COMPILER)
    # Using newer MPI module.
    # Older MPI module
    MARK_AS_ADVANCED(CLEAR MPI_C_INCLUDE_PATH)
    MARK_AS_ADVANCED(CLEAR MPI_C_LIBRARIES)

    INCLUDE_DIRECTORIES(${MPI_C_INCLUDE_PATH})
    SET (ICET_MPI_LIBRARIES ${MPI_C_LIBRARIES})
  ELSE (DEFINED MPI_C_COMPILER)
    # Older MPI module
    MARK_AS_ADVANCED(CLEAR MPI_INCLUDE_PATH)
    MARK_AS_ADVANCED(CLEAR MPI_LIBRARY)
    MARK_AS_ADVANCED(CLEAR MPI_EXTRA_LIBRARY)

    INCLUDE_DIRECTORIES(${MPI_INCLUDE_PATH})
    SET(ICET_MPI_LIBRARIES ${MPI_LIBRARY} ${MPI_EXTRA_LIBRARY})
  ENDIF(DEFINED MPI_C_COMPILER)

  # Set up variables used to run MPI programs.  In CMake 2.8 or later, they are
  # defined by the FindMPI module.
  IF (MPIEXEC)
    SET(ICET_MPIRUN_EXE ${MPIEXEC})
    SET(ICET_MPI_NUMPROC_FLAG ${MPIEXEC_NUMPROC_FLAG})
    SET(ICET_MPI_PREFLAGS ${MPIEXEC_PREFLAGS})
    SET(ICET_MPI_POSTFLAGS ${MPIEXEC_POSTFLAGS})
  ELSE (MPIEXEC)
    FIND_PROGRAM(ICET_MPIRUN_EXE NAMES mpiexec mpirun lamexec)
    SET(ICET_MPI_NUMPROC_FLAG "-np" CACHE STRING "Flag used by MPI start program.  Used to specify the number of processes.")
    SET(ICET_MPI_PREFLAGS "" CACHE STRING "Flags used by MPI start program.  These are placed directly before the executable.")
    SET(ICET_MPI_POSTFLAGS "" CACHE STRING "Flags used by MPI start program.  These are placed after all other flags.")
    MARK_AS_ADVANCED(
      ICET_MPIRUN_EXE
      ICET_MPI_NUMPROC_FLAG
      ICET_MPI_PREFLAGS
      ICET_MPI_POSTFLAGS
      )
  ENDIF (MPIEXEC)
  SET(ICET_MPI_MAX_NUMPROCS "2" CACHE STRING "Maximum number of processors available to run parallel applications.")
  MARK_AS_ADVANCED(ICET_MPI_MAX_NUMPROCS)
  SEPARATE_ARGUMENTS(ICET_MPI_PREFLAGS)
  SEPARATE_ARGUMENTS(ICET_MPI_POSTFLAGS)

  IF (ICET_USE_MPE)
    FIND_PACKAGE(MPE REQUIRED)
    INCLUDE_DIRECTORIES(${MPE_LOG_INCLUDE_PATH})
    SET(ICET_MPI_LIBRARIES ${MPE_LOG_LIBRARIES} ${ICET_MPI_LIBRARIES})
  ENDIF (ICET_USE_MPE)
ENDIF (ICET_USE_MPI)

# Add extra warnings when possible.  The IceT build should be clean.  I expect
# no warnings when bulding this code.
IF(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_COMPILER_IS_CLANG 1)
ENDIF()

IF (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANG)
  SET(ICET_C_FLAGS_WARN "-ansi -Wall -Wno-long-long -Wextra -Wformat-security -Wshadow -Wunused -Wreturn-type -Wpointer-arith -Wdeclaration-after-statement")
ENDIF (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANG)

# Configure testing support.
IF (BUILD_TESTING)
  OPTION(ICET_BUILD_TESTING "Build and run the IceT tests." ON)
  MARK_AS_ADVANCED(ICET_BUILD_TESTING)
ENDIF (BUILD_TESTING)

# Configure data type sizes.
INCLUDE (CheckTypeSize)
CHECK_TYPE_SIZE(char        ICET_SIZEOF_CHAR)
CHECK_TYPE_SIZE(short       ICET_SIZEOF_SHORT)
CHECK_TYPE_SIZE(int         ICET_SIZEOF_INT)
CHECK_TYPE_SIZE(long        ICET_SIZEOF_LONG)
CHECK_TYPE_SIZE("long long" ICET_SIZEOF_LONG_LONG)
CHECK_TYPE_SIZE(__int64     ICET_SIZEOF___INT64)
CHECK_TYPE_SIZE(float       ICET_SIZEOF_FLOAT)
CHECK_TYPE_SIZE(double      ICET_SIZEOF_DOUBLE)
CHECK_TYPE_SIZE("void*"     ICET_SIZEOF_VOID_P)

#-----------------------------------------------------------------------------
# Configure install locations.  This allows parent projects to modify
# the install location.
IF(NOT ICET_INSTALL_BIN_DIR)
  SET(ICET_INSTALL_BIN_DIR bin)
ENDIF(NOT ICET_INSTALL_BIN_DIR)
IF(NOT ICET_INSTALL_INCLUDE_DIR)
  SET(ICET_INSTALL_INCLUDE_DIR include)
ENDIF(NOT ICET_INSTALL_INCLUDE_DIR)
IF(NOT ICET_INSTALL_LIB_DIR)
  SET(ICET_INSTALL_LIB_DIR lib)
ENDIF(NOT ICET_INSTALL_LIB_DIR)
IF(NOT ICET_INSTALL_MAN_DIR)
  SET(ICET_INSTALL_MAN_DIR share/man)
ENDIF(NOT ICET_INSTALL_MAN_DIR)
IF(NOT ICET_INSTALL_EXPORT_NAME)
  SET(ICET_INSTALL_EXPORT_NAME IceTTargets)
ENDIF()

# Shared libraries are considered both runtime and development and
# static libraries are considered development only.  In order to
# switch library installation on and off correctly we make the
# decision here.
SET(ICET_INSTALL_NO_LIBRARIES)
IF(BUILD_SHARED_LIBS)
  IF(ICET_INSTALL_NO_RUNTIME AND ICET_INSTALL_NO_DEVELOPMENT)
    SET(ICET_INSTALL_NO_LIBRARIES 1)
  ENDIF(ICET_INSTALL_NO_RUNTIME AND ICET_INSTALL_NO_DEVELOPMENT)
ELSE(BUILD_SHARED_LIBS)
  IF(ICET_INSTALL_NO_DEVELOPMENT)
    SET(ICET_INSTALL_NO_LIBRARIES 1)
  ENDIF(ICET_INSTALL_NO_DEVELOPMENT)
ENDIF(BUILD_SHARED_LIBS)

# Configure files with settings for use by the build.
CONFIGURE_FILE(${ICET_SOURCE_DIR}/src/include/IceTConfig.h.in
               ${ICET_BINARY_DIR}/src/include/IceTConfig.h)

# Point to IceT include files.
INCLUDE_DIRECTORIES(${ICET_SOURCE_DIR}/src/include)
INCLUDE_DIRECTORIES(${ICET_BINARY_DIR}/src/include)

# When creating one of the IceT libraries, we want special instructions
# for building and installing.
SET(ICET_INSTALL_TARGETS "" CACHE INTERNAL "" FORCE)
MACRO(ICET_ADD_LIBRARY library_name)
  SET(ICET_INSTALL_TARGETS ${ICET_INSTALL_TARGETS} ${library_name}
    CACHE INTERNAL "" FORCE
    )
  SET(icet_library_sources ${ARGN})
  ADD_LIBRARY(${library_name} ${icet_library_sources})
  IF (ICET_C_FLAGS_WARN)
    SET_SOURCE_FILES_PROPERTIES(${icet_library_sources}
      PROPERTIES COMPILE_FLAGS ${ICET_C_FLAGS_WARN}
      )
  ENDIF (ICET_C_FLAGS_WARN)
  IF(NOT ICET_INSTALL_NO_LIBRARIES)
    INSTALL(TARGETS ${library_name}
      EXPORT ${ICET_INSTALL_EXPORT_NAME}
      RUNTIME DESTINATION ${ICET_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
      LIBRARY DESTINATION ${ICET_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION ${ICET_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
      )
    IF (NOT ICET_INSTALL_NO_DEVELOPMENT)
      INSTALL(EXPORT ${ICET_INSTALL_EXPORT_NAME}
        DESTINATION ${ICET_INSTALL_LIB_DIR}
        )
    ENDIF (NOT ICET_INSTALL_NO_DEVELOPMENT)
  ENDIF(NOT ICET_INSTALL_NO_LIBRARIES)
ENDMACRO(ICET_ADD_LIBRARY)

ADD_SUBDIRECTORY(src)

IF (BUILD_TESTING AND ICET_BUILD_TESTING)
  IF (ICET_USE_MPI)
    ADD_SUBDIRECTORY(tests)
  ELSE (ICET_USE_MPI)
    MESSAGE(STATUS "Tests require ICET_USE_MPI.  Disabling tests.")
  ENDIF (ICET_USE_MPI)
ENDIF (BUILD_TESTING AND ICET_BUILD_TESTING)

ADD_SUBDIRECTORY(doc)

# Save the CMake targets so another project can import them.
EXPORT(TARGETS ${ICET_INSTALL_TARGETS}
  FILE ${ICET_LIBRARY_DIR}/IceTTargets.cmake
  )

# Save IceT specific configuration options.

#First, configuration for build directory.
SET(ICET_INCLUDE_DIRS_CONFIG "${ICET_SOURCE_DIR}/src/include;${ICET_BINARY_DIR}/src/include")
SET(ICET_CORE_LIBRARY_TARGET IceTCore)
IF (ICET_USE_OPENGL)
  SET(ICET_GL_LIBRARY_TARGET IceTGL)
ENDIF (ICET_USE_OPENGL)
IF (ICET_USE_MPI)
  SET(ICET_MPI_LIBRARY_TARGET IceTMPI)
ENDIF (ICET_USE_MPI)
CONFIGURE_FILE(
  ${ICET_SOURCE_DIR}/cmake/IceTConfig.cmake.in
  ${ICET_LIBRARY_DIR}/IceTConfig.cmake
  @ONLY IMMEDIATE
  )

#Second, configuration for install directory.
IF (NOT ICET_INSTALL_NO_DEVELOPMENT)
  SET(ICET_INCLUDE_DIRS_CONFIG "\${_install_dir}/${ICET_INSTALL_INCLUDE_DIR}")
  SET(ICET_CORE_LIBRARY_TARGET IceTCore)
  IF (ICET_USE_OPENGL)
    SET(ICET_GL_LIBRARY_TARGET IceTGL)
  ENDIF (ICET_USE_OPENGL)
  IF (ICET_USE_MPI)
    SET(ICET_MPI_LIBRARY_TARGET IceTMPI)
  ENDIF (ICET_USE_MPI)
  CONFIGURE_FILE(
    ${ICET_SOURCE_DIR}/cmake/IceTConfig.cmake.in
    ${ICET_LIBRARY_DIR}/IceTConfig.cmake.install
    @ONLY IMMEDIATE)
  INSTALL(FILES ${ICET_LIBRARY_DIR}/IceTConfig.cmake.install
    DESTINATION ${ICET_INSTALL_LIB_DIR}
    RENAME IceTConfig.cmake
    )
ENDIF (NOT ICET_INSTALL_NO_DEVELOPMENT)

# Create supplemental version configuration file.
CONFIGURE_FILE(
  ${ICET_SOURCE_DIR}/cmake/IceTConfigVersion.cmake.in
  ${ICET_LIBRARY_DIR}/IceTConfigVersion.cmake
  @ONLY
  )
IF (NOT ICET_INSTALL_NO_DEVELOPMENT)
  INSTALL(FILES ${ICET_LIBRARY_DIR}/IceTConfigVersion.cmake
    DESTINATION ${ICET_INSTALL_LIB_DIR}
    )
ENDIF (NOT ICET_INSTALL_NO_DEVELOPMENT)

# Enable CPack packaging.
SET(CPACK_PACKAGE_DESCRIPTION_FILE ${ICET_SOURCE_DIR}/README.md)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The Image Composition Engine for Tiles")
SET(CPACK_PACKAGE_NAME "IceT")
SET(CPACK_PACKAGE_VENDOR "Sandia National Laboratories")
SET(CPACK_PACKAGE_VERSION_MAJOR ${ICET_MAJOR_VERSION})
SET(CPACK_PACKAGE_VERSION_MINOR ${ICET_MINOR_VERSION})
SET(CPACK_PACKAGE_VERSION_PATCH ${ICET_PATCH_VERSION})
INCLUDE(CPack)

# Allow local additions to this file without CVS conflicts.
INCLUDE(${ICET_BINARY_DIR}/LocalUserOptions.cmake OPTIONAL)
INCLUDE(${ICET_SOURCE_DIR}/LocalUserOptions.cmake OPTIONAL)
