//
// Created by Frank on 2024/1/26.
//

#include "LightManager.h"
#include "ConcurrentUtil.h"
#include "MaterialInstanceVk.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"
using namespace xd;
LightManager::LightManager(std::shared_ptr<VulkanDevice> _device) : device(std::move(_device))
{
	{
		VkBufferCreateInfo pointLightBufferCi;
		pointLightBufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		pointLightBufferCi.pNext = nullptr;
		pointLightBufferCi.flags = 0;
		pointLightBufferCi.size = MAX_POINT_LIGHT_COUNT * sizeof(PointLightInfo);
		pointLightBufferCi.usage =
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		pointLightBufferCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		pointLightBuffer =
			device->createBuffer(pointLightBufferCi, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		std::vector<uint8_t> test(pointLightBufferCi.size, 101);
		pointLightBuffer->setData(0, test.data(), pointLightBufferCi.size);
	}
	{
		VkBufferCreateInfo lightIndexBufferCi;
		lightIndexBufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		lightIndexBufferCi.pNext = nullptr;
		lightIndexBufferCi.flags = 0;
		lightIndexBufferCi.size =
			MAX_LIGHT_INDEX_COUNT * sizeof(LightIndex) + sizeof(uint32_t) /*size*/;
		lightIndexBufferCi.usage =
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		lightIndexBufferCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		lightIndexBuffer =
			device->createBuffer(lightIndexBufferCi, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	}
}

void LightManager::addPointLight(const PointLightInfo& info)
{
	std::lock_guard guard{sharedMutex};
	pointLightInfo.emplace_back(info);
	lightIndexes.emplace_back((uint32_t)LightType::POINT, pointLightInfo.size() - 1);
	dirty = true;
}

void LightManager::bindLightInfos(std::shared_ptr<MaterialInstanceVk> mi)
{
	SharedLockGuard guard{sharedMutex};
	updateLightInfos();
	auto descSet = mi->queryDescriptorSet("Light");
	descSet->bindResource(
		0, lightIndexBuffer->getBindingInfo(
			   0, sizeof(uint32_t) + lightIndexes.size() * sizeof(lightIndexes.front())));
	descSet->bindResource(1, pointLightBuffer->getBindingInfo(
								 0, pointLightInfo.size() * sizeof(pointLightInfo.front())));
}

void LightManager::updateLightInfos()
{
	if (!dirty)
		return;

	assert(pointLightInfo.size() < MAX_POINT_LIGHT_COUNT);
	assert(lightIndexes.size() < MAX_LIGHT_INDEX_COUNT);
	pointLightBuffer->setData(0, pointLightInfo.data(),
							  pointLightInfo.size() * sizeof(pointLightInfo.front()));
	const uint32_t size = lightIndexes.size();
	lightIndexBuffer->setData(0, &size, sizeof(uint32_t));
	lightIndexBuffer->setData(sizeof(uint32_t), lightIndexes.data(),
							  lightIndexes.size() * sizeof(lightIndexes.front()));
	dirty = false;
}
