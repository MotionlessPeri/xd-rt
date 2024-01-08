//
// Created by Frank on 2023/8/31.
//

#ifndef XD_RT_TEXTURE_H
#define XD_RT_TEXTURE_H
#include "Contexts.h"
#include "MathUtil.h"
namespace xd {
// template <typename ReturnType, typename SampleType>
// class Texture {
// public:
//	virtual ReturnType sample(const SampleType& sample) = 0;
// };

// TODO: the InputDim and OutputDim here regulates the type of input and output of sample method,
// but seems loses some flexibility. In pbrt, the InputType is handled by a TextureEvaluateContext
// class, which holds all the information all types of texture needs. This is a good approach that
// loses subtle performance for high usability. In the other hand, modern graphic APIs(like DX12 and
// Vulkan) hides the OutputDim into an RGBA format, assuming that all textures sample result can be
// contained in an RGBA color. I still don't know what's the best practice for this renderer, maybe
// we should change the texture implementation when needed.

class Texture {
public:
	Texture() = default;
	Texture(const Texture& other) = default;
	Texture(Texture&& other) noexcept = default;
	Texture& operator=(const Texture& other) = default;
	Texture& operator=(Texture&& other) noexcept = default;
	virtual ~Texture() = default;
	virtual ColorRGBA sample(const TextureEvalContext& ctx) const = 0;
};
}  // namespace xd
#endif	// XD_RT_TEXTURE_H
