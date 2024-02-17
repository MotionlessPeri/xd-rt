cmake_minimum_required(VERSION 3.18)
project(xd_rt_appBase)
add_compile_definitions(PROJECT_ROOT=R\"\(${PROJECT_SOURCE_DIR}\)\")
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
set(GLFW_ROOT_DIR external/glfw3)
find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
set(VULKAN_FILES
        VulkanGLFWAppBase.cpp
        VulkanGLFWAppBase.h
        GLFWGlobal.h
        GLFWInclude.h
)
set(NON_BUILD_FILES
        ../../.clang-format
)
file(GLOB IMGUI_HEADERS ../../3rdparty/imgui/*.h)
file(GLOB IMGUI_SOURCES ../../3rdparty/imgui/*.cpp)

set(IMGUI_FILES ${IMGUI_HEADERS} ${IMGUI_SOURCES})

include_directories(../../3rdparty/imgui/)
include_directories(../../3rdparty/imgui/backends/)
if (USE_VULKAN)
    set(IMGUI_FILES
            ${IMGUI_FILES}
            ../../3rdparty/imgui/backends/imgui_impl_vulkan.h
            ../../3rdparty/imgui/backends/imgui_impl_vulkan.cpp
            ../../3rdparty/imgui/backends/imgui_impl_glfw.h
            ../../3rdparty/imgui/backends/imgui_impl_glfw.cpp
    )
endif ()

set(SRC_FILES
        ${VULKAN_FILES}
        ${IMGUI_FILES}
        ${NON_BUILD_FILES}
)
source_group(vulkan FILES ${VULKAN_FILES})
source_group(imgui FILES ${IMGUI_FILES})
source_group(util FILES ${UTIL_FILES})

add_library(xd_rt_appBase STATIC ${SRC_FILES})
target_link_libraries(xd_rt_appBase ${GLFW_LIBRARIES})
target_link_libraries(xd_rt_appBase xd_rt)

add_tutorial_app(xd_rt_appBase "tutorials/appBase")
