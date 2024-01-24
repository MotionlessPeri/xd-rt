//
// Created by Frank on 2024/1/24.
//

#include "TextureFactoryVk.h"
#include <typeinfo>

#include "SamplerCache.h"
#include "TextureVk.h"
#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanSampler.h"
#include "filter/NearestFilter.h"
#include "filter/TentFilter.h"
#include "texture/ImageTexture.h"
using namespace xd;
TextureFactoryVk* TextureFactoryVk::singleton = nullptr;
void TextureFactoryVk::init(std::shared_ptr<VulkanDevice> device)
{
	singleton = new TextureFactoryVk{std::move(device)};
}

void TextureFactoryVk::terminate()
{
	delete singleton;
}

std::shared_ptr<TextureVk> TextureFactoryVk::buildTexture(
	std::shared_ptr<ImageTexture> texture,
	const BuildingTextureOptions& options) const
{
	const auto image = buildImage2D(texture->getImage(), options);
	const auto sampler = buildSampler(texture->getFilter(), options);

	VkImageViewCreateInfo viewCi;
	viewCi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCi.pNext = nullptr;
	viewCi.flags = 0;
	viewCi.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCi.format = image->getDesc().format;
	viewCi.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCi.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCi.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCi.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCi.subresourceRange.baseMipLevel = 0;
	viewCi.subresourceRange.levelCount = 1;
	viewCi.subresourceRange.baseArrayLayer = 0;
	viewCi.subresourceRange.layerCount = 1;
	const auto imageView = image->createImageView(std::move(viewCi));

	return std::make_shared<TextureVk>(image, imageView, sampler);
}

std::shared_ptr<VulkanImage> TextureFactoryVk::buildImage2D(
	std::shared_ptr<Image2D> image,
	const BuildingTextureOptions& options) const
{
	auto imageFormat = pixelFormatToVkFormat(image->getFormat());
	const auto& originData = image->getData();
	const std::vector<uint8_t>* actualData = &originData;
	std::vector<uint8_t> convertedData;
	static const std::unordered_map<VkFormat, VkFormat> conversionTypes{
		{VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8A8_UNORM},
		{VK_FORMAT_R8G8B8_SRGB, VK_FORMAT_R8G8B8A8_SRGB}};
	const auto needConversionIt = conversionTypes.find(imageFormat);
	if (needConversionIt != conversionTypes.end()) {
		// convert rgb8 to rgba8 cuz most vulkan devices do not support them
		const auto pixelCount = image->getWidth() * image->getHeight();
		convertedData =
			std::vector<uint8_t>(image->getWidth() * image->getHeight() * 4 * sizeof(uint8_t));
		// we can use chunk_view in c++23
		for (const auto pIndex : std::views::iota(0u, pixelCount)) {
			memcpy(&convertedData[4 * pIndex], &originData[3 * pIndex], 3 * sizeof(uint8_t));
			convertedData[4 * pIndex + 3] = 255;
		}
		actualData = &convertedData;
		imageFormat = needConversionIt->second;
	}
	VkImageCreateInfo ci;
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ci.pNext = nullptr;
	ci.flags = 0;
	ci.imageType = VK_IMAGE_TYPE_2D;
	ci.format = imageFormat;
	ci.extent.width = image->getWidth();
	ci.extent.height = image->getHeight();
	ci.extent.depth = 1;
	ci.mipLevels = 1;
	ci.arrayLayers = 1;
	ci.samples = VK_SAMPLE_COUNT_1_BIT;
	ci.tiling = VK_IMAGE_TILING_OPTIMAL;
	ci.usage = options.usage;
	ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ci.queueFamilyIndexCount = 0;
	ci.pQueueFamilyIndices = nullptr;
	ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	auto vkImage = device->createImage(ci, options.memoryProperties);
	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = VK_ACCESS_NONE;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	vkImage->transitState(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
						  std::move(barrier));
	vkImage->setData(0, (void*)actualData->data(),
					 actualData->size() * sizeof(actualData->front()));
	return vkImage;
}

std::shared_ptr<VulkanSampler> TextureFactoryVk::buildSampler(
	std::shared_ptr<ImageFilter2D> filter,
	const BuildingTextureOptions& options) const
{
	const auto determinFilterType = [](std::shared_ptr<ImageFilter2D> filter) -> VkFilter {
		const auto& filterType = typeid(*filter);
		if (filterType == typeid(TentFilter)) {
			return VK_FILTER_LINEAR;
		}
		else if (filterType == typeid(NearestFilter)) {
			return VK_FILTER_NEAREST;
		}
		else {
			// fall back to linear
			assert(false);
			return VK_FILTER_LINEAR;
		}
	};
	SamplerDesc samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.pNext = nullptr;
	samplerInfo.flags = 0;
	samplerInfo.magFilter = determinFilterType(filter);
	samplerInfo.minFilter = determinFilterType(filter);
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = wrapModeToSamplerAddressMode(filter->getWrapS());
	samplerInfo.addressModeV = wrapModeToSamplerAddressMode(filter->getWrapT());
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.mipLodBias = 0.f;
	samplerInfo.anisotropyEnable = options.enableAnistropy;
	samplerInfo.maxAnisotropy = options.maxAnistropy;
	samplerInfo.compareEnable = options.compareEnable;
	samplerInfo.compareOp = options.compareOp;
	samplerInfo.minLod = options.minLod;
	samplerInfo.maxLod = options.maxLod;
	samplerInfo.borderColor = options.borderColor;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	return SamplerCache::get().getOrCreate(samplerInfo);
}
