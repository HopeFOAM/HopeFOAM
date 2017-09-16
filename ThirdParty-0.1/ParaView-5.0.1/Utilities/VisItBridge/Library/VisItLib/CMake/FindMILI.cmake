#
# Find the native MILI includes and library
#
# MILI_INCLUDE_DIR - where to find MILI.h, etc.
# MILI_LIBRARIES   - List of fully qualified libraries to link against when using MILI.
# MILI_FOUND       - Do not attempt to use MILI if "no" or undefined.

FIND_PATH(MILI_INCLUDE_DIR MILIlib.h
  /usr/local/include
  /usr/include
)

FIND_LIBRARY(MILI_LIBRARY MILI
  /usr/local/lib
  /usr/lib
)

SET( MILI_FOUND "NO" )
IF(MILI_INCLUDE_DIR)
  IF(MILI_LIBRARY)
    SET( MILI_LIBRARIES ${MILI_LIBRARY} )
    SET( MILI_FOUND "YES" )
  ENDIF(MILI_LIBRARY)
ENDIF(MILI_INCLUDE_DIR)

IF(MILI_FIND_REQUIRED AND NOT MILI_FOUND)
  message(SEND_ERROR "Unable to find the requested MILI libraries.")
ENDIF(MILI_FIND_REQUIRED AND NOT MILI_FOUND)

# handle the QUIETLY and REQUIRED arguments and set MILI_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MILI DEFAULT_MSG MILI_LIBRARY MILI_INCLUDE_DIR)

MARK_AS_ADVANCED(
  MILI_INCLUDE_DIR
  MILI_LIBRARY
)
