//
// Created by Frank on 2024/1/9.
//

#include "VulkanQueue.h"
#include <ranges>
#include "VulkanFence.h"
#include "VulkanMacros.h"
#include "VulkanSemaphore.h"
#include "VulkanSwapchain.h"
using namespace xd;
void VulkanQueue::present(const QueuePresentInfoContainer& desc) const
{
	const auto waitSemaphoresView =
		desc.waitSemaphores |
		std::views::transform([](const auto& semaphore) { return semaphore->semaphore; });
	std::vector<VkSemaphore> semaphoreHandles{waitSemaphoresView.begin(), waitSemaphoresView.end()};
	const auto swapchainsView = desc.swapchains | std::views::transform([](const auto& swapchain) {
									return swapchain->swapchain;
								});
	std::vector<VkSwapchainKHR> swapchainHandles{swapchainsView.begin(), swapchainsView.end()};
	VkPresentInfoKHR info;
	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.pNext = nullptr;
	info.waitSemaphoreCount = semaphoreHandles.size();
	info.pWaitSemaphores = semaphoreHandles.data();
	info.swapchainCount = swapchainHandles.size();
	info.pSwapchains = swapchainHandles.data();
	info.pImageIndices = desc.imageIndices.data();
	info.pResults = nullptr;
	try {
		CHECK_VK_ERROR(vkQueuePresentKHR(queue, &info));
	}
	catch (const std::exception& err) {
		std::cout << err.what() << std::endl;
	}
}

VulkanQueue::VulkanQueue(std::shared_ptr<const VulkanDevice> device,
						 const QueueDesc& desc,
						 VkQueue queue)
	: VulkanDeviceObject(std::move(device), desc), queue(queue)
{
}

void VulkanQueue::submitAndWait(const std::vector<VkSubmitInfo>& infos,
								std::shared_ptr<VulkanFence> signalingFence) const
{
	submit(infos, std::move(signalingFence));
	waitIdle();
}

void VulkanQueue::submit(const std::vector<VkSubmitInfo>& infos,
						 std::shared_ptr<VulkanFence> signalingFence) const
{
	CHECK_VK_ERROR(vkQueueSubmit(queue, infos.size(), infos.data(),
								 signalingFence ? signalingFence->fence : VK_NULL_HANDLE));
}

void VulkanQueue::waitIdle() const
{
	CHECK_VK_ERROR(vkQueueWaitIdle(queue));
}
