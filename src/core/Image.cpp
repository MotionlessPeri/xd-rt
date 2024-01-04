//
// Created by Frank on 2024/1/3.
//

#include "Image.h"

#include <cassert>
#include <ranges>

#include "MathUtil.h"

using namespace xd;
#define MAKE_PIXEL_FORMAT_INFO(pixelFormat, compCnt, compFormat, cSpace) \
	{                                                                    \
		pixelFormat,                                                     \
		{                                                                \
			compCnt, compFormat, cSpace                                  \
		}                                                                \
	}

std::unordered_map<PixelFormat, PixelFormatInfo> PixelFormatHelper::pixelFormatInfos{
	MAKE_PIXEL_FORMAT_INFO(PixelFormat::FORMAT_R8G8B8_UNORM,
						   3,
						   PixelComponentFormat::U8,
						   ColorSpace::LINEAR),
	MAKE_PIXEL_FORMAT_INFO(PixelFormat::FORMAT_R8G8B8_SRGB,
						   3,
						   PixelComponentFormat::U8,
						   ColorSpace::SRGB),
	MAKE_PIXEL_FORMAT_INFO(PixelFormat::FORMAT_R8G8B8A8_UNORM,
						   4,
						   PixelComponentFormat::U8,
						   ColorSpace::LINEAR),
	MAKE_PIXEL_FORMAT_INFO(PixelFormat::FORMAT_R8G8B8A8_SRGB,
						   4,
						   PixelComponentFormat::U8,
						   ColorSpace::SRGB),
	MAKE_PIXEL_FORMAT_INFO(PixelFormat::FORMAT_R32G32B32_SFLOAT,
						   3,
						   PixelComponentFormat::SFLOAT,
						   ColorSpace::LINEAR),
	MAKE_PIXEL_FORMAT_INFO(PixelFormat::FORMAT_R32G32B32A32_SFLOAT,
						   4,
						   PixelComponentFormat::SFLOAT,
						   ColorSpace::LINEAR)};

PixelFormatInfo PixelFormatHelper::get(PixelFormat format)
{
	const auto findIt = pixelFormatInfos.find(format);
	if (findIt == pixelFormatInfos.end())
		assert(false);
	return findIt->second;
}

Image2D::Image2D(PixelFormat format, uint32_t width, uint32_t height, std::vector<uint8_t>&& data)
	: format(format), width(width), height(height), data(data)
{
}
ColorRGBA Image2D::getPixelValue(uint32_t row, uint32_t col) const
{
	return getPixelValueByIndex(getPixelIndex(row, col));
}

uint32_t Image2D::getPixelIndex(uint32_t row, uint32_t col) const
{
	return row * width + col;
}

template <typename CompType>
float normalize(CompType val)
{
	return val;
}
template <>
float normalize<uint8_t>(uint8_t val)
{
	return float(val) / float(std::numeric_limits<uint8_t>::max());
}
template <>
float normalize<uint16_t>(uint16_t val)
{
	return float(val) / float(std::numeric_limits<uint16_t>::max());
}
template <typename CompType>
ColorRGBA getPixelColor(const uint8_t* ptr, uint32_t compCnt)
{
	const auto* convertedPtr = reinterpret_cast<const CompType*>(ptr);
	ColorRGBA res;
	res.w() = 1.f;
	for (auto i : std::views::iota(0u, compCnt)) {
		res[i] = normalize(convertedPtr[i]);
	}
	return res;
}
ColorRGBA Image2D::getPixelValueByIndex(uint32_t index) const
{
	const auto pixelFormatInfo = PixelFormatHelper::get(format);
	const auto pixelLength = pixelFormatInfo.componentCount *
							 getPixelComponentFormatSize(pixelFormatInfo.componentFormat);
	const auto* ptr = data.data() + index * pixelLength;
	switch (pixelFormatInfo.componentFormat) {
		case PixelComponentFormat::U8:
			return convertToLinear(getPixelColor<uint8_t>(ptr, pixelFormatInfo.componentCount),
								   pixelFormatInfo.colorSpace);
		case PixelComponentFormat::SFLOAT:
			return convertToLinear(getPixelColor<float>(ptr, pixelFormatInfo.componentCount),
								   pixelFormatInfo.colorSpace);
		default: {
			assert(false);
			return {};
		}
	}
}

ColorRGBA Image2D::convertToLinear(const ColorRGBA& color, ColorSpace space) const
{
	switch (space) {
		case ColorSpace::LINEAR:
			return color;
		case ColorSpace::SRGB:
			return SRGBToLinear(color);
		default: {
			assert(false);
			return {};
		}
	}
}
