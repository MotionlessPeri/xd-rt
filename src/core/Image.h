//
// Created by Frank on 2024/1/3.
//

#ifndef XD_RT_IMAGE_H
#define XD_RT_IMAGE_H
#include <MathTypes.h>

#include <vector>
namespace xd {
enum class ColorSpace { UNKNOWN, LINEAR, SRGB };
enum class PixelFormat {
	FORMAT_UNKNOWN,
	FORMAT_R8G8B8_UNORM,
	FORMAT_R8G8B8_SRGB,
	FORMAT_R8G8B8A8_UNORM,
	FORMAT_R8G8B8A8_SRGB,
	FORMAT_R32G32B32_SFLOAT,
	FORMAT_R32G32B32A32_SFLOAT,
};
enum class PixelComponentFormat { UNKONWN, U8, SFLOAT };

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
	Image2D(PixelFormat format, uint32_t width, uint32_t height, std::vector<uint8_t>&& data);
	ColorRGBA getPixelValue(uint32_t row, uint32_t col) const;

private:
	uint32_t getPixelIndex(uint32_t row, uint32_t col) const;
	ColorRGBA getPixelValueByIndex(uint32_t index) const;
	ColorRGBA convertToLinear(const ColorRGBA& color, ColorSpace space) const;
	PixelFormat format;
	uint32_t width, height;
	std::vector<uint8_t> data;	// we use vector to handle low-level memory
};

}  // namespace xd

#endif	// XD_RT_IMAGE_H
