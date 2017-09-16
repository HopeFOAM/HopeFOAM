#get same cmake config as visitlib itself does
include(${CMAKE_CURRENT_SOURCE_DIR}/../Library/VisItLib/CMake/VisItCommon.cmake)
include(${VISIT_BINARY_DIR}/VisItBridgeUse.cmake)

macro(VISIT_INCLUDE_READER_SOURCE name state)
option(VISIT_BUILD_READER_${name} "Build VisIt Reader ${name}" ${state})
if (VISIT_BUILD_READER_${name})
  add_subdirectory(${name})
endif()
endmacro(VISIT_INCLUDE_READER_SOURCE name)

#these are used by visit_include_reader
set(VISIT_DATABASE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(VISIT_SERVER_SOURCES CACHE INTERNAL "vtk classes to wrap for client server" FORCE)
set(VISIT_DB_SOURCES "" CACHE INTERNAL "visit sources for readers" FORCE)
set(VISIT_DB_INC_DIRS CACHE INTERNAL "include directories" FORCE)

set(DEFAULT_BRIDGE_READERS
  ANSYS
  AUXFile
  BOV
  CEAucd
  Chombo
  CMAT
  CTRL
  Curve2D
  DDCMD
  Dyna3D
  Enzo
  ExtrudedVol
  FLASH
  Fluent
  Gadget
  GGCM
  GTC
  GULP
  H5Nimrod
  Image
  LAMMPS
  Lines
  M3D
  M3DC1
  MFIX
  MFIXCDF
  Miranda
  MM5
  NASTRAN
  NETCDF
  Nek5000
  OpenFOAM
  OVERFLOW
  paraDIS
  PATRAN
  PFLOTRAN
  Pixie
  PLOT2D
  PLOT3D
  Point3D
  ProteinDataBank
  RAW
  SAMRAI
  SAR
  SAS
  Spheral
  Tecplot
  Tetrad
  TFT
  TSurf
  UNIC
  VASP
  Velodyne
  Vs
  Xmdv
  XYZ
  )
if(NOT WIN32)
  list(APPEND DEFAULT_BRIDGE_READERS
    Claw
    )
endif(NOT WIN32)

#add generic vtkreader interface parent classes
add_subdirectory(AvtAlgorithms)

#add the default readers to the library
foreach(db ${DEFAULT_BRIDGE_READERS})
  add_subdirectory(${db})
endforeach(db)

add_definitions("-DRC_CPP_VISIT_BUILD")


#-----------------------------------------------------------------------------
# GMV Reader needs OpenGLU library. So check if it exists before enabling the
# GMV reader.
include(vtkOpenGL)
set (__gmv_default_status OFF)
if (OPENGL_GLU_FOUND)
  # BUG #13489. There are cases in FindOpenGL.cmake when OPENGL_GLU_FOUND is not
  # set at all. Hence, don't use ${OPENGL_GLU_FOUND} directly.
  set (__gmv_default_status ON)
endif()
visit_include_reader_source(GMV ${__gmv_default_status})

if (VISIT_BUILD_READER_GMV AND NOT OPENGL_GLU_FOUND)
  message(FATAL_ERROR "Unable to locate GLU library required for GMV reader.")
endif()

#-----------------------------------------------------------------------------
visit_include_reader_source(Silo OFF)
if(VISIT_BUILD_READER_Silo)
  if(NOT HAVE_LIBSILO)
    #if we currently don't have silo setup make it required
    find_package(SILO REQUIRED)
  endif(NOT HAVE_LIBSILO)
  if(NOT SILO_FOUND)
    message(FATAL_ERROR "Unable to locate Silo library.")
  endif(NOT SILO_FOUND)
endif(VISIT_BUILD_READER_Silo)


#-----------------------------------------------------------------------------
visit_include_reader_source(CGNS OFF)
if(VISIT_BUILD_READER_CGNS)
  if(NOT HAVE_LIBCGNS)
    #if we currently don't have silo setup make it required
    find_package(CGNS REQUIRED)
  endif(NOT HAVE_LIBCGNS)
  if(NOT CGNS_FOUND)
    message(FATAL_ERROR "Unable to locate CGNS library.")
  endif(NOT CGNS_FOUND)
endif(VISIT_BUILD_READER_CGNS)

#-----------------------------------------------------------------------------
visit_include_reader_source(Mili OFF)
if(VISIT_BUILD_READER_Mili)
  if(NOT HAVE_LIBMILI)
    #if we currently don't have silo setup make it required
    find_package(MILI REQUIRED)
  endif(NOT HAVE_LIBMILI)
  if(NOT MILI_FOUND)
    message(FATAL_ERROR "Unable to locate Mili library.")
  endif(NOT MILI_FOUND)
endif(VISIT_BUILD_READER_Mili)

#-----------------------------------------------------------------------------
#include all the db directories before
#the visit files
include_directories(BEFORE
  ${VISIT_DB_INC_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/Common
  )

include_directories(
  ${VISITBRIDGE_INCLUDE_DIRS}
  )

if(HAVE_LIBSILO AND VISIT_BUILD_READER_Silo)
  include_directories(${SILO_INCLUDE_DIR})
endif(HAVE_LIBSILO AND VISIT_BUILD_READER_Silo)

if(HAVE_LIBCGNS AND VISIT_BUILD_READER_CGNS)
  include_directories(${CGNS_INCLUDE_DIR})
endif(HAVE_LIBCGNS AND VISIT_BUILD_READER_CGNS)

if(HAVE_LIBMILI AND VISIT_BUILD_READER_Mili)
  include_directories(${MILI_INCLUDE_DIR})
endif(HAVE_LIBMILI AND VISIT_BUILD_READER_Mili)

set(Module_SRCS
  ${VISIT_SERVER_SOURCES}
  )

set_source_files_properties(
  ${VISIT_DB_SOURCES}
  PROPERTIES
    WRAP_EXCLUDE 1
    WRAP_EXCLUDE_PYTHON 1
    SKIP_HEADER_INSTALL 1)

set(vtkIOVisItBridge_INCLUDE_DIRS "${vtkIOVisItBridge_INCLUDE_DIRS};${CMAKE_CURRENT_SOURCE_DIR}/AvtAlgorithms")
vtk_module_library(vtkIOVisItBridge ${Module_SRCS} ${VISIT_DB_SOURCES})

# if you want to add vtk-module dependencies, simply add those in module.cmake,
# rather than explicitly adding those by using target_link_libraries.

if(VISIT_PARALLEL)
   target_link_libraries(vtkIOVisItBridge
        LINK_PRIVATE
          visitcommon avtdatabase_par avtpipeline_par avtivp
          visit_vtk)
else(VISIT_PARALLEL)
  target_link_libraries(vtkIOVisItBridge
        LINK_PRIVATE
          visitcommon avtdatabase_ser avtpipeline_ser avtivp
          visit_vtk)
endif(VISIT_PARALLEL)

if(HAVE_LIBSILO AND VISIT_BUILD_READER_Silo)
  target_link_libraries(vtkIOVisItBridge LINK_PRIVATE ${SILO_LIBRARIES})
endif(HAVE_LIBSILO AND VISIT_BUILD_READER_Silo)

if(HAVE_LIBCGNS AND VISIT_BUILD_READER_CGNS)
  target_link_libraries(vtkIOVisItBridge LINK_PRIVATE ${CGNS_LIBRARIES})
endif(HAVE_LIBCGNS AND VISIT_BUILD_READER_CGNS)

if(HAVE_LIBMILI AND VISIT_BUILD_READER_Mili)
  target_link_libraries(vtkIOVisItBridge LINK_PRIVATE ${MILI_LIBRARIES})
endif(HAVE_LIBMILI AND VISIT_BUILD_READER_Mili)

if (VISIT_BUILD_READER_GMV)
  vtk_opengl_link(vtkIOVisItBridge)
endif()
