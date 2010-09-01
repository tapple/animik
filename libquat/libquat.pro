# Diese Datei wurde mit dem qmake-Manager von KDevelop erstellt. 
# ------------------------------------------- 
# Unterordner relativ zum Projektordner: ./libquat
# Das Target ist eine Bibliothek:  

HEADERS += GEN_List.h \
           GEN_Map.h \
           MEM_NonCopyable.h \
           MEM_RefCountedC-Api.h \
           MEM_RefCounted.h \
           MEM_RefCountPtr.h \
           MEM_SmartPtr.h \
           MT_assert.h \
           MT_CmMatrix4x4.h \
           MT_ExpMap.h \
           MT_Matrix3x3.h \
           MT_Matrix4x4.h \
           MT_MinMax.h \
           MT_Optimize.h \
           MT_Plane3.h \
           MT_Point2.h \
           MT_Point3.h \
           MT_Quaternion.h \
           MT_random.h \
           MT_Scalar.h \
           MT_Stream.h \
           MT_Transform.h \
           MT_Tuple2.h \
           MT_Tuple3.h \
           MT_Tuple4.h \
           MT_Vector2.h \
           MT_Vector3.h \
           MT_Vector4.h \
           NM_Scalar.h 
SOURCES += MEM_RefCountedC-Api.cpp \
           MT_Assert.cpp \
           MT_CmMatrix4x4.cpp \
           MT_ExpMap.cpp \
           MT_Matrix3x3.cpp \
           MT_Matrix4x4.cpp \
           MT_Plane3.cpp \
           MT_Point3.cpp \
           MT_Quaternion.cpp \
           MT_random.cpp \
           MT_Transform.cpp \
           MT_Vector2.cpp \
           MT_Vector3.cpp \
           MT_Vector4.cpp 

DEFINES += NDEBUG=1

macx {
  CONFIG += x86 ppc
}

CONFIG += release \
warn_on \
qt \
thread \
staticlib
TEMPLATE = lib

# make sure the generated name will be "libquat.a"
TARGET = quat
