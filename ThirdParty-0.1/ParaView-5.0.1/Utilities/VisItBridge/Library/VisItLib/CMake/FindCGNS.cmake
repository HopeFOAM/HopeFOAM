#
# Find the native CGNS includes and library
#
# CGNS_INCLUDE_DIR - where to find cgns.h, etc.
# CGNS_LIBRARIES   - List of fully qualified libraries to link against when using CGNS.
# CGNS_FOUND       - Do not attempt to use CGNS if "no" or undefined.

FIND_PATH(CGNS_INCLUDE_DIR cgnslib.h
  /usr/local/include
  /usr/include
)

FIND_LIBRARY(CGNS_LIBRARY cgns
  /usr/local/lib
  /usr/lib
)

SET( CGNS_FOUND "NO" )
IF(CGNS_INCLUDE_DIR)
  IF(CGNS_LIBRARY)
    SET( CGNS_LIBRARIES ${CGNS_LIBRARY} )
    SET( CGNS_FOUND "YES" )
  ENDIF(CGNS_LIBRARY)
ENDIF(CGNS_INCLUDE_DIR)

IF(CGNS_FIND_REQUIRED AND NOT CGNS_FOUND)
  message(SEND_ERROR "Unable to find the requested CGNS libraries.")
ENDIF(CGNS_FIND_REQUIRED AND NOT CGNS_FOUND)

# handle the QUIETLY and REQUIRED arguments and set CGNS_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CGNS DEFAULT_MSG CGNS_LIBRARY CGNS_INCLUDE_DIR)


MARK_AS_ADVANCED(
  CGNS_INCLUDE_DIR
  CGNS_LIBRARY
)