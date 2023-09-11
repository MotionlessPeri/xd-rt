//
// Created by Frank on 2023/8/31.
//

#ifndef XD_RT_TEXTURE_H
#define XD_RT_TEXTURE_H
#include "CoreTypes.h"
namespace xd {
template <typename ReturnType, typename SampleType>
class Texture {
public:
	virtual ReturnType sample(const SampleType& sample) = 0;
};

template <typename ReturnType>
class ConstantTexture : public Texture2D<ReturnType> {
public:
	explicit ConstantTexture(ReturnType c) : c(c) {}
	ReturnType sample(const Vector2f& sample) override { return c; }

protected:
	ReturnType c;
};

}  // namespace xd
#endif	// XD_RT_TEXTURE_H
