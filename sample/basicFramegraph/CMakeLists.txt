cmake_minimum_required(VERSION 3.18)
project(xd_rt_basicFramegraph)
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
set(APP_FILES
        BasicFramegraphApp.h
        BasicFramegraphApp.cpp
)
set(NON_BUILD_FILES
        ../../.clang-format
)

set(SRC_FILES
        ${APP_FILES}
        ${NON_BUILD_FILES}
        ${IMGUI_FILES})
include_directories(../../3rdparty/imgui/)
add_executable(xd_rt_basicFramegraph ${SRC_FILES})
target_link_libraries(xd_rt_basicFramegraph xd_rt_appBase)
include_directories(${XD_REALTIME_INCLUDE})
add_tutorial_app(xd_rt_basicFramegraph "samples/basicFramegraph")
