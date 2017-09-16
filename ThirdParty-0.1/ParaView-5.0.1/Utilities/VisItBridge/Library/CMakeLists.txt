vtk_module_third_party(VisItLib
  LIBRARIES VisItLib
  NO_ADD_SUBDIRECTORY
)

if(VTK_USE_SYSTEM_VISITLIB)
  return()
endif()

add_subdirectory(VisItLib)

#any module that uses visitlib needs to know where its internal
#include directories are
list(APPEND VisItLib_INCLUDE_DIRS ${VISITBRIDGE_INCLUDE_DIRS})

vtk_module_export_info()
