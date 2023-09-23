//
// Created by Frank on 2023/8/31.
//

#ifndef XD_RT_TEXTURE_H
#define XD_RT_TEXTURE_H
#include "CoreTypes.h"
#include "MathUtil.h"
namespace xd {
template <typename ReturnType, typename SampleType>
class Texture {
public:
	virtual ReturnType sample(const SampleType& sample) = 0;
};

template <typename ReturnType, typename SampleType>
class ConstantTexture : public Texture<ReturnType, SampleType> {
public:
	explicit ConstantTexture(ReturnType c) : c(c) {}
	ReturnType sample(const SampleType& sample) override { return c; }

protected:
	ReturnType c;
};

template <typename ReturnType>
class UVTexture : public Texture2D<ReturnType> {
public:
	UVTexture(const std::vector<ReturnType>& data, uint32_t width, uint32_t height)
		: data(data), width(width), height(height)
	{
	}
	ReturnType sample(const Vector2f& sample) override
	{
		const uint32_t col = sample.x() * width;
		const uint32_t row = sample.y() * height;
		const uint32_t index = row * width + col;
		return data[index];
	}

protected:
	std::vector<ReturnType> data;
	uint32_t width, height;
};

template <typename ReturnType>
class SphereTexture : public Texture3D<ReturnType> {
public:
	SphereTexture(const std::vector<ReturnType>& data, uint32_t width, uint32_t height)
		: data(data), width(width), height(height)
	{
	}
	ReturnType sample(const Vector3f& sample) override
	{
		const auto uv = getSphereUV(sample);
		const uint32_t col = std::clamp<uint32_t>(uv.x() * width, 0, width - 1);
		const uint32_t row = std::clamp<uint32_t>(uv.y() * height, 0, height - 1);
		const uint32_t index = row * width + col;
		return data[index];
	}
	const std::vector<ReturnType>& getImage() const { return data; }
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }

protected:
	std::vector<ReturnType> data;
	uint32_t width;
	uint32_t height;
};
}  // namespace xd
#endif	// XD_RT_TEXTURE_H
