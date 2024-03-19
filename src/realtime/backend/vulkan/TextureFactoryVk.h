//
// Created by Frank on 2024/1/24.
//

#ifndef XD_RT_TEXTUREFACTORYVK_H
#define XD_RT_TEXTUREFACTORYVK_H
#include <memory>
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
#include "texture/ImageTexture.h"
namespace xd {
inline VkFormat pixelFormatToVkFormat(PixelFormat format)
{
	switch (format) {
		case PixelFormat::FORMAT_R8G8B8_UNORM:
			return VK_FORMAT_R8G8B8_UNORM;
		case PixelFormat::FORMAT_R8G8B8_SRGB:
			return VK_FORMAT_R8G8B8_SRGB;
		case PixelFormat::FORMAT_R8G8B8A8_UNORM:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case PixelFormat::FORMAT_R8G8B8A8_SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case PixelFormat::FORMAT_R32G32B32_SFLOAT:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case PixelFormat::FORMAT_R32G32B32A32_SFLOAT:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		default:
			return VK_FORMAT_UNDEFINED;
			;
	}
}
inline VkSamplerAddressMode wrapModeToSamplerAddressMode(WrapMode mode)
{
	switch (mode) {
		case WrapMode::CLAMP:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case WrapMode::REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		default: {
			assert(false);
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}
	}
}
struct BuildingTextureOptions {
	VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkBool32 enableAnistropy = VK_FALSE;
	float maxAnistropy = 1.f;
	VkBool32 compareEnable = VK_FALSE;
	VkCompareOp compareOp = VK_COMPARE_OP_NEVER;
	float minLod = 0.f;
	float maxLod = 0.f;
	VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
};
class TextureFactoryVk {
public:
	static void init(std::shared_ptr<VulkanDevice> device);
	static void terminate();
	static TextureFactoryVk& get() { return *singleton; }
	TextureFactoryVk(const TextureFactoryVk& other) = delete;
	TextureFactoryVk(TextureFactoryVk&& other) noexcept = delete;
	TextureFactoryVk& operator=(const TextureFactoryVk& other) = delete;
	TextureFactoryVk& operator=(TextureFactoryVk&& other) noexcept = delete;
	std::shared_ptr<TextureVk> buildTexture(std::shared_ptr<ImageTexture> texture,
											const BuildingTextureOptions& options = {}) const;

private:
	std::shared_ptr<VulkanImage> buildImage2D(std::shared_ptr<Image2D> image,
											  const BuildingTextureOptions& options) const;
	std::shared_ptr<VulkanSampler> buildSampler(std::shared_ptr<ImageFilter2D> filter,
												const BuildingTextureOptions& options) const;
	explicit TextureFactoryVk(std::shared_ptr<VulkanDevice> device) : device(std::move(device)) {}

	std::shared_ptr<VulkanDevice> device;
	static TextureFactoryVk* singleton;
};

}  // namespace xd

#endif	// XD_RT_TEXTUREFACTORYVK_H
