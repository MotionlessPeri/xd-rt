//
// Created by Frank on 2024/1/24.
//

#ifndef XD_RT_SAMPLERCACHE_H
#define XD_RT_SAMPLERCACHE_H
#include <map>
#include <memory>
#include "VulkanDescs.h"
#include "VulkanTypes.h"
namespace xd {

class SamplerCache {
public:
	static void init(std::shared_ptr<VulkanDevice> device);
	static void terminate();
	static SamplerCache& get() { return *singleton; }
	SamplerCache() = delete;
	SamplerCache(const SamplerCache& other) = delete;
	SamplerCache(SamplerCache&& other) noexcept = delete;
	SamplerCache& operator=(const SamplerCache& other) = delete;
	SamplerCache& operator=(SamplerCache&& other) noexcept = delete;
	~SamplerCache() = default;
	std::shared_ptr<VulkanSampler> getOrCreate(const SamplerDesc& desc);

private:
	explicit SamplerCache(std::shared_ptr<VulkanDevice> device);

	std::shared_ptr<VulkanDevice> device;
	std::map<SamplerDesc, std::shared_ptr<VulkanSampler>> caches;
	static SamplerCache* singleton;
};

}  // namespace xd

#endif	// XD_RT_SAMPLERCACHE_H
