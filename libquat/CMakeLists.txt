# Sources
SET (QUAT_SRC MEM_RefCountedC-Api.cpp
           MT_Assert.cpp
           MT_CmMatrix4x4.cpp
           MT_ExpMap.cpp
           MT_Matrix3x3.cpp
           MT_Matrix4x4.cpp
           MT_Plane3.cpp
           MT_Point3.cpp
           MT_Quaternion.cpp
           MT_random.cpp
           MT_Transform.cpp
           MT_Vector2.cpp
           MT_Vector3.cpp
           MT_Vector4.cpp)

# Build a static lib
ADD_LIBRARY (quat ${QUAT_SRC})
