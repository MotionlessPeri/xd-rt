//
// Created by Frank on 2024/1/17.
//

#ifndef XD_RT_FRAMEGRAPH_H
#define XD_RT_FRAMEGRAPH_H
#include <memory>
#include <vector>

#include "VulkanDescs.h"
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {
// Note: a resource mentioned in a frame graph is a resource that used as an attachment <b> at least
// once </b> in the render pass.

class FrameGraphBuilder {
public:
	inline static constexpr uint32_t INVALID_INDEX = -1;
	struct FrameGraphResourceHandle {
		uint32_t passIndex = INVALID_INDEX;
		uint32_t inputIndex = INVALID_INDEX;
		uint32_t colorIndex = INVALID_INDEX;
		uint32_t depthIndex = INVALID_INDEX;
	};
	struct ResourceTransitionEdge {
		FrameGraphResourceHandle from;
		FrameGraphResourceHandle to;
	};
	struct FrameGraphNode {
		// Note: we must build the frame graph follow the topology order of passes. Error will be
		// occured if not.
		FrameGraphResourceHandle addInput(VkAttachmentReference2&& ref,
										  const FrameGraphResourceHandle& from,
										  VkSubpassDependency2&& dep);
		FrameGraphResourceHandle addInput(VkAttachmentReference2&& ref,
										  VkAttachmentDescription2&& desc,
										  VkSubpassDependency2&& extDep);
		FrameGraphResourceHandle addColorAttach(VkAttachmentReference2&& ref,
												const FrameGraphResourceHandle& from,
												VkSubpassDependency2&& dep);
		FrameGraphResourceHandle addColorAttach(VkAttachmentReference2&& ref,
												VkAttachmentDescription2&& desc,
												VkSubpassDependency2&& extDep);
		FrameGraphResourceHandle addDepthAttach(VkAttachmentReference2&& ref,
												const FrameGraphResourceHandle& from,
												VkSubpassDependency2&& dep);
		FrameGraphResourceHandle addDepthAttach(VkAttachmentReference2&& ref,
												VkAttachmentDescription2&& desc,
												VkSubpassDependency2&& extDep);
		void addNullDepth();
		SubpassDesc getSubpassDescription()
		{
			SubpassDesc subpassDesc;
			subpassDesc.inputAttaches = std::move(inputRefs);
			subpassDesc.colorAttaches = std::move(colorRefs);
			subpassDesc.depthStencilAttaches = std::move(depthRefs);
			subpassDesc.preserveAttaches = std::move(preserveAttaches);
			subpassDesc.desc.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
			subpassDesc.desc.pNext = nullptr;
			subpassDesc.desc.flags = 0;
			subpassDesc.desc.pipelineBindPoint = bindPoint;
			subpassDesc.desc.viewMask = 0;
			subpassDesc.desc.inputAttachmentCount = subpassDesc.inputAttaches.size();
			subpassDesc.desc.pInputAttachments =
				subpassDesc.inputAttaches.empty() ? nullptr : subpassDesc.inputAttaches.data();
			subpassDesc.desc.colorAttachmentCount = subpassDesc.colorAttaches.size();
			subpassDesc.desc.pColorAttachments =
				subpassDesc.colorAttaches.empty() ? nullptr : subpassDesc.colorAttaches.data();
			subpassDesc.desc.pResolveAttachments = nullptr;
			subpassDesc.desc.pDepthStencilAttachment =
				subpassDesc.depthStencilAttaches.empty() ? nullptr
														 : subpassDesc.depthStencilAttaches.data();
			subpassDesc.desc.preserveAttachmentCount = subpassDesc.preserveAttaches.size();
			subpassDesc.desc.pPreserveAttachments = subpassDesc.preserveAttaches.empty()
														? nullptr
														: subpassDesc.preserveAttaches.data();
			return subpassDesc;
		}
		FrameGraphBuilder* owner;
		uint32_t index;
		VkPipelineBindPoint bindPoint;
		std::vector<VkAttachmentReference2> inputRefs;
		std::vector<VkAttachmentReference2> colorRefs;
		std::vector<VkAttachmentReference2> depthRefs;
		std::vector<uint32_t> preserveAttaches;
		std::vector<ResourceTransitionEdge> outs;
		std::vector<ResourceTransitionEdge> ins;

	private:
		void addExistingResource(VkAttachmentReference2& ref,
								 const FrameGraphResourceHandle& from,
								 VkSubpassDependency2& dep,
								 std::vector<VkAttachmentReference2>& emplaceVec) const;
		void addNewResource(VkAttachmentReference2& ref,
							VkAttachmentDescription2& desc,
							VkSubpassDependency2& dep,
							std::vector<VkAttachmentReference2>& emplaceVec) const;
		void recordEdges(const ResourceTransitionEdge& edge);
	};
	FrameGraphNode& addSubpass()
	{
		nodes.emplace_back(this, (uint32_t)nodes.size());
		return nodes.back();
	}

	std::shared_ptr<FrameGraph> build(std::shared_ptr<VulkanDevice> device);

private:
	void trackPreserveAttaches(const ResourceTransitionEdge& edge);
	std::vector<FrameGraphNode> nodes;
	std::vector<VkAttachmentDescription2> attachments;
	std::vector<VkSubpassDependency2> dependencies;
};

class FrameGraphPass {
public:
	std::shared_ptr<VulkanSubpass> subpass = nullptr;
	std::shared_ptr<VulkanGraphicsPipeline> pipeline = nullptr;
};

class FrameGraph {
public:
	std::shared_ptr<VulkanRenderPass> renderPass;
	std::vector<FrameGraphPass> subpasses;
};

}  // namespace xd

#endif	// XD_RT_FRAMEGRAPH_H
