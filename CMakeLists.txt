cmake_minimum_required ( VERSION 2.8.7 )
project ( animik C CXX )

# Ogre setup
set ( CMAKE_MODULE_PATH "$ENV{OGRE_HOME}/CMake/;${CMAKE_MODULE_PATH}" )

find_package ( OGRE REQUIRED )
find_package ( OIS REQUIRED )

include_directories ( ${OIS_INCLUDE_DIRS} ${OGRE_INCLUDE_DIRS} )

# Find Boost
set ( BOOST_ROOT "$ENV{OGRE_HOME}/boost/")
set ( Boost_USE_STATIC_LIBS TRUE )
set ( CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_FIND_LIBRARY_PREFIXES} "" )
set (OGRE_BOOST_COMPONENTS system thread date_time )

find_package ( Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} REQUIRED )
 
include_directories ( ${Boost_INCLUDE_DIR} )
add_definitions ( -DBOOST_ALL_NO_LIB )

# configurable install variables, use CMAKE_INSTALL_PREFIX to install into target directory
set ( INSTALL_BIN "." CACHE PATH "Executable install path" )
set ( INSTALL_DATA "." CACHE PATH "Data install path" )
set ( INSTALL_DATA_PATH "." CACHE PATH "Where to search for data" )

# Initialize and search for Qt libraries
set ( CMAKE_AUTOMOC ON )
set ( CMAKE_INCLUDE_CURRENT_DIR ON )
find_package ( Qt5Widgets REQUIRED )
find_package ( Qt5OpenGL REQUIRED )
find_package ( Qt5Xml REQUIRED )

# Search for OpenGL
find_package ( OpenGL REQUIRED )

include_directories ( libquat src ${OPENGL_INCLUDE_DIR} )

# libquat sources
file ( GLOB QUAT_SRC libquat/*.cpp )

# build a static lib
add_library ( quat ${QUAT_SRC} )

# Animik sources
file ( GLOB ANIMIK_SRC src/*.cpp )
#file ( GLOB ANIMIK_MOC_HDR src/*.h )
file ( GLOB ANIMIK_UI src/*.ui )
file ( GLOB ANIMIK_RSC src/*.qrc )

# Prepare and generate all needed sources and headers
qt5_add_resources ( ANIMIK_RSC_SRC ${ANIMIK_RSC} )
qt5_wrap_ui ( ANIMIK_UI_HDR ${ANIMIK_UI} )
#QT5_WRAP_CPP ( ANIMIK_MOC_SRC ${ANIMIK_MOC_HDR} )

# Include path to the generated header files
include_directories ( ${CMAKE_BINARY_DIR}/src )

# Set defines
add_definitions ( -DQAVIMATOR_DATAPATH="${INSTALL_DATA_PATH}" )

# No console on Windows
if ( CYGWIN OR MINGW OR WIN32 )
  set ( EXE_TYPE WIN32 )
endif ()

# Compile and link
add_executable ( animik ${EXE_TYPE} ${ANIMIK_SRC} ${ANIMIK_MOC_SRC} ${ANIMIK_RSC_SRC} ${ANIMIK_UI_HDR} )
target_link_libraries ( animik ${OPENGL_LIBRARIES} ${OGRE_LIBRARIES} ${OIS_LIBRARIES} ${Boost_LIBRARIES} quat )
qt5_use_modules ( animik Widgets OpenGL Xml )

# Use static gcc lib
if ( CYGWIN OR MINGW )
  set_target_properties ( animik PROPERTIES LINK_FLAGS "-static-libgcc" )
endif ()

# Install animik
install ( TARGETS animik DESTINATION ${INSTALL_BIN} )

# Install data
install ( DIRECTORY resources/ DESTINATION ${INSTALL_DATA} )
