# Sources
SET (QAVI_SRC AbstractDocumentTab.cpp
      AnimationList.cpp
      Announcer.cpp
      Avbl.cpp
      Blender.cpp
      BlenderTab.cpp
      BlenderTimeline.cpp
      KeyFramerTab.cpp
      LimbsWeightDialog.cpp
      LimbsWeightForm.cpp
      MixZonesDialog.cpp
      NewFileDialog.cpp
      NoArrowsScrollArea.cpp
      OptionalMessageBox.cpp
      Player.cpp
      PlayerOptionsDialog.cpp
      SaveChangesDialog.cpp
      TimelineTrail.cpp
      TrailItem.cpp
      WeightedAnimation.cpp	
      animation.cpp
			animationview.cpp
			bvh.cpp
			bvhnode.cpp
			camera.cpp
			iktree.cpp
			keyframelist.cpp
			main.cpp
			math3d.cpp
			prop.cpp
			qavimator.cpp
			rotation.cpp
			settings.cpp
			settingsdialog.cpp
			slpartsfemale.cpp
			slpartsmale.cpp
			timeline.cpp
			timelineview.cpp)

# Headers
SET (QAVI_MOC_HDR 	AbstractDocumentTab.h
      AnimationList.h
      Announcer.h
      Avbl.h
      Blender.h
      BlenderTab.h
      BlenderTimeline.h
      KeyFramerTab.h
      LimbsWeightDialog.h
      LimbsWeightForm.h
      MixZonesDialog.h
      NewFileDialog.h
      NoArrowsScrollArea.h
      OptionalMessageBox.h
      Player.h
      PlayerOptionsDialog.h
      SaveChangesDialog.h
      TimelineTrail.h
      WeightedAnimation.h
      animation.h
			animationview.h
			bvh.h
			bvhnode.h
			camera.h
			iktree.h
			keyframelist.h
			math3d.h
			playstate.h
			prop.h
			qavimator.h
			rotation.h
			settings.h
			settingsdialog.h
			slparts.h
			timeline.h
			timelineview.h)
			
# Interfaces
SET (QAVI_UI 	AnimationList.ui
      BlenderTab.ui
      KeyFramerTab.ui
      LimbsWeightDialog.ui
      LimbsWeightForm.ui
      MixZonesDialog.ui
      NewFileDialog.ui
      OptionalMessageBox.ui
      Player.ui
      PlayerOptionsDialog.ui
      SaveChangesDialog.ui
      mainapplicationform.ui
			settingsdialogform.ui)
			
# Resources
SET (QAVI_RSC	icons.qrc)

# Prepare and generate all needed sources and headers
QT4_ADD_RESOURCES (QAVI_RSC_SRC ${QAVI_RSC})
QT4_WRAP_UI (QAVI_UI_HDR ${QAVI_UI})
QT4_WRAP_CPP (QAVI_MOC_SRC ${QAVI_MOC_HDR})

# Include path to the generated header files
INCLUDE_DIRECTORIES (${CMAKE_BINARY_DIR}/src)

# Set defines
ADD_DEFINITIONS (-DQAVIMATOR_DATAPATH="${INSTALL_DATA_PATH}")

# Compile and link
ADD_EXECUTABLE (qavimator ${QAVI_SRC} ${QAVI_MOC_SRC} ${QAVI_RSC_SRC} ${QAVI_UI_HDR})
TARGET_LINK_LIBRARIES (qavimator ${QT_QTCORE_LIBRARY} ${QT_QTOPENGL_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} quat)

# Install qavimator
INSTALL (TARGETS qavimator DESTINATION ${INSTALL_BIN})
