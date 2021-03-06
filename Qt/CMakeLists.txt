PROJECT (PQGUI)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

#########################################################################
# Setup Qt
SET(QT_REQUIRED TRUE)
SET(QT_MIN_VERSION "4.2.2")
FIND_PACKAGE(Qt4)

IF(NOT QT4_FOUND)
  MESSAGE(SEND_ERROR "Qt ${QT_MIN_VERSION} or greater not found.  Please check the QT_QMAKE_EXECUTABLE variable.")
ELSE(NOT QT4_FOUND)

# To make support of ParaView tractable, we only support a single version
# of Qt at one time.  If you really want to use a newer version of Qt, you
# can delete this check, but you are on your own.
STRING(REGEX MATCH "^4\\.2\\.[0-9]+" qt_version_tmp "${QTVERSION}")
IF (NOT qt_version_tmp)
  STRING(REGEX MATCH "^4\\.3\\.[0-9]+" qt_version_tmp2 "${QTVERSION}")
  IF (NOT qt_version_tmp2)
    MESSAGE(SEND_ERROR "Qt ${QTVERSION} not supported.  Please use Qt 4.2 or 4.3.")
  ENDIF (NOT qt_version_tmp2)
ENDIF (NOT qt_version_tmp)

# help CMake fine QtUiTools on Mac (binary install)
IF(Q_WS_MAC AND QT4_FOUND AND QT_QTCORE_FOUND AND NOT QT_QTUITOOLS_FOUND)
  FIND_LIBRARY(QT_QTUITOOLS_LIBRARY_RELEASE NAMES QtUiTools PATHS ${QT_LIBRARY_DIR})
  _QT4_ADJUST_LIB_VARS(QTUITOOLS)
ENDIF(Q_WS_MAC AND QT4_FOUND AND QT_QTCORE_FOUND AND NOT QT_QTUITOOLS_FOUND)

# help CMake fine QtAssistant on Mac (framework install)
IF(Q_WS_MAC AND QT4_FOUND AND NOT QT_QTASSISTANT_INCLUDE_DIR)
  FIND_PATH(QT_QTASSISTANT_INCLUDE_DIR QtAssistant 
            PATHS ${QT_LIBRARY_DIR}/QtAssistant.framework/Headers
            NO_DEFAULT_PATH)
ENDIF(Q_WS_MAC AND QT4_FOUND AND NOT QT_QTASSISTANT_INCLUDE_DIR)

# help CMake fine QtAssistant on Mac (binary install)
IF(Q_WS_MAC AND QT4_FOUND AND NOT QT_QTASSISTANT_INCLUDE_DIR)
  FIND_PATH(QT_QTASSISTANT_INCLUDE_DIR QtAssistant 
            PATHS ${QT_LIBRARY_DIR}/QtAssistant.framework/Headers
            NO_DEFAULT_PATH)
ENDIF(Q_WS_MAC AND QT4_FOUND AND NOT QT_QTASSISTANT_INCLUDE_DIR)

# help CMake fine QtDesigner on Mac (binary install)
IF(Q_WS_MAC AND QT4_FOUND AND NOT QT_QTDESIGNER_INCLUDE_DIR)
  FIND_PATH(QT_QTDESIGNER_INCLUDE_DIR QDesignerComponents
            PATHS ${QT_LIBRARY_DIR}/QtDesigner.framework/Headers
            NO_DEFAULT_PATH)
ENDIF(Q_WS_MAC AND QT4_FOUND AND NOT QT_QTDESIGNER_INCLUDE_DIR)

# help CMake find debug QtUtiTools on Windows
IF(WIN32 AND QT_QTUITOOLS_FOUND)
  IF(NOT QT_QTUITOOLS_LIBRARY_DEBUG MATCHES "QtUiToolsd")
    SET(MY_QT_QTUITOOLS_LIBRARY_DEBUG "${QT_LIBRARY_DIR}/QtUiToolsd.lib")
    IF(EXISTS "${MY_QT_QTUITOOLS_LIBRARY_DEBUG}")
      SET(QT_QTUITOOLS_LIBRARY_DEBUG "${MY_QT_QTUITOOLS_LIBRARY_DEBUG}" CACHE FILEPATH "QtUiTools debug library" FORCE) 
      SET(QT_QTUITOOLS_LIBRARY optimized ${QT_QTUITOOLS_LIBRARY_RELEASE} 
                               debug ${QT_QTUITOOLS_LIBRARY_DEBUG} CACHE FILEPATH "The Qt QtUiTools library" FORCE) 
    ENDIF(EXISTS "${MY_QT_QTUITOOLS_LIBRARY_DEBUG}")
  ENDIF(NOT QT_QTUITOOLS_LIBRARY_DEBUG MATCHES "QtUiToolsd")
ENDIF(WIN32 AND QT_QTUITOOLS_FOUND)

SET(QT_USE_QTUITOOLS TRUE)
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTXML TRUE)
# This ensures that the Qt includes dir etc are set properly.
# We don't need to find the Qt4 package or include the use file
# in any of the SUBDIRS in this directory.
INCLUDE(${QT_USE_FILE})


#########################################################################
# Include Qt dependent code.
ADD_SUBDIRECTORY(Testing)
ADD_SUBDIRECTORY(Widgets)
ADD_SUBDIRECTORY(Chart)

# Include Qt, VTK & Server Manager dependent code.
ADD_SUBDIRECTORY(Core)

IF(PARAVIEW_ENABLE_PYTHON)
  ADD_SUBDIRECTORY(Python)
  SET (MOC_INCLUDE_DIRS
    ${MOC_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/Python
    )
ENDIF(PARAVIEW_ENABLE_PYTHON)

ADD_SUBDIRECTORY(Components)

ENDIF(NOT QT4_FOUND)

