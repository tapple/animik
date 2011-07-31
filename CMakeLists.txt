PROJECT (animik C CXX)
CMAKE_MINIMUM_REQUIRED (VERSION 2.8.0)

# Configurable install variables, use CMAKE_INSTALL_PREFIX to install into target directory
SET (INSTALL_BIN "." CACHE PATH "Executable install path")
SET (INSTALL_DATA "." CACHE PATH "Data install path")
SET (INSTALL_DATA_PATH "." CACHE PATH "Where to search for data")

# Initialize and search for QT4 libraries
SET( QT_USE_QTOPENGL TRUE )
FIND_PACKAGE (Qt4 REQUIRED)
INCLUDE (${QT_USE_FILE})

# Search for OpenGL
FIND_PACKAGE (OpenGL REQUIRED)

INCLUDE_DIRECTORIES (libquat src ${OPENGL_INCLUDE_DIR})

SUBDIRS (libquat)
SUBDIRS (src)

# Install data
INSTALL (DIRECTORY resources/ DESTINATION ${INSTALL_DATA})
