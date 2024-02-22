cmake_minimum_required(VERSION 3.18)
project(xd_rt_imguiAppBase)
add_compile_definitions(PROJECT_ROOT=R\"\(${PROJECT_SOURCE_DIR}\)\")
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
set(GLFW_ROOT_DIR external/glfw3)
find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
set(APP_FILES
        ImguiAppBase.cpp
        ImguiAppBase.h
)
set(NON_BUILD_FILES
        ../../.clang-format
)

file(GLOB IMGUI_HEADERS ../../3rdparty/imgui/*.h)
file(GLOB IMGUI_SOURCES ../../3rdparty/imgui/*.cpp)
include_directories(../../3rdparty/imgui/)
include_directories(../../3rdparty/imgui/backends/)
set(IMGUI_FILES ${IMGUI_HEADERS} ${IMGUI_SOURCES})

set(IMGUI_FILES
        ${IMGUI_FILES}
        ../../3rdparty/imgui/backends/imgui_impl_vulkan.h
        ../../3rdparty/imgui/backends/imgui_impl_vulkan.cpp
        ../../3rdparty/imgui/backends/imgui_impl_glfw.h
        ../../3rdparty/imgui/backends/imgui_impl_glfw.cpp
)

set(SRC_FILES
        ${APP_FILES}
        ${NON_BUILD_FILES}
        ${IMGUI_FILES})
source_group(imgui FILES ${IMGUI_FILES})

add_library(xd_rt_imguiAppBase ${SRC_FILES})
target_link_libraries(xd_rt_imguiAppBase xd_rt_appBase)

add_tutorial_app(xd_rt_imguiAppBase "samples/imguiAppBase")