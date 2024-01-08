//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_CONSTANTTEXTURE_H
#define XD_RT_CONSTANTTEXTURE_H
#include "Texture.h"

namespace xd {
// template <typename ReturnType, typename SampleType>
// class ConstantTexture : public Texture<ReturnType, SampleType> {
// public:
//	explicit ConstantTexture(ReturnType c) : c(std::move(c)) {}
//	ReturnType sample(const SampleType& sample) override { return c; }
//
// protected:
//	ReturnType c;
// };

class ConstantTexture : public Texture {
public:
	explicit ConstantTexture(float f) : c(f, 0, 0, 1) {}
	explicit ConstantTexture(ColorRGB c) : c(c.x(), c.y(), c.z(), 1.f) {}
	explicit ConstantTexture(ColorRGBA c) : c(std::move(c)) {}

	ColorRGBA sample(const TextureEvalContext& ctx) const override { return c; }

protected:
	ColorRGBA c;
};

}  // namespace xd
#endif	// XD_RT_CONSTANTTEXTURE_H
