//
// Created by Frank on 2024/1/11.
//
#include "backend/vulkan/FrameGraph.h"
#include "backend/vulkan/VulkanGlobal.h"
#include "backend/vulkan/VulkanRenderPass.h"
#include "gtest/gtest.h"
using namespace xd;
void addNewColorAttach(FrameGraphBuilder::FrameGraphNode& node,
					   const std::string& name,
					   std::unordered_map<std::string, FrameGraphResourceHandle>& resources)
{
	VkAttachmentDescription2 colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
	colorAttachment.pNext = nullptr;
	colorAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout =
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;  // NOTE: we need to fill final layout of this
												   // subpass instead of whole pass, cuz we
												   // don't know the later usage yet
	VkAttachmentReference2 colorAttachmentRef{};
	colorAttachmentRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
	colorAttachmentRef.pNext = nullptr;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachmentRef.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	VkSubpassDependency2 colorAttachExtDep{};
	colorAttachExtDep.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
	colorAttachExtDep.pNext = nullptr;
	colorAttachExtDep.srcSubpass = VK_SUBPASS_EXTERNAL;
	colorAttachExtDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	colorAttachExtDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	colorAttachExtDep.srcAccessMask = 0;
	colorAttachExtDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	colorAttachExtDep.dependencyFlags = 0;
	colorAttachExtDep.viewOffset = 0;
	resources[name] = node.addColorAttach(std::move(colorAttachmentRef), std::move(colorAttachment),
										  std::move(colorAttachExtDep));
}
void addColorAttachToInput(FrameGraphBuilder::FrameGraphNode& node,
						   const FrameGraphResourceHandle& from,
						   const std::string& name,
						   std::unordered_map<std::string, FrameGraphResourceHandle>& resources)
{
	VkAttachmentReference2 inputRef{};
	inputRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
	inputRef.pNext = nullptr;
	inputRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	inputRef.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	VkSubpassDependency2 colorToInputDep;
	colorToInputDep.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
	colorToInputDep.pNext = nullptr;
	colorToInputDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	colorToInputDep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	colorToInputDep.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	colorToInputDep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	colorToInputDep.dependencyFlags = 0;
	colorToInputDep.viewOffset = 0;
	resources[name] = node.addColorAttach(std::move(inputRef), from, std::move(colorToInputDep));
}

TEST(VulkanTestSuite, FrameGraphBuildTest)
{
	VulkanGlobal::init({}, {"VK_LAYER_KHRONOS_validation"}, false, nullptr, 0, 0, {}, {});
	FrameGraphBuilder builder;
	std::unordered_map<std::string, FrameGraphResourceHandle> resources;
	{
		auto& a = builder.addPass("a");
		addNewColorAttach(a, "attach_a0", resources);
		addNewColorAttach(a, "attach_a1", resources);
	}
	{
		auto& b = builder.addPass("b");
		addColorAttachToInput(b, resources["attach_a0"], "input_a0_b", resources);
		addNewColorAttach(b, "attach_b0", resources);
	}
	{
		auto& c = builder.addPass("c");
		addColorAttachToInput(c, resources["attach_a0"], "input_a0_c", resources);
		addNewColorAttach(c, "attach_c0", resources);
	}
	{
		auto& d = builder.addPass("d");
		addColorAttachToInput(d, resources["attach_a1"], "input_a1_d", resources);
		addColorAttachToInput(d, resources["attach_b0"], "input_b0_d", resources);
		addColorAttachToInput(d, resources["attach_c0"], "input_c0_d", resources);
		addNewColorAttach(d, "attach_d0", resources);
	}
	{
		auto& e = builder.addPass("e");
		addColorAttachToInput(e, resources["attach_a0"], "input_a0_e", resources);
		addNewColorAttach(e, "attach_e0", resources);
	}
	const auto frameGraph = builder.build(VulkanGlobal::device);
	const auto& renderPassDesc = frameGraph->renderPass->getDesc();
	{
		const auto& subpasses = renderPassDesc.subpasses;
		const auto& a = subpasses[0];
		const auto& b = subpasses[1];
		const auto& c = subpasses[2];
		const auto& d = subpasses[3];
		const auto& e = subpasses[4];
		EXPECT_EQ(b.preserveAttaches.size(), 1);
		EXPECT_EQ(b.preserveAttaches[0], 1);
		EXPECT_EQ(c.preserveAttaches.size(), 1);
		EXPECT_EQ(c.preserveAttaches[0], 1);
		EXPECT_EQ(e.preserveAttaches.size(), 0);
	}
}