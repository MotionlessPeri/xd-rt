//
// Created by Frank on 2023/9/19.
//
#include "TextureFactory.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif
using namespace xd;

std::shared_ptr<SphereTexture<Vector3f>> TextureFactory::loadSphereTextureRGB(
	const std::string& path)
{
	int width, height, channels;
	auto* rawData = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
	const int pixelCount = width * height;
	std::vector<Vector3f> textureData;
	textureData.reserve(pixelCount);
	if (channels < 3)
		__debugbreak();
	assert(channels >= 3);
	for (auto i = 0u; i < pixelCount; ++i) {
		textureData.emplace_back(rawData[channels * i], rawData[channels * i + 1],
								 rawData[channels * i + 2]);
	}
	return std::make_shared<SphereTexture<Vector3f>>(textureData, (uint32_t)width,
													 (uint32_t)height);
}
std::shared_ptr<UVTexture<ColorRGB>> TextureFactory::loadUVTextureRGB(const std::string& path)
{
	int width, height, channels;
	auto* rawData = stbi_load(path.c_str(), &width, &height, &channels, 0);
	const auto componentCnt = width * height * channels;
	std::vector<float> textureData;
	for (auto i = 0u; i < componentCnt; ++i) {
		textureData.emplace_back(rawData[i]);
		textureData.back() /= 255.f;
	}
	return std::make_shared<UVTextureRGB>(textureData, (uint32_t)width, (uint32_t)height);
}
