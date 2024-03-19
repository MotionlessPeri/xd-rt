cmake_minimum_required(VERSION 3.24)
project(xd_realtime)
set(USE_VULKAN TRUE)
set(PLATFORM_WINDOWS TRUE)
set(CMAKE_CXX_STANDARD 20)
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
add_compile_definitions(PROJECT_ROOT=R\"\(${PROJECT_SOURCE_DIR}\)\")

set(VULKAN_FILES
        backend/vulkan/VulkanDevice.cpp
        backend/vulkan/VulkanDevice.h
        backend/vulkan/VulkanDeviceObject.h
        backend/vulkan/VulkanImage.cpp
        backend/vulkan/VulkanImage.h
        backend/vulkan/VulkanImageView.cpp
        backend/vulkan/VulkanImageView.h
        backend/vulkan/VulkanInstance.cpp
        backend/vulkan/VulkanInstance.h
        backend/vulkan/VulkanMacros.h
        backend/vulkan/VulkanPhysicalDevice.cpp
        backend/vulkan/VulkanPhysicalDevice.h
        backend/vulkan/VulkanPlatformSpecific.h
        backend/vulkan/VulkanQueue.cpp
        backend/vulkan/VulkanQueue.h
        backend/vulkan/VulkanShader.cpp
        backend/vulkan/VulkanShader.h
        backend/vulkan/VulkanSurface.cpp
        backend/vulkan/VulkanSurface.h
        backend/vulkan/VulkanSwapchain.cpp
        backend/vulkan/VulkanSwapchain.h
        backend/vulkan/VulkanTypes.h
        backend/vulkan/VulkanGraphicsPipeline.cpp
        backend/vulkan/VulkanGraphicsPipeline.h
        backend/vulkan/ModelFactoryVk.cpp
        backend/vulkan/ModelFactoryVk.h
        backend/vulkan/TriangleMeshVk.cpp
        backend/vulkan/TriangleMeshVk.h
        backend/vulkan/VulkanBuffer.cpp
        backend/vulkan/VulkanBuffer.h
        backend/vulkan/VulkanMemory.cpp
        backend/vulkan/VulkanMemory.h
        backend/vulkan/VulkanCommandQueue.cpp
        backend/vulkan/VulkanCommandQueue.h
        backend/vulkan/VulkanCommandBuffer.cpp
        backend/vulkan/VulkanCommandBuffer.h
        backend/vulkan/VulkanCommandPool.cpp
        backend/vulkan/VulkanCommandPool.h
        backend/vulkan/VulkanGlobal.cpp
        backend/vulkan/VulkanGlobal.h
        backend/vulkan/MaterialTemplateVk.cpp
        backend/vulkan/MaterialTemplateVk.h
        backend/vulkan/MaterialInstanceVk.cpp
        backend/vulkan/MaterialInstanceVk.h
        backend/vulkan/VulkanDescriptorSetLayout.cpp
        backend/vulkan/VulkanDescriptorSetLayout.h
        backend/vulkan/VulkanDescriptorSet.cpp
        backend/vulkan/VulkanDescriptorSet.h
        backend/vulkan/VulkanDescriptorPool.cpp
        backend/vulkan/VulkanDescriptorPool.h
        backend/vulkan/VulkanSubpass.cpp
        backend/vulkan/VulkanSubpass.h
        backend/vulkan/VulkanRenderPass.cpp
        backend/vulkan/VulkanRenderPass.h
        backend/vulkan/VulkanFrameBuffer.cpp
        backend/vulkan/VulkanFrameBuffer.h
        backend/vulkan/FrameGraph.cpp
        backend/vulkan/FrameGraph.h
        backend/vulkan/VulkanDescs.h
        backend/vulkan/VulkanFence.cpp
        backend/vulkan/VulkanFence.h
        backend/vulkan/VulkanSemaphore.cpp
        backend/vulkan/VulkanSemaphore.h
        backend/vulkan/TextureFactoryVk.cpp
        backend/vulkan/TextureFactoryVk.h
        backend/vulkan/TextureVk.cpp
        backend/vulkan/TextureVk.h
        backend/vulkan/VulkanSampler.cpp
        backend/vulkan/VulkanSampler.h
        backend/vulkan/SamplerCache.cpp
        backend/vulkan/SamplerCache.h
        backend/vulkan/MaterialFactoryVk.cpp
        backend/vulkan/MaterialFactoryVk.h
        backend/vulkan/VulkanPipelineLayout.cpp
        backend/vulkan/VulkanPipelineLayout.h
        backend/vulkan/LightManager.cpp
        backend/vulkan/LightManager.h
        backend/vulkan/VulkanComputePipeline.cpp
        backend/vulkan/VulkanComputePipeline.h
        backend/vulkan/VulkanPipelineBase.cpp
        backend/vulkan/VulkanPipelineBase.h
)

set(VULKAN_SHADER_FILES
        backend/vulkan/shader/LambertianFragmentShader.cpp
        backend/vulkan/shader/LambertianFragmentShader.h
        backend/vulkan/shader/ShaderTraits.h
        backend/vulkan/shader/ShaderBase.h
        backend/vulkan/shader/ShaderBase.cpp
        backend/vulkan/shader/MainVertexShader.cpp
        backend/vulkan/shader/MainVertexShader.h)

set(SRC_FILES
        ${VULKAN_FILES}
        ${VULKAN_SHADER_FILES}
)

set(NON_BUILD_FILES
        ../../.clang-format
        ../../eigen.natvis
)
source_group(backend/vulkan FILES ${VULKAN_FILES})
source_group(backend/vulkan/shader FILES ${VULKAN_SHADER_FILES})

add_library(xd_realtime STATIC ${SRC_FILES})

find_package(Vulkan REQUIRED)
message("Vulkan Version ${Vulkan_VERSION} ${Vulkan_INCLUDE_DIRS} ${Vulkan_LIBRARIES} ${Vulkan_glslc_FOUND}")
include_directories(${Vulkan_INCLUDE_DIRS})

set(GLFW_ROOT_DIR "${CMAKE_SOURCE_DIR}/external/glfw3")
find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})

target_link_libraries(xd_realtime ${Vulkan_LIBRARIES})
target_link_libraries(xd_realtime ${GLFW_LIBRARIES})
target_link_libraries(xd_realtime xd_raytracing)

file(GLOB files "src/realtime/backend/vulkan/shader/*.vert" "src/realtime/backend/vulkan/shader/*.frag" "src/realtime/backend/vulkan/shader/*.comp")
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
add_custom_target(compile_shaders ALL DEPENDS ${generated_spvs})
add_dependencies(xd_realtime compile_shaders)
unset(generated_spvs)

set(XD_REALTIME_INCLUDE "${CMAKE_SOURCE_DIR}/src/realtime/" ${Vulkan_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} CACHE INTERNAL "XD_REALTIME_INCLUDE")