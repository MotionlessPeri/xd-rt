//
// Created by Frank on 2024/1/13.
//

#include "VulkanMemory.h"

#include "VulkanDevice.h"
#include "VulkanGlobal.h"
using namespace xd;
VulkanMemory::~VulkanMemory()
{
	device->freeMemory(memory);
}

void VulkanMemory::map(uint32_t offset, void* ptr, uint32_t size) const
{
	const auto memoryType = device->getMemoryType(desc.memoryTypeIndex);
	if ((memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
		throw std::runtime_error{"Memory can not be mapped directly!\n"};
	// memory can be accessed by host(CPU)
	auto* data = device->mapMemory(memory, offset, size);
	memcpy(data, ptr, size);
	device->unmapMemory(memory);
	if ((memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
		// Explicit flush is required
		VkMappedMemoryRange mappedRange{};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		device->flushMemory({mappedRange});
	}
}

VulkanMemory::VulkanMemory(std::shared_ptr<const VulkanDevice> device,
						   VkMemoryAllocateInfo desc,
						   VkDeviceMemory memory)
	: VulkanDeviceObject(std::move(device), desc), memory(memory)
{
}
