## Copyright 2003 Sandia Coporation
## Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
## the U.S. Government retains certain rights in this software.
##
## This source code is released under the New BSD License.
#

SET(ICET_GL_SRCS
  gl_state.c
  gl_draw.c
  gl_image.c
  )

SET(ICET_GL_HEADERS
  ../include/IceTGL.h
  )

ICET_ADD_LIBRARY(IceTGL ${ICET_GL_SRCS} ${ICET_GL_HEADERS})

SET_SOURCE_FILES_PROPERTIES(${ICET_GL_HEADERS}
  PROPERTIES HEADER_FILE_ONLY TRUE
  )

TARGET_LINK_LIBRARIES(IceTGL
  IceTCore
  ${ICET_OPENGL_LIBRARIES}
  )

IF(NOT ICET_INSTALL_NO_DEVELOPMENT)
  INSTALL(FILES ${ICET_SOURCE_DIR}/src/include/IceTGL.h
    DESTINATION ${ICET_INSTALL_INCLUDE_DIR})
ENDIF(NOT ICET_INSTALL_NO_DEVELOPMENT)
