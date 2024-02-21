cmake_minimum_required(VERSION 3.18)
project(xd_rt_basicGraphic)
add_compile_definitions(PROJECT_ROOT=R\"\(${PROJECT_SOURCE_DIR}\)\")
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
set(GLFW_ROOT_DIR external/glfw3)
find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
set(APP_FILES
        BasicGraphicApp.cpp
        BasicGraphicApp.h
)
set(NON_BUILD_FILES
        ../../.clang-format
)
set(SHADER_FILES
        shader/test.frag
        shader/test.vert
)

set(SRC_FILES
        ${APP_FILES}
        ${NON_BUILD_FILES}
        ${SHADER_FILES})
source_group(shader FILES ${SHADER_FILES})

add_executable(xd_rt_basicGraphic ${SRC_FILES})
target_link_libraries(xd_rt_basicGraphic xd_rt_appBase)

file(GLOB files "shader/*.vert" "shader/*.frag")

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
add_custom_target(compile_shaders_basicGraphic ALL DEPENDS ${generated_spvs})
add_dependencies(xd_rt_basicGraphic compile_shaders_basicGraphic)
unset(generated_spvs)
set_target_properties(compile_shaders_basicGraphic PROPERTIES
        FOLDER "tutorials/basicGraphic"
)
add_tutorial_app(xd_rt_basicGraphic "tutorials/basicGraphic")
unset(compile_app_shaders)
