//
// Created by Frank on 2024/1/30.
//
#include "ShaderBase.h"
#include "backend/vulkan/VulkanShader.h"

using namespace xd;
ShaderBase::ShaderBase(std::shared_ptr<VulkanShader> shader) : shader(std::move(shader)) {}
