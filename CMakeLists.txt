cmake_minimum_required(VERSION 3.18)
project(xd_rt)

set(CMAKE_CXX_STANDARD 20)
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/cmake")
# list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/external")
include_directories(src)
include_directories(src/core)
include_directories(external)

set(3RDPARTY_FILES
        src/3rdParty/embree/EmbreeGeomManager.h
        src/3rdParty/embree/EmbreeGeomManager.cpp
        src/3rdParty/embree/EmbreeUtil.h
        src/3rdParty/embree/EmbreeGlobal.h
        src/3rdParty/embree/EmbreeGlobal.cpp
        src/3rdParty/tinygltf/TinyGLTFUtil.h
        src/3rdParty/tinygltf/TinyGLTFWrapper.cpp
        src/3rdParty/stb/STBWrapper.cpp
)
set(BXDF_FILES
        src/bxdf/Lambertian.cpp
        src/bxdf/PerfectReflection.cpp
        src/bxdf/PerfectTransmission.cpp
)
set(CAMERA_FILES
        src/camera/Camera.cpp
        src/camera/CameraFactory.h
        src/camera/CameraFactory.cpp
)
set(CORE_FILES
        src/core/BxDF.h
        src/core/Camera.h
        src/core/CoreTypes.h
        src/core/Distribution.h
        src/core/Enums.h
        src/core/Film.cpp
        src/core/Film.h
        src/core/HitAccel.h
        src/core/HitRecord.h
        src/core/HitSolver.h
        src/core/Hitable.h
        src/core/Integrator.h
        src/core/Light.h
        src/core/Material.h
        src/core/Model.h
        src/core/Primitive.cpp
        src/core/Primitive.h
        src/core/Ray.cpp
        src/core/Ray.h
        src/core/Sampler.h
        src/core/Scene.cpp
        src/core/Scene.h
        src/core/Texture.h
        src/core/Triangle.h
        src/core/SceneBuilder.cpp
        src/core/SceneBuilder.h
        src/core/FloatWithError.h
)
set(DISTRIBUTION_FILES
        src/distribution/PieceWise1D.cpp
        src/distribution/PieceWise2D.cpp
        src/distribution/CosineHemisphere.cpp
        src/distribution/UniformHemisphere.cpp
)
set(HITACCEL_FILES
        src/core/AABB.h

        src/hitAccel/AABB.cpp
        src/hitAccel/BVHNode.cpp
        src/hitAccel/NoAccel.cpp
        src/hitAccel/EmbreeHitAccel.cpp
)
set(HITSOLVER_FILES
        src/hitSolver/HitSolver.cpp
)
set(INTEGRATOR_FILES
        src/integrator/DirectIntegrator.cpp
        src/integrator/DebugIntegrator.cpp
        src/integrator/PathIntegrator.cpp
        src/integrator/SamplerIntegrator.cpp
        src/integrator/Integrator.cpp
)
set(LIGHT_FILES
        src/light/PointLight.cpp
        src/light/DomeLight.cpp
)
set(LOADER_FILES
        src/loader/MeshLoader.h
        src/loader/ObjMeshLoader.cpp
        src/loader/SceneLoader.h
        src/loader/MaterialLoader.h
        src/loader/GLTFMaterialLoader.h
        src/loader/GLTFMaterialLoader.cpp
        src/loader/GLTFMeshLoader.h
        src/loader/GLTFMeshLoader.cpp
        src/loader/GLTFSceneLoader.h
        src/loader/GLTFSceneLoader.cpp
)
set(MATERIAL_FILES
        src/material/MatteMaterial.cpp
        src/material/PerfectReflectionMaterial.cpp
        src/material/PerfectTransmissionMaterial.cpp
        src/material/PerfectTransmissionMaterial.h
)
set(MODEL_FILES
        src/model/Sphere.cpp
        src/model/Triangle.cpp
        src/model/Box.cpp
        src/core/Model.cpp
)
set(SAMPLER_FILES
        src/core/Sampler.cpp
        src/sampler/SimpleSampler.h
        src/sampler/SimpleSampler.cpp
        src/sampler/DebugSampler.cpp
        src/sampler/DebugSampler.h
)
set(TEXTURE_FILES
        src/texture/TextureFactory.h
        src/texture/TextureFactory.cpp
)
set(MISC_FILES
        src/MathUtil.h
)
set(NON_BUILD_FILES
        ./.clang-format
        ./eigen.natvis
)
set(SRC_FILES
        ${3RDPARTY_FILES}
        ${BXDF_FILES}
        ${CAMERA_FILES}
        ${CORE_FILES}
        ${DISTRIBUTION_FILES}
        ${HITACCEL_FILES}
        ${HITSOLVER_FILES}
        ${INTEGRATOR_FILES}
        ${LIGHT_FILES}
        ${HITSOLVER_FILES}
        ${LOADER_FILES}
        ${MATERIAL_FILES}
        ${MODEL_FILES}
        ${SAMPLER_FILES}
        ${TEXTURE_FILES}
        ${MISC_FILES}
        ${NON_BUILD_FILES}
        src/integrator/SamplerIntegrator.cpp
)
add_executable(xd_rt_playground main.cpp)
add_library(xd_rt STATIC ${SRC_FILES})

target_link_libraries(xd_rt TBB)
set(TBB_ROOT "${CMAKE_SOURCE_DIR}/external/TBB")
set(EMBREE_ROOT_DIR "${CMAKE_SOURCE_DIR}/external/embree")

find_package(TBB REQUIRED)
include_directories(${TBB_INCLUDE_DIRS})
find_package(embree 4 REQUIRED)
include_directories(${EMBREE_INCLUDE_DIRS})

target_link_libraries(xd_rt ${EMBREE_LIBRARIES})

#target_link_directories("${TBB_ROOT}/lib")
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