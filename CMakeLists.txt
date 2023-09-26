cmake_minimum_required(VERSION 3.18)
project(xd_rt)

set(CMAKE_CXX_STANDARD 17)
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/cmake")
# list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/external")
include_directories(include)
include_directories(external)
set(INCLUDE_FILES
        include/Camera.h
        include/Ray.h
        include/Film.h
        include/Sampler.h include/Model.h include/HitRecord.h include/CoreTypes.h include/MathUtil.h include/Scene.h include/Primitive.h include/Light.h include/CameraFactory.h include/HitSolver.h include/Material.h include/BRDF.h include/Distribution.h include/Texture.h include/Triangle.h
        include/MeshLoader.h
        include/Hitable.h
        include/AABB.h
        include/HitAccel.h
        include/Enums.h
        include/TextureFactory.h)
set(SRC_FILES
        src/Ray.cpp
        src/Film.cpp src/SimpleSampler.cpp src/Camera.cpp src/Sphere.cpp src/CameraFactory.cpp src/HitSolver.cpp src/Scene.cpp src/Primitive.cpp src/UniformHemisphere.cpp src/PointLight.cpp src/Lambertian.cpp src/MatteMaterial.cpp src/Triangle.cpp
        src/ObjLoader.cpp
        src/AABB.cpp
        src/BVHNode.cpp
        src/NoAccel.cpp
        src/SpecularReflectionMaterial.cpp
        src/Box.cpp
        src/TextureFactory.cpp
        src/DomeLight.cpp
        src/PieceWise1D.cpp
        src/PieceWise2D.cpp
        src/CosineHemisphere.cpp)
add_executable(xd_rt_playground main.cpp)
add_library(xd_rt STATIC ${INCLUDE_FILES} ${SRC_FILES})

set(TBB_ROOT "${CMAKE_SOURCE_DIR}/external/TBB")
set(EMBREE_ROOT_DIR "${CMAKE_SOURCE_DIR}/external/embree")

target_link_libraries(xd_rt_playground xd_rt)
target_link_libraries(xd_rt_playground TBB)

add_subdirectory(test)

add_custom_command(TARGET xd_rt
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${TBB_ROOT}/bin"
        $<TARGET_FILE_DIR:xd_rt>
)
add_custom_command(TARGET xd_rt
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${EMBREE_ROOT_DIR}/bin"
        $<TARGET_FILE_DIR:xd_rt>
)
message("${TBB_ROOT}")