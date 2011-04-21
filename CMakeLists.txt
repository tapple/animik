PROJECT (qavimator)
CMAKE_MINIMUM_REQUIRED (VERSION 2.6.0)

# Configurable install variables, use CMAKE_INSTALL_PREFIX to install into target directory
SET (INSTALL_BIN "." CACHE PATH "Executable install path")
SET (INSTALL_DATA "." CACHE PATH "Data install path")
SET (INSTALL_DATA_PATH "." CACHE PATH "Where to search for data")

# Initialize and search for QT4 libraries
SET( QT_USE_QTOPENGL TRUE )
SET( QT_USE_QTXML TRUE )
FIND_PACKAGE (Qt4 REQUIRED)
INCLUDE (${QT_USE_FILE})

#Debug
MESSAGE(STATUS "QtXml was found in ${QT_QTXML_INCLUDE_DIR}")

# Search for OpenGL and GLUT
FIND_PACKAGE (OpenGL REQUIRED)
FIND_PACKAGE (GLUT REQUIRED)

#Debug
IF(GLUT_FOUND)
  MESSAGE(STATUS "Glut found. Detected path is : ${GLUT_INCLUDE_DIR}")
ELSE(GLUT_FOUND)
  MESSAGE(STATUS "Error: Glut NOT found. Using content of directory (${GLUT_INCLUDE_DIR})")
ENDIF(GLUT_FOUND)

INCLUDE_DIRECTORIES (libquat src ${OPENGL_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})

SUBDIRS (libquat)
SUBDIRS (src)

# Install data
INSTALL (DIRECTORY bin/data DESTINATION ${INSTALL_DATA})
