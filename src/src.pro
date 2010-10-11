# DEFINES += QT_NO_DEBUG_OUTPUT
SOURCES += animation.cpp \
    animationview.cpp \
    bvh.cpp \
    bvhnode.cpp \
    camera.cpp \
    iktree.cpp \
    keyframelist.cpp \
    main.cpp \
    math3d.cpp \
    prop.cpp \
    qavimator.cpp \
    rotation.cpp \
    settings.cpp \
    settingsdialog.cpp \
    slpartsfemale.cpp \
    slpartsmale.cpp \
    timeline.cpp \
    timelineview.cpp \
    KeyFramerTab.cpp \
    AbstractDocumentTab.cpp \
    NewFileDialog.cpp \
    SaveChangesDialog.cpp \
    BlenderTab.cpp \
    Blender.cpp \
    BlenderTimeline.cpp \
    AnimationList.cpp
HEADERS += animation.h \
    animationview.h \
    bvh.h \
    bvhnode.h \
    camera.h \
    iktree.h \
    keyframelist.h \
    math3d.h \
    playstate.h \
    prop.h \
    qavimator.h \
    rotation.h \
    settings.h \
    settingsdialog.h \
    slparts.h \
    timeline.h \
    timelineview.h \
    KeyFramerTab.h \
    AbstractDocumentTab.h \
    NewFileDialog.h \
    SaveChangesDialog.h \
    BlenderTab.h \
    Blender.h \
    BlenderTimeline.h \
    AnimationList.h
FORMS += mainapplicationform.ui \
    settingsdialogform.ui \
    KeyFramerTab.ui \
    NewFileDialog.ui \
    SaveChangesDialog.ui \
    BlenderTab.ui \
    AnimationList.ui
RESOURCES += icons.qrc
TEMPLATE = app
TARGET = ../bin/qavimator
QT += opengl

# LIBS += -lglut \
# -lGLU
INCLUDEPATH += ../libquat \
    /usr/include
QMAKE_CXXFLAGS_DEBUG += -g3
CONFIG += release \
    warn_on \
    qt \
    thread
INSTALLS += target \
    datafiles \
    examples \
    documentation
target.path = /usr/bin
datafiles.path = /usr/share/qavimator/data
datafiles.files += ../bin/data/SLMale.bvh \
    ../bin/data/SLFemale.bvh \
    ../bin/data/SL.lim \
    ../bin/data/TPose.avm
examples.path = /usr/share/qavimator/examples
examples.files += ../examples/*
documentation.path = /usr/share/doc/packages/qavimator
documentation.files += ../documentation/*
win32 { 
    DEFINES += QAVIMATOR_DATAPATH=\\\".\\\"
    TARGETDEPS += ../libquat/release/libquat.a
    LIBS += ../libquat/release/libquat.a
    INCLUDEPATH += "C:/glut" # system specific, needs to be ajdusted
    LIBS += -L"C:/glut/GL/" \
        -lglut32
}
!win32 { 
    LIBS += -lglut \
        -lGLU
    DEFINES += QAVIMATOR_DATAPATH=\\\"/usr/share/qavimator\\\"
    TARGETDEPS += ../libquat/libquat.a
    LIBS += ../libquat/libquat.a
}
macx { 
    QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.3
    DEFINES -= QAVIMATOR_DATAPATH=\\\"/usr/share/qavimator\\\"
    DEFINES += QAVIMATOR_DATAPATH=\\\".\\\"
    LIBS -= -lGLU \
        -lglut
    LIBS += -framework \
        OpenGL \
        -framework \
        AGL \
        -framework \
        GLUT \
        -bind_at_load
    CONFIG += x86 \
        ppc
    QMAKE_POST_LINK += sh \
        fixbundle.sh
}
OTHER_FILES += ../TODO
