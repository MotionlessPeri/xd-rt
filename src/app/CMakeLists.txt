cmake_minimum_required(VERSION 3.18)
project(xd_rt_app)
add_compile_definitions(PROJECT_ROOT=R\"\(${PROJECT_SOURCE_DIR}\)\")
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
set(GLFW_ROOT_DIR external/glfw3)
find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
set(UTIL_FILES
        CameraHandler.cpp
        CameraHandler.h)
set(VULKAN_FILES
        vulkan/VulkanGLFWApp.cpp
        vulkan/VulkanGLFWApp.h
        vulkan/Types.h
        vulkan/GLFWGlobal.h
        vulkan/GLFWInclude.h
)
set(NON_BUILD_FILES
        ../../.clang-format
)
set(SHADER_FILES
        vulkan/shader/test.frag
        vulkan/shader/test.vert
)
file(GLOB IMGUI_HEADERS ../3rdparty/imgui/*.h)
file(GLOB IMGUI_SOURCES ../3rdparty/imgui/*.cpp)

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
        ${SHADER_FILES}
        ${UTIL_FILES})
source_group(vulkan FILES ${VULKAN_FILES})
source_group(imgui FILES ${IMGUI_FILES})
source_group(shader FILES ${SHADER_FILES})
source_group(util FILES ${UTIL_FILES})

add_library(xd_rt_app STATIC ${SRC_FILES})
target_link_libraries(xd_rt_app ${GLFW_LIBRARIES})
target_link_libraries(xd_rt_app xd_rt)

file(GLOB files "vulkan/shader/*.vert" "vulkan/shader/*.frag")
# Custom target will always cause its dependencies to be evaluated and is
# run by default
#add_custom_target(compile_app_shaders ALL
#        DEPENDS dummy_output)
#add_custom_command(
#        OUTPUT dummy_output
#        DEPENDS always_rebuild
#)
#add_custom_command(
#        OUTPUT always_rebuild
#        COMMAND ${CMAKE_COMMAND} -E echo
#)
# add_dependencies(xd_rt_app compile_shaders)

set(generated_spvs)
foreach (file ${files})
    add_custom_command(
            OUTPUT ${file}.spv
            COMMAND Vulkan::glslc ${file} -o ${file}.spv
            COMMAND ${CMAKE_COMMAND} -E echo "Compiling ${file} to ${file}.spv"
            MAIN_DEPENDENCY ${file}
            VERBATIM
    )
    list(APPEND generated_spvs ${file}.spv)
endforeach ()
add_custom_target(compile_app_shaders ALL DEPENDS ${generated_spvs})
add_dependencies(xd_rt_app compile_app_shaders)