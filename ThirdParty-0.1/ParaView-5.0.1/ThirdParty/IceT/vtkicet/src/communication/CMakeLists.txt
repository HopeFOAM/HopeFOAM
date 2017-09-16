## Copyright 2003 Sandia Coporation
## Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
## the U.S. Government retains certain rights in this software.
##
## This source code is released under the New BSD License.
#

SET(ICET_MPI_SRCS
  mpi.c
  )

SET(ICET_MPI_HEADERS
  ../include/IceTMPI.h
  )

IF (ICET_USE_MPI)
  ICET_ADD_LIBRARY(IceTMPI ${ICET_MPI_SRCS} ${ICET_MPI_HEADERS})

  SET_SOURCE_FILES_PROPERTIES(${ICET_MPI_HEADERS}
    PROPERTIES HEADER_FILE_ONLY TRUE
    )

  TARGET_LINK_LIBRARIES(IceTMPI
    IceTCore
    ${ICET_MPI_LIBRARIES}
    )

  IF(NOT ICET_INSTALL_NO_DEVELOPMENT)
    INSTALL(FILES ${ICET_SOURCE_DIR}/src/include/IceTMPI.h
      DESTINATION ${ICET_INSTALL_INCLUDE_DIR})
    INSTALL(TARGETS IceTMPI
      DESTINATION ${ICET_INSTALL_LIB_DIR} COMPONENT Development)
  ENDIF(NOT ICET_INSTALL_NO_DEVELOPMENT)

ENDIF (ICET_USE_MPI)
