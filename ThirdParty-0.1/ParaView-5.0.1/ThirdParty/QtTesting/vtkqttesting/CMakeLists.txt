CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(QtTesting)

IF(NOT DEFINED QtTesting_QT_VERSION)
  SET(QtTesting_QT_VERSION "4" CACHE STRING "Expected Qt version")
  MARK_AS_ADVANCED(QtTesting_QT_VERSION)
  SET_PROPERTY(CACHE QtTesting_QT_VERSION PROPERTY STRINGS 4 5)
ENDIF()
IF(NOT (QtTesting_QT_VERSION VERSION_EQUAL "4" OR
  QtTesting_QT_VERSION VERSION_EQUAL "5"))
  message(FATAL_ERROR "Expected value for QtTesting_QT_VERSION is either '4' or '5'")
ENDIF()

IF(QtTesting_QT_VERSION VERSION_GREATER "4")
  IF(NOT Qt5Test_FOUND)
    FIND_PACKAGE(Qt5Widgets REQUIRED)
    FIND_PACKAGE(Qt5Test REQUIRED)
    ADD_DEFINITIONS(
      ${Qt5Widgets_DEFINITIONS}
      ${Qt5Test_DEFINITIONS}
      )
    INCLUDE_DIRECTORIES(
      ${Qt5Widgets_INCLUDE_DIRS}
      ${Qt5Test_INCLUDE_DIRS}
      )
    SET(QT_LIBRARIES
      ${Qt5Widgets_LIBRARIES}
      ${Qt5Test_LIBRARIES}
      )
  ENDIF()
ELSE()
  IF(NOT QT4_FOUND)
    FIND_PACKAGE(Qt4 REQUIRED)
    INCLUDE( ${QT_USE_FILE} )
  ENDIF()
ENDIF()

IF(NOT DEFINED QT_TESTING_WITH_PYTHON)
  OPTION(QT_TESTING_WITH_PYTHON "Enable Qt Testing with Python" OFF)
ENDIF()

IF(NOT DEFINED QtTesting_INSTALL_BIN_DIR)
  SET(QtTesting_INSTALL_BIN_DIR bin)
ENDIF()

IF(NOT DEFINED QtTesting_INSTALL_INCLUDE_DIR)
  SET(QtTesting_INSTALL_INCLUDE_DIR include/QtTesting)
ENDIF()

IF(NOT DEFINED QtTesting_INSTALL_LIB_DIR)
  SET(QtTesting_INSTALL_LIB_DIR lib)
ENDIF()

IF(NOT DEFINED QtTesting_INSTALL_CMAKE_DIR)
  SET(QtTesting_INSTALL_CMAKE_DIR lib/cmake/qttesting)
ENDIF()

IF(NOT DEFINED QT_TESTING_EVENT_PLAYBACK_DELAY)
  SET(QT_TESTING_EVENT_PLAYBACK_DELAY "100" CACHE STRING "Delay between invocation of each testing event." FORCE)
  MARK_AS_ADVANCED(QT_TESTING_EVENT_PLAYBACK_DELAY)
ENDIF()

IF(NOT QT_TESTING_INSTALL_EXPORT_NAME)
  SET(QT_TESTING_INSTALL_EXPORT_NAME QtTestingTargets)
ENDIF()

IF(QT_TESTING_WITH_PYTHON)

  IF(NOT PythonLibs_FOUND)
    FIND_PACKAGE(PythonLibs REQUIRED)
  ENDIF()

  IF(UNIX)
    FIND_LIBRARY(PYTHON_UTIL_LIBRARY
      NAMES util
      PATHS /usr/lib
      DOC "Utility library needed for vtkpython"
      )
    MARK_AS_ADVANCED(PYTHON_UTIL_LIBRARY)
    IF(PYTHON_UTIL_LIBRARY)
      SET(PYTHON_UTIL_LIBRARY_LIB ${PYTHON_UTIL_LIBRARY})
    ENDIF()
  ENDIF()

  INCLUDE_DIRECTORIES(
    ${PYTHON_INCLUDE_PATH}
  )
  SET(PYTHON_MOCS
    pqPythonEventObserver.h
    pqPythonEventSource.h
  )
  SET(PYTHON_SRCS
    pqPythonEventObserver.cxx
    pqPythonEventObserver.h
    pqPythonEventSource.cxx
    pqPythonEventSource.h
  )
ENDIF(QT_TESTING_WITH_PYTHON)

INCLUDE_DIRECTORIES(
  ${QtTesting_BINARY_DIR}
  ${QtTesting_SOURCE_DIR}
)

# Reduce the number of dirs that get included on moc command line
# since it causes issues on Windows 2000.
GET_DIRECTORY_PROPERTY(include_dirs_tmp INCLUDE_DIRECTORIES)
SET_DIRECTORY_PROPERTIES(PROPERTIES INCLUDE_DIRECTORIES "${MOC_INCLUDE_DIRS}")

SET(MOC_SRCS
  pq3DViewEventTranslator.h
  pq3DViewEventPlayer.h
  pqAbstractActivateEventPlayer.h
  pqAbstractButtonEventTranslator.h
  pqAbstractBooleanEventPlayer.h
  pqAbstractDoubleEventPlayer.h
  pqAbstractIntEventPlayer.h
  pqAbstractItemViewEventPlayer.h
  pqAbstractItemViewEventTranslator.h
  pqAbstractMiscellaneousEventPlayer.h
  pqAbstractSliderEventTranslator.h
  pqAbstractStringEventPlayer.h
  pqBasicWidgetEventTranslator.h
  pqBasicWidgetEventPlayer.h
  pqComboBoxEventTranslator.h
  pqCommentEventPlayer.h
  pqDoubleSpinBoxEventTranslator.h
  pqEventComment.h
  pqEventDispatcher.h
  pqEventObserver.h
  pqEventPlayer.h
  pqEventSource.h
  pqEventTranslator.h
  pqLineEditEventTranslator.h
  pqMenuEventTranslator.h
  pqNativeFileDialogEventPlayer.h
  pqNativeFileDialogEventTranslator.h
  pqPlayBackEventsDialog.h
  pqEventRecorder.h
  pqRecordEventsDialog.h
  pqSpinBoxEventTranslator.h
  pqStdoutEventObserver.h
  pqTabBarEventPlayer.h
  pqTabBarEventTranslator.h
  pqTestUtility.h
  pqThreadedEventSource.h
  pqTimer.h
  pqTreeViewEventPlayer.h
  pqTreeViewEventTranslator.h
  pqWidgetEventTranslator.h
  pqWidgetEventPlayer.h
)

IF(QtTesting_QT_VERSION VERSION_GREATER "4")
  QT5_WRAP_CPP(MOC_BUILT_SOURCES
    ${MOC_SRCS}
    ${PYTHON_MOCS}
  )
ELSE()
  QT4_WRAP_CPP(MOC_BUILT_SOURCES
    ${MOC_SRCS}
    ${PYTHON_MOCS}
  )
ENDIF()

SET_DIRECTORY_PROPERTIES(PROPERTIES INCLUDE_DIRECTORIES "${include_dirs_tmp}")

IF(QtTesting_QT_VERSION VERSION_GREATER "4")
  QT5_WRAP_UI(UI_BUILT_SOURCES
    pqPlayBackEventsDialog.ui
    pqRecordEventsDialog.ui
  )
ELSE()
  QT4_WRAP_UI(UI_BUILT_SOURCES
    pqPlayBackEventsDialog.ui
    pqRecordEventsDialog.ui
  )
ENDIF()

IF(QtTesting_QT_VERSION VERSION_GREATER "4")
  QT5_ADD_RESOURCES(QRC_BUILT_SOURCES
    Resources/QtTesting.qrc
  )
ELSE()
  QT4_ADD_RESOURCES(QRC_BUILT_SOURCES
    Resources/QtTesting.qrc
  )
ENDIF()

SET(QtTesting_SOURCES
  pq3DViewEventPlayer.cxx
  pq3DViewEventTranslator.cxx
  pqAbstractActivateEventPlayer.cxx
  pqAbstractBooleanEventPlayer.cxx
  pqAbstractButtonEventTranslator.cxx
  pqAbstractDoubleEventPlayer.cxx
  pqAbstractIntEventPlayer.cxx
  pqAbstractItemViewEventPlayer.cxx
  pqAbstractItemViewEventTranslator.cxx
  pqAbstractMiscellaneousEventPlayer.cxx
  pqAbstractSliderEventTranslator.cxx
  pqAbstractStringEventPlayer.cxx
  pqBasicWidgetEventPlayer.cxx
  pqBasicWidgetEventTranslator.cxx
  pqComboBoxEventTranslator.cxx
  pqCommentEventPlayer.cxx
  pqDoubleSpinBoxEventTranslator.cxx
  pqEventComment.cxx
  pqEventDispatcher.cxx
  pqEventObserver.cxx
  pqEventPlayer.cxx
  pqEventTranslator.cxx
  pqLineEditEventTranslator.cxx
  pqMenuEventTranslator.cxx
  pqNativeFileDialogEventPlayer.cxx
  pqNativeFileDialogEventTranslator.cxx
  pqObjectNaming.cxx
  pqPlayBackEventsDialog.cxx
  pqEventRecorder.cxx
  pqRecordEventsDialog.cxx
  pqSpinBoxEventTranslator.cxx
  pqStdoutEventObserver.cxx
  pqTabBarEventPlayer.cxx
  pqTabBarEventTranslator.cxx
  pqTestUtility.cxx
  pqThreadedEventSource.cxx
  pqTimer.cxx
  pqTreeViewEventPlayer.cxx
  pqTreeViewEventTranslator.cxx
  pqWidgetEventPlayer.cxx
)

SET(QtTesting_DEVEL_HEADERS
  pq3DViewEventPlayer.h
  pq3DViewEventTranslator.h
  pqAbstractActivateEventPlayer.h
  pqAbstractMiscellaneousEventPlayer.h
  pqAbstractDoubleEventPlayer.h
  pqAbstractBooleanEventPlayer.h
  pqAbstractButtonEventTranslator.h
  pqAbstractIntEventPlayer.h
  pqAbstractItemViewEventPlayer.h
  pqAbstractItemViewEventTranslator.h
  pqAbstractSliderEventTranslator.h
  pqAbstractStringEventPlayer.h
  pqBasicWidgetEventPlayer.h
  pqBasicWidgetEventTranslator.h
  pqComboBoxEventTranslator.h
  pqCommentEventPlayer.h
  pqDoubleSpinBoxEventTranslator.h
  pqEventComment.h
  pqEventDispatcher.h
  pqEventObserver.h
  pqEventPlayer.h
  pqEventSource.h
  pqEventTranslator.h
  pqLineEditEventTranslator.h
  pqMenuEventTranslator.h
  pqNativeFileDialogEventPlayer.h
  pqNativeFileDialogEventTranslator.h
  pqObjectNaming.h
  pqPlayBackEventsDialog.h
  pqEventRecorder.h
  pqRecordEventsDialog.h
  pqSpinBoxEventTranslator.h
  pqStdoutEventObserver.h
  pqTabBarEventPlayer.h
  pqTabBarEventTranslator.h
  pqTestUtility.h
  pqThreadedEventSource.h
  pqTimer.h
  pqTreeViewEventPlayer.h
  pqTreeViewEventTranslator.h
  pqWidgetEventPlayer.h
  pqWidgetEventTranslator.h
  QtTestingExport.h
  ${QtTesting_BINARY_DIR}/QtTestingConfigure.h
)

ADD_LIBRARY(QtTesting
  ${QtTesting_SOURCES}
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
  ${QRC_BUILT_SOURCES}
  ${PYTHON_SRCS}
)

SOURCE_GROUP("Generated" FILES
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
)

TARGET_LINK_LIBRARIES(QtTesting
  ${QT_LIBRARIES}
)

IF(QT_TESTING_WITH_PYTHON)
  TARGET_LINK_LIBRARIES(QtTesting
    ${PYTHON_LIBRARIES}
    ${PYTHON_UTIL_LIBRARY_LIB}
  )
ENDIF()

SET(QTTESTING_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
CONFIGURE_FILE(${QtTesting_SOURCE_DIR}/QtTestingConfigure.h.in
               ${QtTesting_BINARY_DIR}/QtTestingConfigure.h)

INSTALL(TARGETS QtTesting
  EXPORT ${QT_TESTING_INSTALL_EXPORT_NAME}
  RUNTIME DESTINATION ${QtTesting_INSTALL_BIN_DIR} COMPONENT Runtime
  LIBRARY DESTINATION ${QtTesting_INSTALL_LIB_DIR} COMPONENT Runtime
  ARCHIVE DESTINATION ${QtTesting_INSTALL_LIB_DIR} COMPONENT Development)


add_subdirectory(Examples)

include(CTest)
IF(BUILD_TESTING)
  add_subdirectory(Testing)
ENDIF()

export(TARGETS QtTesting FILE ${QtTesting_BINARY_DIR}/QtTestingExports.cmake)

# Set up the build export configuration
set(QtTesting_EXPORT_FILE "${QtTesting_BINARY_DIR}/QtTestingConfig.cmake")
configure_file(
  "${QtTesting_SOURCE_DIR}/QtTestingConfig.cmake.in"
  "${QtTesting_EXPORT_FILE}"
  @ONLY
)

# Set up the install export
IF(IS_ABSOLUTE QtTesting_INSTALL_INCLUDE_DIR)
  set(QtTesting_INSTALL_INCLUDE_FULL_DIR "${QtTesting_INSTALL_INCLUDE_DIR}")
ELSE()
  set(QtTesting_INSTALL_INCLUDE_FULL_DIR "${CMAKE_INSTALL_PREFIX}/${QtTesting_INSTALL_INCLUDE_DIR}")
  get_filename_component(QtTesting_INSTALL_INCLUDE_FULL_DIR "${QtTesting_INSTALL_INCLUDE_FULL_DIR}" ABSOLUTE)
ENDIF()

IF(IS_ABSOLUTE QtTesting_INSTALL_LIB_DIR)
  set(QtTesting_INSTALL_LIB_FULL_DIR "${QtTesting_INSTALL_LIB_DIR}")
ELSE()
  set(QtTesting_INSTALL_LIB_FULL_DIR "${CMAKE_INSTALL_PREFIX}/${QtTesting_INSTALL_LIB_DIR}")
  get_filename_component(QtTesting_INSTALL_LIB_FULL_DIR "${QtTesting_INSTALL_LIB_FULL_DIR}" ABSOLUTE)
ENDIF()

set(QtTesting_EXPORT_INSTALL_FILE "${QtTesting_BINARY_DIR}/CMakeFiles/QtTestingConfig.cmake")
configure_file(
  "${QtTesting_SOURCE_DIR}/QtTestingConfig-install.cmake.in"
  "${QtTesting_EXPORT_INSTALL_FILE}"
  @ONLY
)

IF(NOT QtTesting_INSTALL_NO_DEVELOPMENT)
  install( FILES ${QtTesting_DEVEL_HEADERS}
    DESTINATION ${QtTesting_INSTALL_INCLUDE_DIR}
  )

  # Configure the CMake EXPORT file during installation
  install( FILES ${QtTesting_BINARY_DIR}/CMakeFiles/QtTestingConfig.cmake
    DESTINATION ${QtTesting_INSTALL_CMAKE_DIR}
  )
  install( EXPORT ${QT_TESTING_INSTALL_EXPORT_NAME}
    DESTINATION ${QtTesting_INSTALL_CMAKE_DIR}
  )
ENDIF()
