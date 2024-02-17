//
// Created by Frank on 2024/1/17.
//

#ifndef XD_RT_FRAMEGRAPH_H
#define XD_RT_FRAMEGRAPH_H
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "VulkanDescs.h"
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {
// Note: a resource mentioned in a frame graph is a resource that used as an attachment <b> at least
// once </b> in the render pass.

struct FrameGraphResourceHandle {
	inline static constexpr uint32_t INVALID_INDEX = -1;
	uint32_t passIndex = INVALID_INDEX;
	uint32_t inputIndex = INVALID_INDEX;
	uint32_t colorIndex = INVALID_INDEX;
	uint32_t depthIndex = INVALID_INDEX;
};
enum class FGPassType { Graphic = 0, Compute, RayTracing };
class FGResource {};
class FGImage;
class FGBuffer;
class FGImageBinding;
class FGBufferBinding;

class FGImage {
public:
	std::shared_ptr<VulkanImage> create() const;

private:
	std::vector<FGImageBinding*> bindings;
};
class FGBuffer {
public:
	std::shared_ptr<VulkanBuffer> create() const;

private:
	std::vector<FGBufferBinding*> bindings;
};
// Note: we can not deduce whether a resource is written or read(or both) in a binding: For example,
// a storage buffer(UAV access) is allowed to be written and read simultaneously in a
// pipeline(though its weird). Additional info is required to represent read or write
class FGResourceBinding {};
class FGColorAttachment : public FGResourceBinding {
public:
private:
};
class FGDepthAttachment : public FGResourceBinding {};
class FGInputAttachment : public FGResourceBinding {};
class FGResolveAttachment : public FGResourceBinding {};
class FrameGraphBuilder {
public:
	struct ResourceTransitionEdge {
		FrameGraphResourceHandle from;
		FrameGraphResourceHandle to;
	};
	struct FrameGraphNode : SubpassDescBase {
		// Note: we must build the frame graph follow the topology order of passes. Error will be
		// occured if not.
		// TODO: we can merge dependencies in one subpass to one single dependency to save space
		FrameGraphResourceHandle addInput(VkAttachmentReference2&& ref,
										  const FrameGraphResourceHandle& from,
										  VkSubpassDependency2&& dep);
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
		SubpassDesc getSubpassDescription();
		FrameGraphBuilder* owner;
		uint32_t index;
		std::string name;
		VkPipelineBindPoint bindPoint;
		std::unordered_set<uint32_t> firstDeclaredAttaches;
		std::vector<ResourceTransitionEdge> outs;
		std::vector<ResourceTransitionEdge> ins;

	private:
		void addExistingResource(VkAttachmentReference2& ref,
								 const FrameGraphResourceHandle& from,
								 VkSubpassDependency2& dep,
								 std::vector<VkAttachmentReference2>& emplaceVec);
		void addNewResource(VkAttachmentReference2& ref,
							VkAttachmentDescription2& desc,
							VkSubpassDependency2& dep,
							std::vector<VkAttachmentReference2>& emplaceVec);
		void recordEdges(const ResourceTransitionEdge& edge);
	};
	FrameGraphNode& addPass(const std::string& name);

	std::shared_ptr<FrameGraph> build(std::shared_ptr<VulkanDevice> device);

private:
	void trackPreserveAttaches(const ResourceTransitionEdge& edge);
	std::vector<FrameGraphNode> nodes;
	std::vector<VkAttachmentDescription2> attachments;
	std::vector<VkSubpassDependency2> dependencies;
};

class FrameGraphPass {
public:
	FrameGraphPass() = default;
	FrameGraphPass(std::shared_ptr<FrameGraph> owner, std::shared_ptr<VulkanSubpass> subpass)
		: owner(std::move(owner)), subpass(std::move(subpass))
	{
	}
	void bindAttachmentBuffer(FrameGraphResourceHandle pos,
							  std::shared_ptr<VulkanImageView> imageView);
	std::shared_ptr<FrameGraph> owner = nullptr;
	std::shared_ptr<VulkanSubpass> subpass = nullptr;
	std::shared_ptr<VulkanGraphicsPipeline> pipeline = nullptr;
	std::unordered_set<uint32_t> firstDeclaredAttaches;
};

class FrameGraph {
public:
	void bindAttachmentBuffer(FrameGraphResourceHandle pos,
							  std::shared_ptr<VulkanImageView> imageView);
	void buildFrameBuffer(uint32_t width, uint32_t height);
	std::shared_ptr<VulkanRenderPass> renderPass = nullptr;
	std::vector<FrameGraphPass> subpasses;
	std::unordered_map<std::string, FrameGraphPass*> subpassDict;
	std::vector<std::shared_ptr<VulkanImageView>> boundBuffers;
	std::shared_ptr<VulkanFrameBuffer> frameBuffer;
};

}  // namespace xd

#endif	// XD_RT_FRAMEGRAPH_H
