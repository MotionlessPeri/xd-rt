//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_UVTEXTURE_H
#define XD_RT_UVTEXTURE_H
#include <ranges>
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
	UVTexture(std::vector<float>&& data, uint32_t width, uint32_t height)
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
	UVTexture(std::vector<float> data, uint32_t width, uint32_t height)
		: data(std::move(data)), width(width), height(height)
	{
	}
	ColorRGB sample(const Vector2f& sample) override
	{
		const uint32_t col = std::clamp<uint32_t>(sample.x() * width, 0, width - 1);
		const uint32_t row = std::clamp<uint32_t>(sample.y() * height, 0, height - 1);
		const uint32_t index = row * width + col;
		// return {data[3 * index], data[3 * index + 1], data[3 * index + 2]};

		// Test code for bi-linear filter
		const float x = sample.x() * width;
		const float y = sample.y() * height;
		const float dx = x - col;
		const float dy = y - row;
		std::array<ColorRGB, 4> values;
		const auto getColor = [&](uint32_t row, uint32_t col) -> ColorRGB {
			// Note: set wrap mode to clamp
			row = std::clamp(row, 0u, height - 1);
			col = std::clamp(col, 0u, width - 1);
			const auto index = row * width + col;
			return {data[3 * index], data[3 * index + 1], data[3 * index + 2]};
		};
		values[0] = getColor(row, col);
		values[1] = getColor(row, col + 1);
		values[2] = getColor(row + 1, col);
		values[3] = getColor(row + 1, col + 1);
		return ((1 - dx) * (1 - dy) * values[0] + dx * (1 - dy) * values[1] +
				(1 - dx) * dy * values[2] + dx * dy * values[3]);
	}

protected:
	std::vector<float> data;
	uint32_t width, height;
};

template <>
class UVTexture<ColorRGBA> : public Texture2D<ColorRGBA> {
public:
	UVTexture(std::vector<float> data, uint32_t width, uint32_t height)
		: data(std::move(data)), width(width), height(height)
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
