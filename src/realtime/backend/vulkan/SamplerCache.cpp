//
// Created by Frank on 2024/1/24.
//

#include "SamplerCache.h"

#include "VulkanDevice.h"
using namespace xd;
SamplerCache* SamplerCache::singleton = nullptr;
void SamplerCache::init(std::shared_ptr<VulkanDevice> device)
{
	singleton = new SamplerCache{std::move(device)};
}
void SamplerCache::terminate()
{
	delete singleton;
}
std::shared_ptr<VulkanSampler> SamplerCache::getOrCreate(const SamplerDesc& desc)
{
	const auto it = caches.find(desc);
	if (it != caches.end())
		return it->second;
	auto ret = device->createSampler(desc);
	caches.insert({desc, ret});
	return ret;
}

SamplerCache::SamplerCache(std::shared_ptr<VulkanDevice> device) : device(std::move(device)) {}
