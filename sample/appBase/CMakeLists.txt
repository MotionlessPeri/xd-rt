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

set(SRC_FILES
        ${VULKAN_FILES}
        ${NON_BUILD_FILES}
)
source_group(vulkan FILES ${VULKAN_FILES})
source_group(util FILES ${UTIL_FILES})

add_library(xd_rt_appBase STATIC ${SRC_FILES})
target_link_libraries(xd_rt_appBase ${GLFW_LIBRARIES})
target_link_libraries(xd_rt_appBase xd_realtime)
include_directories(${XD_REALTIME_INCLUDE})

add_tutorial_app(xd_rt_appBase "samples/appBase")
