//
// Created by Frank on 2023/9/19.
//
#include "TextureFactory.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif
#include "filter/NearestFilter.h"
#include "filter/TentFilter.h"
#include "mapping/EquirectangularMapping.h"
#include "mapping/UVMapping.h"
#include "texture/ImageTexture.h"
using namespace xd;

std::shared_ptr<ImageTexture> TextureFactory::loadUVTexture(const std::string& path,
															   LoadTextureOptions options) const
{
	int width, height, channels;
	auto* rawData = stbi_load(path.c_str(), &width, &height, &channels, 0);
	if (rawData == nullptr)
		assert(false);
	const auto componentCnt = width * height * channels;
	const auto image = std::make_shared<Image2D>(
		chooseIntegerPixelFormat(channels, options.isSrgb), width, height, 0u,
		std::vector<uint8_t>{rawData, rawData + componentCnt * sizeof(uint8_t)});
	STBI_FREE(rawData);
	const auto filter = createFilter(options.filterType, options.wrapS, options.wrapT);
	const auto mapping = std::make_shared<UVMapping>();
	return std::make_shared<ImageTexture>(image, filter, mapping);
}

std::shared_ptr<ImageTexture> TextureFactory::loadSphereTexture(const std::string& path,
																   LoadTextureOptions options) const
{
	int width, height, channels;
	auto* rawData = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
	if (rawData == nullptr)
		assert(false);
	const auto* convertedData = reinterpret_cast<const uint8_t*>(rawData);
	const auto image = std::make_shared<Image2D>(
		chooseFloatingPixelFormat(channels), width, height, 0u,
		std::vector<uint8_t>{convertedData,
							 convertedData + width * height * channels * sizeof(float)});
	STBI_FREE(rawData);
	const auto filter = createFilter(options.filterType, options.wrapS, options.wrapT);
	const auto mapping = std::make_shared<EquirectangularMapping>();
	return std::make_shared<ImageTexture>(image, filter, mapping);
}

PixelFormat TextureFactory::chooseIntegerPixelFormat(int channels, bool isSrgb)
{
	if (channels == 3) {
		return isSrgb ? PixelFormat::FORMAT_R8G8B8_SRGB : PixelFormat::FORMAT_R8G8B8_UNORM;
	}
	if (channels == 4) {
		return isSrgb ? PixelFormat::FORMAT_R8G8B8A8_SRGB : PixelFormat::FORMAT_R8G8B8A8_UNORM;
	}
	assert(false);
	return PixelFormat::FORMAT_UNKNOWN;
}
PixelFormat TextureFactory::chooseFloatingPixelFormat(int channels)
{
	if (channels == 3)
		return PixelFormat::FORMAT_R32G32B32_SFLOAT;
	if (channels == 4)
		return PixelFormat::FORMAT_R32G32B32A32_SFLOAT;
	assert(false);
	return PixelFormat::FORMAT_UNKNOWN;
}

std::shared_ptr<ImageFilter2D> TextureFactory::createFilter(FilterType filterType,
															   WrapMode wrapS,
															   WrapMode wrapT)
{
	switch (filterType) {
		case FilterType::NEAREST:
			return std::make_shared<NearestFilter>(wrapS, wrapT);
		case FilterType::TENT:
			return std::make_shared<TentFilter>(wrapS, wrapT);
		default: {
			assert(false);
			return nullptr;
		}
	}
}
