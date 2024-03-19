//
// Created by Frank on 2024/1/30.
//

#ifndef XD_RT_LAMBERTIANFRAGMENTSHADER_H
#define XD_RT_LAMBERTIANFRAGMENTSHADER_H
#include "ShaderBase.h"
#include "ShaderTraits.h"
#include "realtime/backend/vulkan/FrameGraph.h"
#include "realtime/backend/vulkan/VulkanTypes.h"
namespace xd {

class LambertianFragmentShader : public ShaderBase {
public:
	struct ShaderParams {};
};
template <>
struct ShaderTraits<LambertianFragmentShader>;
}  // namespace xd

#endif	// XD_RT_LAMBERTIANFRAGMENTSHADER_H
