cmake_minimum_required(VERSION 3.18)
project(xd_rt_test)

add_subdirectory(../3rdparty/googletest test.out)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

find_package(TBB REQUIRED)
include_directories(${TBB_INCLUDE_DIRS})
find_package(embree 4 REQUIRED)
include_directories(${EMBREE_INCLUDE_DIRS})
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
set(GLFW_ROOT_DIR external/glfw3)
find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})

set(NON_BUILD_FILES
        ../.clang-format
        ../eigen.natvis
)

file(GLOB IMGUI_HEADERS ../3rdparty/imgui/*.h)
file(GLOB IMGUI_SOURCES ../3rdparty/imgui/*.cpp)

set(IMGUI_FILES ${IMGUI_HEADERS} ${IMGUI_SOURCES})

include_directories(../3rdparty/imgui/)
include_directories(../3rdparty/imgui/backends/)
if (USE_VULKAN)
    set(IMGUI_FILES
            ${IMGUI_FILES}
            ../3rdparty/imgui/backends/imgui_impl_vulkan.h
            ../3rdparty/imgui/backends/imgui_impl_vulkan.cpp
            ../3rdparty/imgui/backends/imgui_impl_glfw.h
            ../3rdparty/imgui/backends/imgui_impl_glfw.cpp
    )
endif ()

source_group(imgui FILES ${IMGUI_FILES})

add_executable(xd_rt_test
        ${NON_BUILD_FILES}
        ${IMGUI_FILES}
        AABBTest.cpp
        BxDFTest.cpp
        CameraTest.cpp
        DistributionTest.cpp
        FilmTest.cpp
        FloatWithErrorTest.cpp
        GLTFLoaderTest.cpp
        HitAccelTest.cpp
        HitRecordTest.cpp
        HitSolverTest.cpp
        IntegratorTest.cpp
        LightTest.cpp
        Macros.h
        MaterialTest.cpp
        MathUtilTest.cpp
        ModelHitTest.cpp
        ObjLoaderTest.cpp
        PrimitiveTest.cpp
        SamplerTest.cpp
        TextureTest.cpp
        TriangleTest.cpp
        VisibilityTest.cpp
        TestScenes.h
        ImageTest.cpp
        FilterTest.cpp
        MappingTest.cpp
        ImguiIntegrationTest.cpp
        VulkanTest.cpp
)

target_link_libraries(xd_rt_test xd_rt)
target_link_libraries(xd_rt_test xd_rt_app)
target_link_libraries(xd_rt_test gtest gtest_main)
target_link_libraries(xd_rt_test TBB)
target_link_libraries(xd_rt_test ${EMBREE_LIBRARIES})
target_link_libraries(xd_rt_test ${GLFW_LIBRARIES})

add_custom_command(TARGET xd_rt_test
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${TBB_ROOT}/bin"
        $<TARGET_FILE_DIR:xd_rt_test>
)
add_custom_command(TARGET xd_rt_test
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${EMBREE_ROOT_DIR}/bin"
        $<TARGET_FILE_DIR:xd_rt_test>
)
add_custom_command(TARGET xd_rt_test
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${GLFW_LIB_DIR}/glfw3.dll"
        $<TARGET_FILE_DIR:xd_rt_test>
)