//
// Created by Frank on 2024/1/17.
//

#include "VulkanFrameBuffer.h"

#include "VulkanDevice.h"
using namespace xd;
VulkanFrameBuffer::~VulkanFrameBuffer()
{
	device->destroyFrameBuffer(frameBuffer);
}

VulkanFrameBuffer::VulkanFrameBuffer(std::shared_ptr<const VulkanDevice> device,
									 const VkFramebufferCreateInfo& desc,
									 VkFramebuffer frame_buffer)
	: VulkanDeviceObject(std::move(device), desc), frameBuffer(frame_buffer)
{
}
