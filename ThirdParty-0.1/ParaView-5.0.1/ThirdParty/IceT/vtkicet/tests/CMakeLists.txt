## Copyright 2003 Sandia Coporation
## Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
## the U.S. Government retains certain rights in this software.
##
## This source code is released under the New BSD License.
#

IF (ICET_USE_OPENGL AND NOT ICET_USE_OFFSCREEN_EGL)
  SET(ICET_OPENGL_WINDOW_LIBRARY GLUT
    CACHE STRING
    "The name of the library the IceT tests will use to create windows for
OpenGL contexts. Supported values are GLUT and GLFW."
    )
  SET_PROPERTY(CACHE ICET_OPENGL_WINDOW_LIBRARY
    PROPERTY STRINGS GLUT GLFW
    )
  MARK_AS_ADVANCED(ICET_OPENGL_WINDOW_LIBRARY)
  IF (ICET_OPENGL_WINDOW_LIBRARY STREQUAL GLUT)
    FIND_PACKAGE(GLUT)
    IF (GLUT_FOUND)
      INCLUDE_DIRECTORIES(${GLUT_INCLUDE_DIR})
      SET(ICET_TESTS_USE_OPENGL 1)
      SET(ICET_TESTS_USE_GLUT 1)
    ELSE (GLUT_FOUND)
      MESSAGE(STATUS "Cannot find GLUT library. OpenGL tests disabled.")
    ENDIF (GLUT_FOUND)
  ELSEIF (ICET_OPENGL_WINDOW_LIBRARY STREQUAL GLFW)
    FIND_PACKAGE(GLFW)
    IF (GLFW_FOUND)
      INCLUDE_DIRECTORIES(${GLFW_INCLUDE_DIR})
      SET(ICET_TESTS_USE_OPENGL 1)
      SET(ICET_TESTS_USE_GLFW 1)
    ELSE (GLFW_FOUND)
      MESSAGE(WARNING "Cannot find GLFW library. OpenGL tests disabled.")
    ENDIF (GLFW_FOUND)
  ELSE ()
    MESSAGE(SEND_ERROR "Invalid value for ICET_OPENGL_WINDOW_LIBRARY: ${ICET_OPENGL_WINDOW_LIBRARY}")
  ENDIF ()
ENDIF ()

IF (NOT ICET_TESTS_USE_OPENGL)
  MESSAGE(STATUS "Most tests require ICET_USE_OPENGL and GLUT or GLFW. Those tests are disabled.")
ENDIF (NOT ICET_TESTS_USE_OPENGL)

SET(IceTTestSrcs
  BackgroundCorrect.c
  CompressionSize.c
  FloatingViewport.c
  Interlace.c
  MaxImageSplit.c
  OddImageSizes.c
  OddProcessCounts.c
  PreRender.c
  RadixkrUnitTests.c
  RadixkUnitTests.c
  RenderEmpty.c
  SimpleTiming.c
  SparseImageCopy.c
  )

SET(IceTOpenGLTestSrcs
  BlankTiles.c
  BoundsBehindViewer.c
  DisplayNoDraw.c
  RandomTransform.c
  SimpleExample.c
  )

SET(UTIL_SRCS
  test_common.c
  ppm.c
  )

SET(TEST_HEADERS
  test_codes.h
  test_util.h
  test_mpi.h
  test_mpi_opengl.h
  )
SET_SOURCE_FILES_PROPERTIES(${TEST_HEADERS}
  PROPERTIES HEADER_ONLY TRUE
  )

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/test_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/test_config.h
  )

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

IF (ICET_MPIRUN_EXE)
  SET(PRE_TEST_FLAGS ${ICET_MPIRUN_EXE} ${ICET_MPI_NUMPROC_FLAG} ${ICET_MPI_MAX_NUMPROCS} ${ICET_MPI_PREFLAGS})
  SET(POST_TEST_FLAGS ${ICET_MPI_POSTFLAGS})
ENDIF (ICET_MPIRUN_EXE)

SET(ICET_TEST_FLAGS "" CACHE STRING "Arguments given to the test program.  These are given between the test executable and the name of the test.  Run the test executable with the -h flag to see what is available.")
MARK_AS_ADVANCED(ICET_TEST_FLAGS)
SEPARATE_ARGUMENTS(ICET_TEST_FLAGS)

CREATE_TEST_SOURCELIST(Tests icetTests_mpi.c ${IceTTestSrcs}
  EXTRA_INCLUDE test_mpi.h
  FUNCTION init_mpi)

ADD_EXECUTABLE(icetTests_mpi ${Tests} ${UTIL_SRCS} ${TEST_HEADERS})
TARGET_LINK_LIBRARIES(icetTests_mpi
  IceTCore
  IceTMPI
  )

FOREACH (test ${IceTTestSrcs})
  GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
  ADD_TEST(NAME IceT${TName}
    COMMAND
    ${PRE_TEST_FLAGS}
    $<TARGET_FILE:icetTests_mpi> ${ICET_TEST_FLAGS} ${TName}
    ${POST_TEST_FLAGS})
  IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.1)
    SET_TESTS_PROPERTIES(IceT${TName}
      PROPERTIES FAIL_REGULAR_EXPRESSION
      ":ERROR:;TEST NOT RUN;TEST NOT PASSED;TEST FAILED"
      )
    SET_TESTS_PROPERTIES(IceT${TName}
      PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed"
      )
  ENDIF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.1)
ENDFOREACH(test)

IF (ICET_TESTS_USE_OPENGL)
  CREATE_TEST_SOURCELIST(OpenGLTests icetTests_mpi_opengl.c ${IceTOpenGLTestSrcs}
    EXTRA_INCLUDE test_mpi_opengl.h
    FUNCTION init_mpi_opengl)

  ADD_EXECUTABLE(icetTests_mpi_opengl ${OpenGLTests} ${UTIL_SRCS} ${TEST_HEADERS})
  TARGET_LINK_LIBRARIES(icetTests_mpi_opengl
    IceTCore
    IceTMPI
    IceTGL
    )
  IF (ICET_TESTS_USE_GLUT)
    TARGET_LINK_LIBRARIES(icetTests_mpi_opengl ${GLUT_LIBRARIES})
  ENDIF (ICET_TESTS_USE_GLUT)
  IF (ICET_TESTS_USE_GLFW)
    TARGET_LINK_LIBRARIES(icetTests_mpi_opengl ${GLFW_LIBRARY})
  ENDIF (ICET_TESTS_USE_GLFW)

  FOREACH (test ${IceTOpenGLTestSrcs})
    GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
    ADD_TEST(NAME IceT${TName}
      COMMAND
      ${PRE_TEST_FLAGS}
      $<TARGET_FILE:icetTests_mpi_opengl> ${ICET_TEST_FLAGS} ${TName}
      ${POST_TEST_FLAGS})
    IF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.1)
      SET_TESTS_PROPERTIES(IceT${TName}
        PROPERTIES FAIL_REGULAR_EXPRESSION
        ":ERROR:;TEST NOT RUN;TEST NOT PASSED;TEST FAILED"
        )
      SET_TESTS_PROPERTIES(IceT${TName}
        PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed"
        )
    ENDIF (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.1)
  ENDFOREACH(test)
ENDIF (ICET_TESTS_USE_OPENGL)

ADD_CUSTOM_TARGET(logclean
  ${CMAKE_COMMAND} -E remove
    ${CMAKE_CURRENT_BINARY_DIR}/log.????
    ${CMAKE_CURRENT_BINARY_DIR}/*.ppm
    ${CMAKE_CURRENT_BINARY_DIR}/core.*
  )
