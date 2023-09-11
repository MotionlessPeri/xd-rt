cmake_minimum_required(VERSION 3.18)
project(xd_rt)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
include_directories(external/include)
set(INCLUDE_FILES
        include/Camera.h
        include/Ray.h
        include/Film.h
        include/Sampler.h include/Model.h include/HitRecord.h include/CoreTypes.h include/MathUtil.h include/Scene.h include/Primitive.h include/Light.h include/CameraFactory.h include/HitSolver.h include/Material.h include/BRDF.h include/Distribution.h include/Texture.h include/Triangle.h include/Plane.h
        include/MeshLoader.h
        include/Hitable.h
        include/AABB.h)
set(SRC_FILES
        src/Ray.cpp
        src/Film.cpp src/SimpleSampler.cpp src/Camera.cpp src/Sphere.cpp src/CameraFactory.cpp src/HitSolver.cpp src/Scene.cpp src/Primitive.cpp src/UniformHemisphere.cpp src/PointLight.cpp src/Lambertian.cpp src/MatteMaterial.cpp src/Triangle.cpp src/Plane.cpp
        src/ObjLoader.cpp
        src/AABB.cpp)
add_executable(xd_rt_playground main.cpp)
add_library(xd_rt STATIC ${INCLUDE_FILES} ${SRC_FILES})

find_package(TBB REQUIRED)

target_link_libraries(xd_rt_playground xd_rt)
target_link_libraries(xd_rt_playground TBB::tbb)

add_subdirectory(test)