#Setup the version since some files need this
set(VISIT_VERSION "2.7.0")

if (COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

#disable compiler warnings from the bridge
option(VISIT_DISABLE_COMPILER_WARNINGS "Disable compiler warnings" ON)
mark_as_advanced(VISIT_DISABLE_COMPILER_WARNINGS)
if(VISIT_DISABLE_COMPILER_WARNINGS)
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
endif(VISIT_DISABLE_COMPILER_WARNINGS)

#set up some vars we need to compile
set(VISIT_STATIC)
if (BUILD_SHARED_LIBS)
  set(VISIT_STATIC 0)
  add_definitions(-DVISIT_BUILD_SHARED_LIBS)
else(VISIT_STATIC)
  set(VISIT_STATIC 1)
  add_definitions(-DVISIT_STATIC -DGLEW_STATIC)
endif()

set(VISIT_SOURCE_DIR ${VisItBridgePlugin_SOURCE_DIR})
set(VISIT_BINARY_DIR ${VisItBridgePlugin_BINARY_DIR})
set(VISIT_CMAKE_DIR ${VISIT_SOURCE_DIR}/CMake )

#include the visit cmake directory on the cmake search path
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${VISIT_CMAKE_DIR})

#include the visit install target and plugin function
MACRO(VISIT_INSTALL_TARGETS target)
# Removed to let vtk modular take care of installation
ENDMACRO(VISIT_INSTALL_TARGETS)

FUNCTION(ADD_PARALLEL_LIBRARY target)
    VTK_ADD_LIBRARY(${target} ${ARGN})
    IF(VISIT_PARALLEL_CXXFLAGS)
        SET(PAR_COMPILE_FLAGS "")
        FOREACH (X ${VISIT_PARALLEL_CXXFLAGS})
            SET(PAR_COMPILE_FLAGS "${PAR_COMPILE_FLAGS} ${X}")
        ENDFOREACH(X)
        SET_TARGET_PROPERTIES(${target} PROPERTIES
            COMPILE_FLAGS ${PAR_COMPILE_FLAGS}
        )
        IF(VISIT_PARALLEL_LINKER_FLAGS)
            SET(PAR_LINK_FLAGS "")
            FOREACH(X ${VISIT_PARALLEL_LINKER_FLAGS})
                SET(PAR_LINK_FLAGS "${PAR_LINK_FLAGS} ${X}")
            ENDFOREACH(X)
            SET_TARGET_PROPERTIES(${target} PROPERTIES
                LINK_FLAGS ${PAR_LINK_FLAGS}
            )
        ENDIF(VISIT_PARALLEL_LINKER_FLAGS)

        IF(VISIT_PARALLEL_RPATH)
            SET(PAR_RPATHS "")
            FOREACH(X ${CMAKE_INSTALL_RPATH})
                SET(PAR_RPATHS "${PAR_RPATHS} ${X}")
            ENDFOREACH(X)
            FOREACH(X ${VISIT_PARALLEL_RPATH})
                SET(PAR_RPATHS "${PAR_RPATHS} ${X}")
            ENDFOREACH(X)
            SET_TARGET_PROPERTIES(${target} PROPERTIES
                INSTALL_RPATH ${PAR_RPATHS}
            )
        ENDIF(VISIT_PARALLEL_RPATH)

        IF(NOT VISIT_NOLINK_MPI_WITH_LIBRARIES)
            target_link_libraries(${target} ${VISIT_PARALLEL_LIBS})
        ENDIF(NOT VISIT_NOLINK_MPI_WITH_LIBRARIES)
    ENDIF(VISIT_PARALLEL_CXXFLAGS)
ENDFUNCTION(ADD_PARALLEL_LIBRARY)

#called from readers that are being built into paraview
FUNCTION(ADD_VISIT_READER NAME VERSION)
  set(PLUGIN_NAME "vtk${NAME}")
  set(PLUGIN_VERSION "${VERSION}")
  set(ARG_VISIT_READER_NAME)
  set(ARG_VISIT_INCLUDE_NAME)
  set(ARG_VISIT_READER_TYPE)
  set(ARG_VISIT_READER_USES_OPTIONS OFF)
  set(ARG_SERVER_SOURCES)
  set(ARG_VISIT_READER_OPTIONS_NAME)

  PV_PLUGIN_PARSE_ARGUMENTS(ARG
    "VISIT_READER_NAME;VISIT_INCLUDE_NAME;VISIT_READER_TYPE;VISIT_READER_USES_OPTIONS;SERVER_SOURCES;VISIT_READER_OPTIONS_NAME"
      "" ${ARGN} )
  #check reader types
  string(REGEX MATCH "^[SM]T[SM]D$" VALID_READER_TYPE ${ARG_VISIT_READER_TYPE})

  if ( NOT VALID_READER_TYPE)
    MESSAGE(FATAL_ERROR "Invalid Reader Type. Valid Types are STSD, STMD, MTSD, MTMD")
  endif()

  #if the user hasn't defined an include name, we presume the reader name
  #is also the include name
  if(NOT ARG_VISIT_INCLUDE_NAME)
    set(ARG_VISIT_INCLUDE_NAME ${ARG_VISIT_READER_NAME})
  endif()

  if(ARG_VISIT_READER_USES_OPTIONS)
    #determine the name of the plugin info class by removing the
    #avt from the start and the FileFormat from the back
    if(ARG_VISIT_READER_OPTIONS_NAME)
      string(REGEX REPLACE "^avt|FileFormat$" "" TEMP_NAME ${ARG_VISIT_READER_OPTIONS_NAME})
    else()
      string(REGEX REPLACE "^avt|FileFormat$" "" TEMP_NAME ${ARG_VISIT_READER_NAME})
    endif()
    set(ARG_VISIT_PLUGIN_INFO_HEADER ${TEMP_NAME}PluginInfo)
    set(ARG_VISIT_PLUGIN_INFO_CLASS ${TEMP_NAME}CommonPluginInfo)
  endif()

  set(XML_NAME ${NAME})
  set(LIBRARY_NAME "vtkVisItDatabases")
  #need to generate the VTK class wrapper
  string(SUBSTRING ${ARG_VISIT_READER_TYPE} 0 2 READER_WRAPPER_TYPE)

  configure_file(
    ${VISIT_CMAKE_DIR}/VisItExport.h.in
    ${VISIT_DATABASE_BINARY_DIR}/${PLUGIN_NAME}Export.h @ONLY)

  configure_file(
      ${VISIT_CMAKE_DIR}/VisIt${READER_WRAPPER_TYPE}.h.in
      ${VISIT_DATABASE_BINARY_DIR}/${PLUGIN_NAME}.h @ONLY)
  configure_file(
      ${VISIT_CMAKE_DIR}/VisIt${READER_WRAPPER_TYPE}.cxx.in
      ${VISIT_DATABASE_BINARY_DIR}/${PLUGIN_NAME}.cxx @ONLY)

  set(reader_sources
  ${VISIT_DATABASE_BINARY_DIR}/${PLUGIN_NAME}.cxx
  ${VISIT_DATABASE_BINARY_DIR}/${PLUGIN_NAME}.h
    )

  #fix up the arg_server_sources path for compilation
  set(ABS_SERVER_SOURCES "")
  foreach(SRC_FILENAME ${ARG_SERVER_SOURCES})
    list(APPEND ABS_SERVER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FILENAME}")
  endforeach()

  set(VISIT_SERVER_SOURCES ${VISIT_SERVER_SOURCES} ${reader_sources}
    CACHE INTERNAL "vtk classes to wrap for client server")
  set(VISIT_DB_SOURCES ${VISIT_DB_SOURCES} ${ABS_SERVER_SOURCES}
    CACHE INTERNAL "visit sources for readers")
  set(VISIT_DB_INC_DIRS ${VISIT_DB_INC_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE INTERNAL "include directories")
ENDFUNCTION(ADD_VISIT_READER)

#called from readers that are being built into paraview
FUNCTION(ADD_VISIT_INTERFACE_READER NAME VERSION)

  set(INTERFACE_NAME "vtk${NAME}")
  set(INTERFACE_VERSION "${VERSION}")
  set(ARG_VISIT_READER_NAMES)
  set(ARG_VISIT_READER_TYPES)
  set(ARG_VISIT_READER_INCLUDES)
  set(ARG_VISIT_INTERFACE_CALL)
  set(ARG_VISIT_INTERFACE_FILE)
  set(ARG_VISIT_INTERFACE_EXEMPT_CLASSES)
  set(ARG_VISIT_READER_FILE_PATTERN)
  set(ARG_SERVER_SOURCES)
  PV_PLUGIN_PARSE_ARGUMENTS(ARG
  "VISIT_READER_NAMES;VISIT_READER_TYPES;VISIT_READER_INCLUDES;VISIT_INTERFACE_CALL;VISIT_INTERFACE_FILE;VISIT_INTERFACE_EXEMPT_CLASSES;SERVER_SOURCES"
    "" ${ARGN} )

  if (NOT ARG_VISIT_INTERFACE_CALL OR NOT ARG_VISIT_INTERFACE_FILE )
    MESSAGE(FATAL_ERROR "The macro file for the file interface needs to be defined.")
  endif(NOT ARG_VISIT_INTERFACE_CALL OR NOT ARG_VISIT_INTERFACE_FILE)

  #if the user hasn't defined an include name, we presume the reader name
  #is also the include name
  if(NOT ARG_VISIT_INCLUDE_NAME)
    set(ARG_VISIT_INCLUDE_NAME ${ARG_VISIT_READER_NAME})
  endif()

  set(LIBRARY_NAME "vtkVisItDatabases")

  list(LENGTH ARG_VISIT_READER_NAMES NUM_READERS)
  foreach( index RANGE ${NUM_READERS})
    if ( index LESS NUM_READERS )
      list(GET ARG_VISIT_READER_NAMES ${index} ARG_VISIT_READER_NAME)
      list(GET ARG_VISIT_READER_TYPES ${index} ARG_VISIT_READER_TYPE)
      list(GET ARG_VISIT_READER_INCLUDES ${index} ARG_VISIT_INCLUDE_NAME)

      #need to set up the vars needed by the configures
      string(REGEX REPLACE "^avt|FileFormat$" "" TEMP_NAME ${ARG_VISIT_READER_NAME})
      set(PLUGIN_NAME "vtkVisIt${TEMP_NAME}Reader")
      set(XML_NAME "VisItVisIt${TEMP_NAME}Reader")


      #need to generate the VTK class wrapper
      string(SUBSTRING ${ARG_VISIT_READER_TYPE} 0 2 READER_WRAPPER_TYPE)

      #determine if this file is exempt from the interface CanReadFile macro
      list(FIND ARG_VISIT_INTERFACE_EXEMPT_CLASSES ${ARG_VISIT_READER_NAME} EXEMPT_READER)
      if ( EXEMPT_READER EQUAL -1 )
        set(VISIT_READER_USES_INTERFACE ON)
      else( EXEMPT_READER EQUAL -1 )
        set(VISIT_READER_USES_INTERFACE OFF)
      endif( EXEMPT_READER EQUAL -1 )

      #we have to configure the macro file
      configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_VISIT_INTERFACE_FILE}.h
        ${VISIT_DATABASE_BINARY_DIR}/${PLUGIN_NAME}${ARG_VISIT_INTERFACE_FILE}.h @ONLY)

      configure_file(
        ${VISIT_CMAKE_DIR}/VisItExport.h.in
        ${VISIT_DATABASE_BINARY_DIR}/${PLUGIN_NAME}Export.h @ONLY)

      configure_file(
        ${VISIT_CMAKE_DIR}/VisIt${READER_WRAPPER_TYPE}.h.in
        ${VISIT_DATABASE_BINARY_DIR}/${PLUGIN_NAME}.h @ONLY)

      configure_file(
        ${VISIT_CMAKE_DIR}/VisIt${READER_WRAPPER_TYPE}.cxx.in
        ${VISIT_DATABASE_BINARY_DIR}/${PLUGIN_NAME}.cxx @ONLY)

      set(reader_sources
        ${VISIT_DATABASE_BINARY_DIR}/${PLUGIN_NAME}.cxx
        ${VISIT_DATABASE_BINARY_DIR}/${PLUGIN_NAME}.h)


      set(VISIT_SERVER_SOURCES ${VISIT_SERVER_SOURCES} ${reader_sources}
        CACHE INTERNAL "vtk classes to wrap for client server")

    endif(index LESS NUM_READERS)
  endforeach(index)

  #fix up the arg_server_sources path for compilation
  set(ABS_SERVER_SOURCES "")
  foreach(SRC_FILENAME ${ARG_SERVER_SOURCES})
    list(APPEND ABS_SERVER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FILENAME}")
  endforeach()

  set(VISIT_DB_SOURCES ${VISIT_DB_SOURCES} ${ABS_SERVER_SOURCES}
    CACHE INTERNAL "visit sources for readers")

  set(VISIT_DB_INC_DIRS ${VISIT_DB_INC_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE INTERNAL "include directories")


ENDFUNCTION(ADD_VISIT_INTERFACE_READER)

#Used for readers that are plugins for paraview
FUNCTION(ADD_VISIT_PLUGIN_READER NAME VERSION)
set(PLUGIN_NAME "vtk${NAME}")
set(PLUGIN_VERSION "${VERSION}")
set(ARG_VISIT_READER_NAME)
set(ARG_VISIT_INCLUDE_NAME)
set(ARG_VISIT_READER_TYPE)
set(ARG_VISIT_READER_FILE_PATTERN)
set(ARG_VISIT_READER_USES_OPTIONS OFF)
set(ARG_SERVER_SOURCES)

PV_PLUGIN_PARSE_ARGUMENTS(ARG
  "VISIT_READER_NAME;VISIT_INCLUDE_NAME;VISIT_READER_TYPE;VISIT_READER_FILE_PATTERN;VISIT_READER_USES_OPTIONS;SERVER_SOURCES"
    "" ${ARGN} )
#check reader types
string(REGEX MATCH "^[SM]T[SM]D$" VALID_READER_TYPE ${ARG_VISIT_READER_TYPE})

if ( NOT VALID_READER_TYPE)
  MESSAGE(FATAL_ERROR "Invalid Reader Type. Valid Types are STSD, STMD, MTSD, MTMD")
endif()

#if the user hasn't defined an include name, we presume the reader name
#is also the include name
if(NOT ARG_VISIT_INCLUDE_NAME)
  set(ARG_VISIT_INCLUDE_NAME ${ARG_VISIT_READER_NAME})
endif()

MESSAGE(STATUS "Generating wrappings for ${PLUGIN_NAME}")
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${VISITBRIDGE_INCLUDE_DIRS}
  )

if(ARG_VISIT_READER_USES_OPTIONS)
  #determine the name of the plugin info class by removing the
  #avt from the start and the FileFormat from the back
  string(REGEX REPLACE "^avt|FileFormat$" "" TEMP_NAME ${ARG_VISIT_READER_NAME})
  set(ARG_VISIT_PLUGIN_INFO_HEADER ${TEMP_NAME}PluginInfo)
  set(ARG_VISIT_PLUGIN_INFO_CLASS ${TEMP_NAME}CommonPluginInfo)
endif()

set(XML_NAME ${NAME})
set(LIBRARY_NAME ${NAME})
#need to generate the VTK class wrapper
string(SUBSTRING ${ARG_VISIT_READER_TYPE} 0 2 READER_WRAPPER_TYPE)
configure_file(
    ${VISIT_CMAKE_DIR}/VisItExport.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}Export.h @ONLY)
configure_file(
    ${VISIT_CMAKE_DIR}/VisIt${READER_WRAPPER_TYPE}.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.h @ONLY)
configure_file(
    ${VISIT_CMAKE_DIR}/VisIt${READER_WRAPPER_TYPE}.cxx.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.cxx @ONLY)

#generate server manager xml file
configure_file(
  ${VISIT_CMAKE_DIR}/VisIt${READER_WRAPPER_TYPE}SM.xml.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}SM.xml @ONLY)

set(reader_sources
  ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.h
    )
set(reader_server_xml
  ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}SM.xml
  )

#add the vtk classes to the argument list
set(PV_ARGS ${ARGN})
list(APPEND PV_ARGS "SERVER_MANAGER_SOURCES;${reader_sources}")

#now we need to add the XML info
list(APPEND PV_ARGS "SERVER_MANAGER_XML;${reader_server_xml}")

#all of the readers can be server side only
list(APPEND PV_ARGS "REQUIRED_ON_SERVER")

ADD_PARAVIEW_PLUGIN( ${NAME} ${VERSION} ${PV_ARGS} )
ENDFUNCTION(ADD_VISIT_PLUGIN_READER)

FUNCTION(ADD_VISIT_INTERFACE_PLUGIN_READER NAME VERSION)

set(INTERFACE_NAME "vtk${NAME}")
set(INTERFACE_VERSION "${VERSION}")
set(ARG_VISIT_READER_NAMES)
set(ARG_VISIT_READER_TYPES)
set(ARG_VISIT_READER_INCLUDES)
set(ARG_VISIT_INTERFACE_CALL)
set(ARG_VISIT_INTERFACE_FILE)
set(ARG_VISIT_INTERFACE_EXEMPT_CLASSES)
set(ARG_VISIT_READER_FILE_PATTERN)
set(ARG_SERVER_SOURCES)

PV_PLUGIN_PARSE_ARGUMENTS(ARG
  "VISIT_READER_NAMES;VISIT_READER_TYPES;VISIT_READER_INCLUDES;VISIT_INTERFACE_CALL;VISIT_INTERFACE_FILE;VISIT_INTERFACE_EXEMPT_CLASSES;VISIT_READER_FILE_PATTERN;SERVER_SOURCES"
    "" ${ARGN} )

if ( NOT ARG_VISIT_INTERFACE_CALL OR NOT ARG_VISIT_INTERFACE_FILE )
  MESSAGE(FATAL_ERROR "The macro file for the file interface needs to be defined.")
endif()


message(STATUS "Generating wrappings for ${INTERFACE_NAME}")
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${VISITBRIDGE_INCLUDE_DIRS}
  )

#check reader types
set(INTERFACE_SOURCES)
set(INTERFACE_SMXML)
set(INTERFACE_GUIXML)
list(LENGTH ARG_VISIT_READER_NAMES NUM_READERS)
foreach( index RANGE ${NUM_READERS})
  if ( index LESS NUM_READERS )
    list(GET ARG_VISIT_READER_NAMES ${index} ARG_VISIT_READER_NAME)
    list(GET ARG_VISIT_READER_TYPES ${index} ARG_VISIT_READER_TYPE)
    list(GET ARG_VISIT_READER_INCLUDES ${index} ARG_VISIT_INCLUDE_NAME)

    #need to set up the vars needed by the configures
    string(REGEX REPLACE "^avt|FileFormat$" "" TEMP_NAME ${ARG_VISIT_READER_NAME})
    set(PLUGIN_NAME "vtk${TEMP_NAME}Reader")
    set(XML_NAME "VisIt${TEMP_NAME}Reader")

    #need to generate the VTK class wrapper
    string(SUBSTRING ${ARG_VISIT_READER_TYPE} 0 2 READER_WRAPPER_TYPE)

    #determine if this file is exempt from the interface CanReadFile macro
    list(FIND ARG_VISIT_INTERFACE_EXEMPT_CLASSES ${ARG_VISIT_READER_NAME} EXEMPT_READER)
    if ( EXEMPT_READER EQUAL -1 )
      set(VISIT_READER_USES_INTERFACE ON)
    else()
      set(VISIT_READER_USES_INTERFACE OFF)
    endif()

    #we have to configure the macro file
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_VISIT_INTERFACE_FILE}.h
        ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}${ARG_VISIT_INTERFACE_FILE}.h @ONLY)

    #configure the declspec header
    configure_file(
        ${VISIT_CMAKE_DIR}/VisItExport.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}Export.h @ONLY)

    #configure the header and implementation
    configure_file(
        ${VISIT_CMAKE_DIR}/VisIt${READER_WRAPPER_TYPE}.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.h @ONLY)
    configure_file(
        ${VISIT_CMAKE_DIR}/VisIt${READER_WRAPPER_TYPE}.cxx.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.cxx @ONLY)



    #generate server manager xml file
    configure_file(
      ${VISIT_CMAKE_DIR}/VisIt${READER_WRAPPER_TYPE}SM.xml.in
      ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}SM.xml @ONLY)

    #generate reader xml
    configure_file(
      ${VISIT_CMAKE_DIR}/VisItGUI.xml.in
      ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}GUI.xml @ONLY)

    LIST(APPEND INTERFACE_SOURCES
      ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.cxx
      ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.h
      )
    LIST(APPEND INTERFACE_SMXML
      ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}SM.xml
      )
    LIST(APPEND INTERFACE_GUIXML
      ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}GUI.xml
      )

  endif()
endforeach( index )

#add the vtk classes to the argument list
set(PV_ARGS ${ARGN})
list(APPEND PV_ARGS "SERVER_MANAGER_SOURCES;${INTERFACE_SOURCES}")

#now we need to add the XML info
list(APPEND PV_ARGS "SERVER_MANAGER_XML;${INTERFACE_SMXML}")

#all of the readers can be server side only
list(APPEND PV_ARGS "REQUIRED_ON_SERVER")

ADD_PARAVIEW_PLUGIN( ${NAME} ${VERSION} ${PV_ARGS} )

ENDFUNCTION(ADD_VISIT_INTERFACE_PLUGIN_READER)

#need to grab some helper macros from paraview plugin
include(ParaViewPlugins)

#set up MPI
set(VISIT_PARALLEL ${PARAVIEW_USE_MPI})
if(PARAVIEW_USE_MPI)
  include(FindMPI)
  include_directories(
    ${MPI_INCLUDE_PATH}
    )
  set(VISIT_PARALLEL_LIBS ${MPI_LIBRARY})
  if(MPI_EXTRA_LIBRARY)
    set(VISIT_PARALLEL_LIBS ${VISIT_PARALLEL_LIBS} ${MPI_EXTRA_LIBRARY})
  endif(MPI_EXTRA_LIBRARY)
endif(PARAVIEW_USE_MPI)

# setup to use vtkzlib
set(ZLIB_LIB ${vtkzlib_LIBRARIES})

#block out most of the warnings in visit on windows
if (WIN32)
  add_definitions(-D_USE_MATH_DEFINES)
  if (MSVC_VERSION GREATER 1400)
      add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
      add_definitions(-D_SCL_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE)
      add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  endif (MSVC_VERSION GREATER 1400)
endif(WIN32)

#-----------------------------------------------------------------------------
# Setup ParaView and Common includes before the visit-config.h so that
# we can use utilities like HDF5 from ParaView
#-----------------------------------------------------------------------------


# Set up easy to use includes for the common directory
set(VISIT_COMMON_INCLUDES
    ${VISIT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common/Exceptions/Database
    ${VISIT_SOURCE_DIR}/common/Exceptions/Pipeline
    ${VISIT_SOURCE_DIR}/common/Exceptions/Plotter
    ${VISIT_SOURCE_DIR}/common/comm
    ${VISIT_SOURCE_DIR}/common/expr
    ${VISIT_SOURCE_DIR}/common/icons
    ${VISIT_SOURCE_DIR}/common/misc
    ${VISIT_SOURCE_DIR}/common/parser
    ${VISIT_SOURCE_DIR}/common/plugin
    ${VISIT_SOURCE_DIR}/common/proxybase
    ${VISIT_SOURCE_DIR}/common/state
    ${VISIT_SOURCE_DIR}/common/utility
    ${VISIT_SOURCE_DIR}/common/common/misc
    ${VISIT_SOURCE_DIR}/common/common/plugin
    ${VISIT_SOURCE_DIR}/common/common/state
    ${VISIT_SOURCE_DIR}/common/common/utility
  )

#watch out, you need to make sure common/parser is always in front of
# python2.X includes
include_directories(BEFORE ${VISIT_COMMON_INCLUDES})
include_directories(${VTK_INCLUDE_DIR})
include_directories(${HDF5_INCLUDE_DIR})

set(VTK_BINARY_DIR ${PARAVIEW_VTK_DIR} )

#-----------------------------------------------------------------------------
# Setup Vars for visit-config.h
#-----------------------------------------------------------------------------


set(VISIT_DBIO_ONLY ON) #Build only visitconvert and engine plugins
if(VISIT_DBIO_ONLY)
    add_definitions(-DDBIO_ONLY)
endif(VISIT_DBIO_ONLY)

#Check to see if ofstreams rdbuf is public. If it is NOT public set NO_SETBUF
try_compile(VISIT_COMPILER_FSTREAM_WORKAROUND
        ${CMAKE_CURRENT_BINARY_DIR}/CMakeTmp
        ${VISIT_CMAKE_DIR}/testFStream.cxx)
if (NOT VISIT_COMPILER_FSTREAM_WORKAROUND)
   set(NO_SETBUF 1)
endif (NOT VISIT_COMPILER_FSTREAM_WORKAROUND)

# Set the slash characters based on the platform
if(WIN32)
    set(VISIT_SLASH_CHAR   "'\\\\'")
    set(VISIT_SLASH_STRING "\"\\\\\"")
else(WIN32)
    set(VISIT_SLASH_CHAR   "'/'")
    set(VISIT_SLASH_STRING "\"/\"")
endif(WIN32)

# Check for plugin extension
if(VISIT_STATIC)
  if(WIN32)
    set(VISIT_PLUGIN_EXTENSION   ".dll")
  else(WIN32)
    set(VISIT_PLUGIN_EXTENSION   ".lib")
  endif(WIN32)
else(VISIT_STATIC)
    if(WIN32)
        set(VISIT_PLUGIN_EXTENSION   ".dll")
    else(WIN32)
        if(APPLE)
            set(VISIT_PLUGIN_EXTENSION   ".dylib")
        else(APPLE)
            set(VISIT_PLUGIN_EXTENSION   ".so")
        endif(APPLE)
    endif(WIN32)
endif(VISIT_STATIC)

#-----------------------------------------------------------------------------
# Setup lib settings
#-----------------------------------------------------------------------------

# Set up boost (interval) library
find_package(Boost 1.40 REQUIRED) #only need header libraries
if (Boost_Found)
  set(HAVE_BILIB 1)
else (Boost_Found)
  set(HAVE_BILIB 0)
endif(Boost_Found)


#setup non third party vtk utilities
set(HAVE_LIBHDF5 1)
set(HAVE_LIBNETCDF 1)
set(HAVE_NETCDF_H 1)
set(HAVE_LIBEXODUSII 1)

# Setup SILO
find_package(SILO QUIET)
if(SILO_FOUND)
  set(HAVE_LIBSILO ${SILO_FOUND})
endif(SILO_FOUND)

# Setup CGNS
find_package(CGNS QUIET)
if(CGNS_FOUND)
  set(HAVE_LIBCGNS ${CGNS_FOUND})
endif(CGNS_FOUND)

# Setup Mili
find_package(MILI QUIET)
if(MILI_FOUND)
  set(HAVE_LIBMILI ${MILI_FOUND})
endif(MILI_FOUND)

#-----------------------------------------------------------------------------
# Detect packages here. We could probably write macros that we can include from
# elsewhere for this.
#-----------------------------------------------------------------------------
include(CheckIncludeFiles)
include(CMakeBackwardCompatibilityC)
include(CMakeBackwardCompatibilityCXX)
include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(TestBigEndian)
include(FindOpenGL)

check_include_files (fcntl.h     HAVE_FCNTL_H)
check_include_files (inttypes.h  HAVE_INTTYPES_H)
check_include_files (malloc.h    HAVE_MALLOC_H)
check_include_files (limits.h    HAVE_LIMITS_H)
check_include_files (memory.h    HAVE_MEMORY_H)
check_include_files (stdint.h    HAVE_STDINT_H)
check_include_files (stdlib.h    HAVE_STDLIB_H)
check_include_files (strings.h   HAVE_STRINGS_H)
check_include_files (string.h    HAVE_STRING_H)
check_include_files (sys/time.h  HAVE_SYS_TIME_H)
check_include_files (sys/types.h HAVE_SYS_TYPES_H)
check_include_files (sys/stat.h  HAVE_SYS_STAT_H)
check_include_files (unistd.h    HAVE_UNISTD_H)
check_include_files (stdbool.h   HAVE_STDBOOL_H)

# Check for type sizes, endian
set(SIZEOF_BOOLEAN              ${CMAKE_SIZEOF_BOOLEAN})
set(SIZEOF_CHAR                 ${CMAKE_SIZEOF_CHAR})
set(SIZEOF_DOUBLE               ${CMAKE_SIZEOF_DOUBLE})
set(SIZEOF_FLOAT                ${CMAKE_SIZEOF_FLOAT})
set(SIZEOF_INT                  ${CMAKE_SIZEOF_INT})
set(SIZEOF_LONG                 ${CMAKE_SIZEOF_LONG})
set(SIZEOF_LONG_DOUBLE          ${CMAKE_SIZEOF_LONG_DOUBLE})
set(SIZEOF_LONG_FLOAT           ${CMAKE_SIZEOF_LONG_FLOAT})
set(SIZEOF_LONG_LONG            ${CMAKE_SIZEOF_LONG_LONG})
set(SIZEOF_SHORT                ${CMAKE_SIZEOF_SHORT})
set(SIZEOF_UNSIGNED_CHAR        ${CMAKE_SIZEOF_UNSIGNED_CHAR})
set(SIZEOF_UNSIGNED_INT         ${CMAKE_SIZEOF_UNSIGNED_INT})
set(SIZEOF_UNSIGNED_LONG        ${CMAKE_SIZEOF_UNSIGNED_LONG})
set(SIZEOF_UNSIGNED_LONG_LONG   ${CMAKE_SIZEOF_UNSIGNED_LONG_LONG})
set(SIZEOF_UNSIGNED_SHORT       ${CMAKE_SIZEOF_UNSIGNED_SHORT})
set(SIZEOF_VOID_P               ${CMAKE_SIZEOF_VOID_P})
set(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE)
check_type_size("off64_t" SIZEOF_OFF64_T)
test_big_endian(WORDS_BIGENDIAN)

# manually check for socklen_t as CHECK_SYMBOL_EXISTS
# doesn't appear to work on linux (at least)
try_compile(HAVE_SOCKLEN_T
    ${CMAKE_CURRENT_BINARY_DIR}
    ${VISIT_SOURCE_DIR}/CMake/TestSocklenT.c
    OUTPUT_VARIABLE SLT
)
if (HAVE_SOCKLEN_T)
    set(HAVE_SOCKLEN_T 1 CACHE INTERNAL "support for socklen_t")
else(HAVE_SOCKLEN_T)
    set(HAVE_SOCKLEN_T 0 CACHE INTERNAL "support for socklen_t")
endif (HAVE_SOCKLEN_T)
