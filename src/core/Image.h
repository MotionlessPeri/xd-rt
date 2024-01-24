//
// Created by Frank on 2024/1/3.
//

#ifndef XD_RT_IMAGE_H
#define XD_RT_IMAGE_H
#include <vector>
#include "Enums.h"
#include "MathTypes.h"
namespace xd {
inline uint32_t getPixelComponentFormatSize(PixelComponentFormat compFormat)
{
	switch (compFormat) {
		case PixelComponentFormat::U8:
			return sizeof(uint8_t);
		case PixelComponentFormat::SFLOAT:
			return sizeof(float);
		default:
			return -1;
	}
}
struct PixelFormatInfo {
	uint32_t componentCount;
	PixelComponentFormat componentFormat;
	ColorSpace colorSpace;
};

class PixelFormatHelper {
public:
	static PixelFormatInfo get(PixelFormat format);

private:
	static std::unordered_map<PixelFormat, PixelFormatInfo> pixelFormatInfos;
};

class Image2D {
public:
	friend class ImageFilter2D;
	Image2D(PixelFormat format,
			uint32_t width,
			uint32_t height,
			uint32_t stride,
			std::vector<uint8_t> data);
	ColorRGBA getPixelValue(uint32_t row, uint32_t col) const;
	ColorRGBA getPixelValue(uint32_t index) const;
	uint32_t getWidth() const { return extent.x(); }
	uint32_t getHeight() const { return extent.y(); }
	Vector2i getExtent() const { return extent; }
	PixelFormat getFormat() const { return format; }
	const std::vector<uint8_t>& getData() const { return data; }

private:
	uint32_t getPixelIndex(uint32_t row, uint32_t col) const;
	ColorRGBA getPixelValueByIndex(uint32_t index) const;
	ColorRGBA convertToLinear(const ColorRGBA& color, ColorSpace space) const;
	PixelFormat format;
	Vector2i extent;
	uint32_t stride = 0u;
	std::vector<uint8_t> data;	// we use vector to handle low-level memory
};

}  // namespace xd

#endif	// XD_RT_IMAGE_H
