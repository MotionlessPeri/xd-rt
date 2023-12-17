//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_UVTEXTURE_H
#define XD_RT_UVTEXTURE_H
#include <vector>
#include "Texture.h"
namespace xd {
template <typename ReturnType>
class UVTexture : public Texture2D<ReturnType> {
public:
	ReturnType sample(const Vector2f& sample) override
	{
		// static_assert(false);
		return {};
	}
};

template <>
class UVTexture<float> : public Texture2D<float> {
public:
	UVTexture(const std::vector<float>& data, uint32_t width, uint32_t height)
		: data(data), width(width), height(height)
	{
	}
	float sample(const Vector2f& sample) override
	{
		const uint32_t col = sample.x() * width;
		const uint32_t row = sample.y() * height;
		const uint32_t index = row * width + col;
		return data[index];
	}

protected:
	std::vector<float> data;
	uint32_t width, height;
};

template <>
class UVTexture<ColorRGB> : public Texture2D<ColorRGB> {
public:
	UVTexture(const std::vector<float>& data, uint32_t width, uint32_t height)
		: data(data), width(width), height(height)
	{
	}
	ColorRGB sample(const Vector2f& sample) override
	{
		const uint32_t col = sample.x() * width;
		const uint32_t row = sample.y() * height;
		const uint32_t index = row * width + col;
		return {data[3 * index], data[3 * index + 1], data[3 * index + 2]};
	}

protected:
	std::vector<float> data;
	uint32_t width, height;
};

template <>
class UVTexture<ColorRGBA> : public Texture2D<ColorRGBA> {
public:
	UVTexture(const std::vector<float>& data, uint32_t width, uint32_t height)
		: data(data), width(width), height(height)
	{
	}
	ColorRGBA sample(const Vector2f& sample) override
	{
		const uint32_t col = sample.x() * width;
		const uint32_t row = sample.y() * height;
		const uint32_t index = row * width + col;
		return {data[4 * index], data[4 * index + 1], data[4 * index + 2], data[4 * index + 3]};
	}

protected:
	std::vector<float> data;
	uint32_t width, height;
};
}  // namespace xd
#endif	// XD_RT_UVTEXTURE_H
