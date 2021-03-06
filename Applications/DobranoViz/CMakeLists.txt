PROJECT(DobranoViz)

CMAKE_MINIMUM_REQUIRED(VERSION 2.2)

IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

INCLUDE_DIRECTORIES(
  ${DobranoViz_SOURCE_DIR}
  ${DobranoViz_BINARY_DIR}
)

IF(VTK_USE_MPI)
  INCLUDE_DIRECTORIES(${MPI_INCLUDE_PATH})
ENDIF(VTK_USE_MPI)

# Reduce the number of dirs that get included on moc command line
# since it causes issues on Windows 2000.
GET_DIRECTORY_PROPERTY(include_dirs_tmp INCLUDE_DIRECTORIES)
SET_DIRECTORY_PROPERTIES(PROPERTIES INCLUDE_DIRECTORIES "${MOC_INCLUDE_DIRS}")

QT4_WRAP_CPP(MOC_BUILT_SOURCES
  AboutDialog.h
  MainWindow.h
  LineChartAdapter.h
  ${PYTHON_MOC_SOURCES}
)

SET_DIRECTORY_PROPERTIES(PROPERTIES INCLUDE_DIRECTORIES "${include_dirs_tmp}")

QT4_WRAP_UI(UI_BUILT_SOURCES
  AboutDialog.ui
  MainWindow.ui
)

SOURCE_GROUP("Generated" FILES
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
)

IF(WIN32)
  LINK_DIRECTORIES(${QT_LIBRARY_DIR})
ENDIF(WIN32)

ADD_EXECUTABLE(DobranoViz WIN32 MACOSX_BUNDLE
  AboutDialog.cxx
  AboutDialog.h
  LineChartAdapter.cxx
  LineChartAdapter.h
  MainWindow.cxx
  MainWindow.h
  main.cxx
  ProcessModuleGUIHelper.cxx
  ProcessModuleGUIHelper.h
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
)

TARGET_LINK_LIBRARIES(DobranoViz
  pqComponents
  pqCore
  QtChart
  QtTesting
  pqWidgets
  ${QT_XML_LIBRARIES}
)

IF(WIN32)
  TARGET_LINK_LIBRARIES(DobranoViz ${QT_QTMAIN_LIBRARY})
ENDIF(WIN32)

