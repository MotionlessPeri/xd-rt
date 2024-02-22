cmake_minimum_required(VERSION 3.18)
project(xd_rt_tonemapping)
add_compile_definitions(PROJECT_ROOT=R\"\(${PROJECT_SOURCE_DIR}\)\")
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
set(GLFW_ROOT_DIR external/glfw3)
find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
set(APP_FILES
        TonemappingApp.cpp
        TonemappingApp.h
)
set(NON_BUILD_FILES
        ../../.clang-format
)

set(SRC_FILES
        ${APP_FILES}
        ${NON_BUILD_FILES})

include_directories(../../3rdparty/imgui/)
add_executable(xd_rt_tonemapping ${SRC_FILES})
target_link_libraries(xd_rt_tonemapping xd_rt_imguiAppBase)

add_tutorial_app(xd_rt_tonemapping "samples/tonemapping")
