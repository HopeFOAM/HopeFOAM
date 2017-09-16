cmake_minimum_required(VERSION 2.6.3)

FOREACH(policy CMP0012 CMP0013 CMP0014 CMP0017)
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()

project(VisItBridgePlugin)

include(${CMAKE_CURRENT_SOURCE_DIR}/CMake/VisItCommon.cmake)

#-----------------------------------------------------------------------------
# Create visit-config.h
#-----------------------------------------------------------------------------

if(WIN32)
  add_definitions(-DWIN32)
endif()

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/include/visit-cmake.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/visit-config.h @ONLY IMMEDIATE)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/common/misc/ConfigureInfo.C.in
               ${CMAKE_CURRENT_BINARY_DIR}/common/misc/ConfigureInfo.C @ONLY IMMEDIATE)


add_subdirectory(common)
add_subdirectory(visit_vtk)
add_subdirectory(avt)

set(VISITBRIDGE_INCLUDE_DIRS
  ${vtkVisItAVTAlgorithms_BINARY_DIR}
  ${vtkVisItAVTAlgorithms_SOURCE_DIR}
  ${VISIT_COMMON_INCLUDES}
  ${VISIT_SOURCE_DIR}/include
  ${AVT_DATABASE_SOURCE_DIR}/Database
  ${AVT_DATABASE_SOURCE_DIR}/Ghost
  ${AVT_DATABASE_SOURCE_DIR}/Formats
  ${AVT_DBATTS_SOURCE_DIR}/MetaData
  ${AVT_DBATTS_SOURCE_DIR}/SIL
  ${VISIT_SOURCE_DIR}/avt/Math
  ${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
  ${VISIT_SOURCE_DIR}/avt/IVP
  ${AVT_PIPELINE_SOURCE_DIR}/AbstractFilters
  ${AVT_PIPELINE_SOURCE_DIR}/Data
  ${AVT_PIPELINE_SOURCE_DIR}/Pipeline
  ${AVT_PIPELINE_SOURCE_DIR}/Sinks
  ${AVT_PIPELINE_SOURCE_DIR}/Sources
  ${VISIT_SOURCE_DIR}/visit_vtk/full
  ${VISIT_SOURCE_DIR}/visit_vtk/lightweight
  ${Boost_INCLUDE_DIR}
  ${VTK_INCLUDE_DIR}
  CACHE INTERNAL ""
  )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/VisItBridgeUse.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/VisItBridgeUse.cmake @ONLY IMMEDIATE)

#######################################################################
# INSTALL
if (NOT VTK_INSTALL_NO_DEVELOPMENT)
  set(BUILD_TREE ${ParaView_BINARY_DIR})

  #install
  set(cmakeDevFiles
    ${VISIT_CMAKE_DIR}/VisItCommon.cmake
    ${VISIT_CMAKE_DIR}/VisItExport.h.in
    ${VISIT_CMAKE_DIR}/VisItMT.h.in
    ${VISIT_CMAKE_DIR}/VisItMT.cxx.in
    ${VISIT_CMAKE_DIR}/VisItMTSM.xml.in
    ${VISIT_CMAKE_DIR}/VisItST.h.in
    ${VISIT_CMAKE_DIR}/VisItST.cxx.in
    ${VISIT_CMAKE_DIR}/VisItSTSM.xml.in)

  install(
    FILES ${cmakeDevFiles}
    DESTINATION ${VTK_INSTALL_PACKAGE_DIR}/VisItLib
    COMPONENT Development)

  set(_temp_VISITBRIDGE_INCLUDE_DIRS ${VISITBRIDGE_INCLUDE_DIRS})
  set(VISITBRIDGE_INCLUDE_DIRS "\${ParaView_INSTALL_PREFIX}/${PV_INSTALL_INCLUDE_DIR}")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/VisItBridgeUse.cmake.in
                "${BUILD_TREE}/CMake/tmp/VisItBridgeUse.cmake" @ONLY IMMEDIATE)
  install(
        FILES "${BUILD_TREE}/CMake/tmp/VisItBridgeUse.cmake"
        DESTINATION ${VTK_INSTALL_PACKAGE_DIR}/VisItLib
        COMPONENT Development)

  set(VISITBRIDGE_INCLUDE_DIRS "${_temp_VISITBRIDGE_INCLUDE_DIRS}" CACHE INTERNAL "")
endif()
