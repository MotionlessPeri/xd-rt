//
// Created by Frank on 2023/9/19.
//

#ifndef XD_RT_TEXTUREFACTORY_H
#define XD_RT_TEXTUREFACTORY_H
#include <unordered_map>

#include "Filter.h"
#include "Image.h"
#include "Texture.h"
#include "texture/ImageTexture.h"

namespace xd {
enum class FilterType { UNKNOWN, NEAREST, TENT };
struct LoadTextureOptions {
	FilterType filterType = FilterType::NEAREST;
	bool isSrgb = false;
	WrapMode wrapS = WrapMode::CLAMP;
	WrapMode wrapT = WrapMode::CLAMP;
};
class TextureFactory {
public:
	// Note: the singleton can not be created by multiple threads simultaneously, considering init
	// it first by calling in the beginning of the program
	static TextureFactory& get()
	{
		static TextureFactory* singleton = new TextureFactory;
		return *singleton;
	}
	TextureFactory(const TextureFactory& other) = delete;
	TextureFactory(TextureFactory&& other) noexcept = delete;
	TextureFactory& operator=(const TextureFactory& other) = delete;
	TextureFactory& operator=(TextureFactory&& other) noexcept = delete;

	std::shared_ptr<ImageTexture> loadUVTexture(const std::string& path,
												LoadTextureOptions options = {}) const;
	std::shared_ptr<ImageTexture> loadSphereTexture(const std::string& path,
													LoadTextureOptions options = {}) const;

private:
	static PixelFormat chooseIntegerPixelFormat(int channels, bool isSrgb);
	static PixelFormat chooseFloatingPixelFormat(int channels);
	static std::shared_ptr<ImageFilter2D> createFilter(FilterType filterType,
													   WrapMode wrapS,
													   WrapMode wrapT);
	TextureFactory() = default;
	~TextureFactory() = default;
};
}  // namespace xd
#endif	// XD_RT_TEXTUREFACTORY_H
