//
// Created by Frank on 2024/1/17.
//

#ifndef XD_RT_FRAMEGRAPH_H
#define XD_RT_FRAMEGRAPH_H
#include <cassert>
#include <memory>
#include <optional>
#include <queue>
#include <ranges>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include "VulkanDescs.h"
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {

// Note: a resource mentioned in a frame graph is a resource that used as an attachment <b> at least
// once </b> in the render pass.
inline static constexpr int INVALID_INDEX = -1;
enum class PassType { GRAPHICS = 0, COMPUTE = 1, RAY_TRACING = 2, OTHER = 3 };
inline VkPipelineBindPoint passTypeToPipelineBindPoint(PassType type)
{
	switch (type) {
		case PassType::GRAPHICS:
			return VK_PIPELINE_BIND_POINT_GRAPHICS;
		case PassType::COMPUTE:
			return VK_PIPELINE_BIND_POINT_COMPUTE;
		case PassType::RAY_TRACING:
			return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
		default:
			return VK_PIPELINE_BIND_POINT_MAX_ENUM;
	}
}
class FGBuilder;
class FGNode;
class FGResource;
class FGImage;
class FGBuffer;
class FGResourceBinding;
class FGImageBinding;
class FGBufferBinding;
class FGAttachment;
class FGColorAttachment;
class FGDepthAttachment;
class FGResolveAttachment;
class FGInputAttachment;
class FGPass;
class FGSubpass;
class FGResourceList;
class FrameGraph;

class FGResource {
public:
	friend class FGPass;
	friend class FGSubpass;
	friend class FGResourceList;
	FGResource(FGBuilder* builder, bool imported);

	virtual ~FGResource() = default;
	const FGImage* toFGImage() const;
	const FGBuffer* toFGBuffer() const;

private:
	FGBuilder* builder;
	bool imported;
};

class FGResourceBindingCreationHelper {
public:
	static FGColorAttachment* createColorAttachment(const std::string& name,
													FGBuilder* builder,
													int ownerIndex,
													int nodeIndex,
													FGResourceBinding* prev,
													VkAttachmentLoadOp loadOp,
													VkAttachmentStoreOp storeOp,
													VkClearColorValue clearColor);

	static FGDepthAttachment* createDepthAttachment(const std::string& name,
													VkImageLayout layout,
													FGBuilder* builder,
													int ownerIndex,
													int nodeIndex,
													bool isWrite,
													VkPipelineStageFlags stageMask,
													VkAccessFlags accessMask,
													VkImageAspectFlags aspectMask,
													FGResourceBinding* prev,
													VkAttachmentLoadOp loadOp,
													VkAttachmentStoreOp storeOp,
													VkAttachmentLoadOp stencilLoadOp,
													VkAttachmentStoreOp stencilStoreOp,
													VkClearDepthStencilValue clearValue);

	static FGBufferBinding* createBufferBinding(const std::string& name,
												FGBuilder* builder,
												int ownerIndex,
												int nodeIndex,
												bool isWrite,
												VkPipelineStageFlags stageMask,
												VkAccessFlags accessMask,
												FGResourceBinding* prev);

	static FGImageBinding* createImageBinding(const std::string& name,
											  FGBuilder* builder,
											  int ownerIndex,
											  int nodeIndex,
											  bool isWrite,
											  VkPipelineStageFlags stageMask,
											  VkAccessFlags accessMask,
											  VkImageAspectFlags aspectMask,
											  VkImageLayout layout,
											  FGResourceBinding* prev);
};
class FGResourceBinding {
public:
	friend class FGBuilder;
	friend class FGImage;
	friend class FGNode;
	friend class FGPass;
	friend class FGSubpass;
	friend class FGResourceList;
	FGResourceBinding(FGBuilder* builder,
					  int ownerIndex,
					  int nodeIndex,
					  bool write,
					  FGResourceBinding* prev,
					  VkPipelineStageFlags stageMask,
					  VkAccessFlags accessMask);

	FGResourceBinding(const FGResourceBinding& other) = delete;
	FGResourceBinding(FGResourceBinding&& other) noexcept = delete;
	FGResourceBinding& operator=(const FGResourceBinding& other) = delete;
	FGResourceBinding& operator=(FGResourceBinding&& other) noexcept = delete;
	virtual ~FGResourceBinding() = default;
	const FGImageBinding* toImageBinding() const;
	const FGBufferBinding* toBufferBinding() const;

protected:
	FGResourceBinding* getPrev()
	{
		if (write)
			return this;
		else
			return this->prev;
	}
	void recordNewBinding(bool isWrite, FGResourceBinding* rb);
	FGBuilder* builder;
	int ownerIndex;
	int nodeIndex;
	bool write = false;
	FGResourceBinding* prev = nullptr;	// must be a write
	FGResourceBinding* next = nullptr;	// must be a write
	VkPipelineStageFlags stageMask;
	VkAccessFlags accessMask;
	std::vector<FGResourceBinding*> reads;
};
class FGBufferBinding : public FGResourceBinding {
public:
	friend class FGPass;
	friend class FGSubpass;
	FGBufferBinding(FGBuilder* builder,
					int ownerIndex,
					int nodeIndex,
					bool write,
					FGResourceBinding* prev,
					VkPipelineStageFlags stageMask,
					VkAccessFlags accessMask);

	FGBufferBinding* createBufferBinding(const std::string& name,
										 int nodeIndex,
										 VkPipelineStageFlags stageMask,
										 VkAccessFlags accessMask);
};
class FGImageBinding : public FGResourceBinding {
public:
	friend class FGBuilder;
	friend class FGPass;
	friend class FGSubpass;
	friend class FGResourceList;
	FGImageBinding(FGBuilder* builder,
				   int ownerIndex,
				   int nodeIndex,
				   bool write,
				   FGResourceBinding* prev,
				   VkPipelineStageFlags stageMask,
				   VkAccessFlags accessMask,
				   VkImageAspectFlags aspectMask,
				   VkImageLayout layout);

	FGColorAttachment* createColorAttach(const std::string& name,
										 FGNode& node,
										 VkAttachmentLoadOp loadOp,
										 VkAttachmentStoreOp storeOp,
										 VkClearColorValue clearValue = {0, 0, 0, 0});
	FGDepthAttachment* createDepthAttach(const std::string& name,
										 FGNode& node,
										 VkImageLayout layout,
										 VkPipelineStageFlags stageMask,
										 VkAccessFlags accessMask,
										 VkImageAspectFlags aspectMask,
										 VkAttachmentLoadOp loadOp,
										 VkAttachmentStoreOp storeOp,
										 VkAttachmentLoadOp stencilLoadOp,
										 VkAttachmentStoreOp stencilStoreOp,
										 VkClearDepthStencilValue clearValue = {1.f, 0u});
	FGInputAttachment* createInputAttach(const std::string& name,
										 FGNode& node,
										 VkImageLayout layout,
										 VkImageAspectFlags aspectMask,
										 VkPipelineStageFlags stageMask,
										 VkAccessFlags accessMask);
	FGImageBinding* createImageBinding(const std::string& name,
									   FGNode& node,
									   VkImageLayout layout,
									   VkImageAspectFlags aspectMask,
									   VkPipelineStageFlags stageMask,
									   VkAccessFlags accessMask);
	virtual bool isAttachment() const { return false; }
	const FGAttachment* toAttachment() const;

protected:
	VkImageAspectFlags aspectMask;
	VkImageLayout layout;
};
class FGDummyImageBinding {
public:
	explicit FGDummyImageBinding(FGBuilder* builder, int ownerIndex);

	// Note: only write binding can be created in FGDummyBinding
	FGColorAttachment* createColorAttach(const std::string& name,
										 FGNode& node,
										 VkAttachmentLoadOp loadOp,
										 VkAttachmentStoreOp storeOp,
										 VkClearColorValue clearValue = {0, 0, 0, 0}) const;
	FGDepthAttachment* createDepthAttach(const std::string& name,
										 FGNode& node,
										 VkPipelineStageFlags stageMask,
										 VkAccessFlags accessMask,
										 VkImageAspectFlags aspectMask,
										 VkAttachmentLoadOp loadOp,
										 VkAttachmentStoreOp storeOp,
										 VkAttachmentLoadOp stencilLoadOp,
										 VkAttachmentStoreOp stencilStoreOp,
										 VkClearDepthStencilValue clearValue) const;
	FGImageBinding* createImageBinding(const std::string& name,
									   FGNode& node,
									   VkImageLayout layout,
									   VkImageAspectFlags aspectMask,
									   VkPipelineStageFlags stageMask,
									   VkAccessFlags accessMask) const;

private:
	void setFirstBinding(FGImageBinding* binding) const;
	FGBuilder* builder;
	int ownerIndex;
};

class FGAttachment : public FGImageBinding {
public:
	friend class FGPass;
	friend class FGSubpass;
	FGAttachment(FGBuilder* builder,
				 int ownerIndex,
				 int nodeIndex,
				 bool write,
				 FGResourceBinding* prev,
				 VkPipelineStageFlags stageMask,
				 VkAccessFlags accessMask,
				 VkImageAspectFlags aspectMask,
				 VkImageLayout layout,
				 VkAttachmentLoadOp loadOp,
				 VkAttachmentStoreOp storeOp)
		: FGImageBinding(builder,
						 ownerIndex,
						 nodeIndex,
						 write,
						 prev,
						 stageMask,
						 accessMask,
						 aspectMask,
						 layout),
		  loadOp(loadOp),
		  storeOp(storeOp)
	{
	}

	bool isAttachment() const final { return true; }
	VkAttachmentLoadOp getLoadOp() const { return loadOp; }
	VkAttachmentStoreOp getStoreOp() const { return storeOp; }
	virtual VkAttachmentLoadOp getStencilLoadOp() const { return VK_ATTACHMENT_LOAD_OP_DONT_CARE; }
	virtual VkAttachmentStoreOp getStencilStoreOp() const
	{
		return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}
	const FGColorAttachment* toColorAttach() const;
	const FGDepthAttachment* toDepthAttach() const;
	const FGInputAttachment* toInputAttach() const;
	const FGResolveAttachment* toResolveAttach() const;
	VkAttachmentReference2 getAttachmentRef(uint32_t attachIndex) const
	{
		VkAttachmentReference2 ret;
		ret.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
		ret.pNext = nullptr;
		ret.attachment = attachIndex;
		ret.layout = layout;
		ret.aspectMask = aspectMask;
		return ret;
	}

protected:
	VkAttachmentLoadOp loadOp;
	VkAttachmentStoreOp storeOp;
};
class FGColorAttachment : public FGAttachment {
public:
	friend class FGNode;
	friend class FGPass;
	friend class FGSubpass;
	FGColorAttachment(FGBuilder* builder,
					  int ownerIndex,
					  int nodeIndex,
					  bool write,
					  FGResourceBinding* prev,
					  VkPipelineStageFlags stageMask,
					  VkAccessFlags accessMask,
					  VkImageAspectFlags aspectMask,
					  VkImageLayout layout,
					  VkAttachmentLoadOp loadOp,
					  VkAttachmentStoreOp storeOp,
					  VkClearColorValue clearColor)
		: FGAttachment(builder,
					   ownerIndex,
					   nodeIndex,
					   write,
					   prev,
					   stageMask,
					   accessMask,
					   aspectMask,
					   layout,
					   loadOp,
					   storeOp),
		  clearColor(clearColor)
	{
	}
	VkClearColorValue clearColor;
};
class FGDepthAttachment : public FGAttachment {
public:
	friend class FGNode;
	friend class FGPass;
	friend class FGSubpass;
	FGDepthAttachment(FGBuilder* builder,
					  int ownerIndex,
					  int nodeIndex,
					  bool write,
					  FGResourceBinding* prev,
					  VkPipelineStageFlags stageMask,
					  VkAccessFlags accessMask,
					  VkImageAspectFlags aspectMask,
					  VkImageLayout layout,
					  VkAttachmentLoadOp loadOp,
					  VkAttachmentStoreOp storeOp,
					  VkAttachmentLoadOp stencilLoadOp,
					  VkAttachmentStoreOp stencilStoreOp,
					  VkClearDepthStencilValue depthStencilValue)
		: FGAttachment(builder,
					   ownerIndex,
					   nodeIndex,
					   write,
					   prev,
					   stageMask,
					   accessMask,
					   aspectMask,
					   layout,
					   loadOp,
					   storeOp),
		  stencilLoadOp(stencilLoadOp),
		  stencilStoreOp(stencilStoreOp),
		  depthStencilClearValue(depthStencilValue)
	{
	}

	VkAttachmentLoadOp getStencilLoadOp() const override { return stencilLoadOp; }
	VkAttachmentStoreOp getStencilStoreOp() const override { return stencilStoreOp; }

	VkAttachmentLoadOp stencilLoadOp;
	VkAttachmentStoreOp stencilStoreOp;
	VkClearDepthStencilValue depthStencilClearValue;
};
class FGInputAttachment : public FGAttachment {
public:
	friend class FGNode;
	friend class FGPass;
	friend class FGSubpass;
	FGInputAttachment(FGBuilder* builder,
					  int ownerIndex,
					  int nodeIndex,
					  bool write,
					  FGResourceBinding* prev,
					  VkPipelineStageFlags stageMask,
					  VkAccessFlags accessMask,
					  VkImageAspectFlags aspectMask,
					  VkImageLayout layout)
		: FGAttachment(builder,
					   ownerIndex,
					   nodeIndex,
					   write,
					   prev,
					   stageMask,
					   accessMask,
					   aspectMask,
					   layout,
					   VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					   VK_ATTACHMENT_STORE_OP_DONT_CARE)
	{
	}
};
class FGResolveAttachment : public FGAttachment {
public:
	friend class FGNode;
	friend class FGPass;
	friend class FGSubpass;
	FGResolveAttachment(FGBuilder* builder,
						int ownerIndex,
						int nodeIndex,
						bool write,
						FGResourceBinding* prev,
						VkPipelineStageFlags stageMask,
						VkAccessFlags accessMask,
						VkImageAspectFlags aspectMask,
						VkImageLayout layout,
						VkAttachmentLoadOp loadOp,
						VkAttachmentStoreOp storeOp)
		: FGAttachment(builder,
					   ownerIndex,
					   nodeIndex,
					   write,
					   prev,
					   stageMask,
					   accessMask,
					   aspectMask,
					   layout,
					   loadOp,
					   storeOp)
	{
	}
};

class FGBuffer : public FGResource {
public:
	friend class FGPass;
	friend class FGSubpass;
	friend class FGResourceList;
	FGBuffer(FGBuilder* builder,
			 bool imported,
			 VkBufferCreateInfo buffer_ci,
			 VkBufferViewCreateInfo buffer_view_ci);

private:
	VkBufferCreateInfo bufferCi;
	VkBufferViewCreateInfo bufferViewCi;
};
class FGImage : public FGResource {
public:
	friend class FGBuilder;
	friend class FGPass;
	friend class FGSubpass;
	friend class FGResourceList;
	friend class FGDummyImageBinding;
	// For creating new image
	FGImage(FGBuilder* builder,
			VkImageCreateInfo imageCi,
			VkImageViewCreateInfo imageViewCi,
			std::shared_ptr<VulkanSampler> sampler,
			bool imported);
	FGImage(const FGImage& other) = default;
	FGImage(FGImage&& other) noexcept = default;
	FGImage& operator=(const FGImage& other) = default;
	FGImage& operator=(FGImage&& other) noexcept = default;
	~FGImage() override = default;

private:
	VkImageCreateInfo imageCi;
	VkImageViewCreateInfo imageViewCi;
	std::shared_ptr<VulkanSampler> sampler;
	FGImageBinding* firstBinding = nullptr;
};

struct FGSyncObject {
	std::shared_ptr<VulkanFence> fence;
	std::shared_ptr<VulkanSemaphore> executeComplete;
};

class FGPassExecutorBase {
public:
	FGPassExecutorBase() = default;
	FGPassExecutorBase(const FGPassExecutorBase& other) = delete;
	FGPassExecutorBase(FGPassExecutorBase&& other) noexcept = delete;
	FGPassExecutorBase& operator=(const FGPassExecutorBase& other) = delete;
	FGPassExecutorBase& operator=(FGPassExecutorBase&& other) noexcept = delete;
	virtual ~FGPassExecutorBase() = default;
	virtual void execute(
		FGResourceList& resources,
		std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
		const std::vector<std::shared_ptr<VulkanSemaphore>>& prevPassSemaphores) = 0;
};

template <typename TLambda>
class FGPassExecutor : public FGPassExecutorBase {
public:
	explicit FGPassExecutor(TLambda lambda) : lambda(std::move(lambda)) {}
	void execute(FGResourceList& resources,
				 std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
				 const std::vector<std::shared_ptr<VulkanSemaphore>>& prevPassSemaphores) override
	{
		lambda(resources, cmdBuffer, prevPassSemaphores);
	}

private:
	TLambda lambda;
};
class FGNode {
public:
	friend class FGImage;
	friend class FGResourceBinding;
	friend class FGBufferBinding;
	friend class FGImageBinding;
	friend class FGDummyImageBinding;
	friend class FGColorAttachment;
	friend class FGDepthAttachment;
	friend class FGInputAttachment;
	friend class FGResolveAttachment;
	friend class FGResourceBindingCreationHelper;
	friend class FGBuilder;

	FGNode() = delete;

	FGNode(FGBuilder* owner,
		   std::string name,
		   PassType type,
		   int index,
		   int passIndex,
		   std::shared_ptr<FGPassExecutorBase> executor,
		   bool externalSync);
	FGNode(const FGNode& other) = default;
	FGNode(FGNode&& other) noexcept = default;
	FGNode& operator=(const FGNode& other) = default;
	FGNode& operator=(FGNode&& other) noexcept = default;
	~FGNode();

	FGDummyImageBinding createImage(VkImageCreateInfo imageCi,
									VkImageViewCreateInfo imageViewCi,
									std::shared_ptr<VulkanSampler> sampler);
	FGDummyImageBinding importImage(VkImageCreateInfo imageCi,
									VkImageViewCreateInfo imageViewCi,
									std::shared_ptr<VulkanSampler> sampler);

private:
	void addImageBinding(const std::string& name, FGImageBinding* ib);
	void addBufferBinding(const std::string& name, FGBufferBinding* bb);
	void addAttachment(const std::string& name, FGAttachment* attach);
	void addColorAttach(const std::string& name, FGColorAttachment* attach);
	void addDepthAttach(const std::string& name, FGDepthAttachment* attach);
	void addInputAttach(const std::string& name, FGInputAttachment* attach);
	void addResolveAttach(const std::string& name, FGResolveAttachment* attach);
	bool isFirstNode() const;
	int getPassIndex() const;
	FGBuilder* owner = nullptr;
	std::string name;
	PassType type;
	int index;
	int passIndex;	// the relevant FGRenderPassNode index. If type is not GRAPHIC, this field is -1
	std::shared_ptr<FGPassExecutorBase> executor;
	bool externalSync = false;
	std::unordered_map<std::string, FGImageBinding*> nameToImageBindings;
	std::unordered_map<std::string, FGBufferBinding*> nameToBufferBindings;
	std::vector<FGAttachment*> attachments;
};
class FGGraphicsPassNode {
public:
	FGGraphicsPassNode(FGBuilder* owner, std::string name, int index);
	template <typename TLambda>
	FGNode& addSubpass(const std::string& name, TLambda&& lambda, bool externalSync = false);

private:
	FGBuilder* owner = nullptr;
	std::string name;
	int index;
	std::unordered_set<int> nodeIndexes;
};

// TODO: non-memory pass dependency and execution barrier
// like present pass needs to be in the last
class FGBuilder {
public:
	friend class FGNode;
	friend class FGImage;
	friend class FGResourceBindingCreationHelper;
	friend class FGResource;
	friend class FGImage;
	friend class FGBuffer;
	friend class FGResourceBinding;
	friend class FGImageBinding;
	friend class FGBufferBinding;
	friend class FGDummyImageBinding;
	friend class FGAttachment;
	friend class FGColorAttachment;
	friend class FGDepthAttachment;
	friend class FGResolveAttachment;
	friend class FGInputAttachment;
	friend class FGGraphicsPassNode;
	struct NodeHelper {
		int index;
		std::unordered_set<int> ins;
		std::unordered_set<int> outs;
	};
	struct FGPassRef : public NodeHelper {
		PassType type;
		std::unordered_set<int> subpassRefIndexes;
		std::unordered_map<int, int> imageIndexToAttachIndex;
		std::unordered_map<int, int> subpassDescIndexToSubpassRefIndex;
	};
	struct FGSubpassRef : public NodeHelper {
		int passRefIndex = INVALID_INDEX;
		int nodeIndex;
	};
	struct BindingIndex {
		int passIndex = INVALID_INDEX;
		int subpassIndex = INVALID_INDEX;

		friend bool operator<(const BindingIndex& lhs, const BindingIndex& rhs)
		{
			return std::tie(lhs.passIndex, lhs.subpassIndex) <
				   std::tie(rhs.passIndex, rhs.subpassIndex);
		}

		friend bool operator<=(const BindingIndex& lhs, const BindingIndex& rhs)
		{
			return !(rhs < lhs);
		}

		friend bool operator>(const BindingIndex& lhs, const BindingIndex& rhs)
		{
			return rhs < lhs;
		}

		friend bool operator>=(const BindingIndex& lhs, const BindingIndex& rhs)
		{
			return !(lhs < rhs);
		}
	};

	explicit FGBuilder(std::shared_ptr<VulkanDevice> device);

	FGGraphicsPassNode& addGraphicsPass(const std::string& name);

	template <typename TLambda>
	FGNode& addPass(const std::string& name,
					PassType type,
					TLambda&& lambda,
					bool externalSync = false)
	{
		return nodes.emplace_back(
			this, name, type, nodes.size(), -1,
			std::make_shared<FGPassExecutor<TLambda>>(std::forward<TLambda>(lambda)), externalSync);
	}
	template <typename TLambda>
	FGNode& addComputePass(const std::string& name, TLambda&& lambda, bool externalSync = false)
	{
		return addPass(name, PassType::COMPUTE, std::forward<TLambda>(lambda), externalSync);
	}
	std::shared_ptr<FrameGraph> build(std::shared_ptr<VulkanDevice> device);
	~FGBuilder() = default;

	FGDummyImageBinding createImage(VkImageCreateInfo imageCi,
									VkImageViewCreateInfo imageViewCi,
									std::shared_ptr<VulkanSampler> sampler);
	FGDummyImageBinding importImage(VkImageCreateInfo imageCi,
									VkImageViewCreateInfo imageViewCi,
									std::shared_ptr<VulkanSampler> sampler);

private:
	struct BuildSubpassRefRetType {
		std::vector<FGSubpassRef> subpassRefs;
		std::unordered_map<int, int> nodeIndexToSubpassRefIndex;
		std::vector<FGBuilder::FGPassRef> passRefs;
		std::unordered_map<int, int> graphicsNodeIndexToPassRefIndex;
	};
	class NodeToRefConverter {
	public:
		BuildSubpassRefRetType buildSubpassAndPassRefs() const;

	private:
		const std::vector<FGNode>& nodes;
	};
	BuildSubpassRefRetType buildSubpassAndPassRefs() const;
	template <typename NodeHelperType>
		requires std::derived_from<NodeHelperType, NodeHelper>
	static void topologicalSort(std::vector<NodeHelperType>& refs)
	{
		const int refCount = refs.size();
		std::queue<int> refIndexQueue;
		std::vector<int> inDegrees(refCount);
		for (const auto i : std::views::iota(0, refCount)) {
			const auto& ref = refs[i];
			inDegrees[i] = ref.ins.size();
			if (inDegrees[i] == 0)
				refIndexQueue.emplace(i);
		}
		int newIndex = 0;
		std::unordered_map<int, int> oldToNewIndex;
		while (!refIndexQueue.empty()) {
			const auto refIndex = refIndexQueue.front();
			refIndexQueue.pop();
			auto& ref = refs[refIndex];
			oldToNewIndex[ref.index] = newIndex;
			ref.index = newIndex;
			++newIndex;
			for (const auto outRefIndex : ref.outs) {
				auto& inDegree = inDegrees[outRefIndex];
				--inDegree;
				if (inDegree == 0)
					refIndexQueue.emplace(outRefIndex);
			}
		}
		std::ranges::sort(refs, [](const NodeHelperType& lhs, const NodeHelperType& rhs) {
			return lhs.index < rhs.index;
		});
		// change node index in node.ins and node.outs to new index
		for (auto& node : refs) {
			const auto inView = node.ins | std::views::transform([&](const int index) {
									return oldToNewIndex.at(index);
								});
			node.ins = {inView.begin(), inView.end()};
			const auto outView = node.outs | std::views::transform([&](const int index) {
									 return oldToNewIndex.at(index);
								 });
			node.outs = {outView.begin(), outView.end()};
		}
	}
	std::shared_ptr<VulkanDevice> device;
	std::vector<FGNode> nodes;
	std::vector<FGGraphicsPassNode> graphicNodes;
	std::vector<FGImage> images;
	std::vector<FGBuffer> buffers;
};

// TODO: do we need to inherit FGGraphicPass/FGComputePass/... from FGPass or we just mix them
// together in one FGPass
// Note: There're 2 main purpose of FGPass: execute rendering and binding resouces
class FGPass {
public:
	friend class FrameGraph;
	friend class FGBuilder;
	friend class FGResourceList;
	friend class FGSubpass;
	FGPass() = default;
	FGPass(const FGPass& other) = default;
	FGPass(FGPass&& other) noexcept = default;
	FGPass& operator=(const FGPass& other) = default;
	FGPass& operator=(FGPass&& other) noexcept = default;
	void execute(std::shared_ptr<VulkanDevice> device,
				 FGResourceList& resources,
				 std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
				 const std::vector<std::shared_ptr<VulkanSemaphore>>& prevPassSemaphores) const;
	std::shared_ptr<VulkanRenderPass> getRenderPass() const { return renderPass; }
	bool isExternalSync() const;

private:
	void executeGraphicPass(
		std::shared_ptr<VulkanDevice> device,
		FGResourceList& resources,
		std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
		const std::vector<std::shared_ptr<VulkanSemaphore>>& prevPassSemaphores) const;
	void executeComputePass(
		std::shared_ptr<VulkanDevice> device,
		FGResourceList& resources,
		std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
		const std::vector<std::shared_ptr<VulkanSemaphore>>& prevPassSemaphores) const;
	void executeRayTracingPass(
		std::shared_ptr<VulkanDevice> device,
		FGResourceList& resources,
		std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
		const std::vector<std::shared_ptr<VulkanSemaphore>>& prevPassSemaphores) const;
	void executeOtherPass(
		std::shared_ptr<VulkanDevice> device,
		FGResourceList& resources,
		std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
		const std::vector<std::shared_ptr<VulkanSemaphore>>& prevPassSemaphores) const;
	std::shared_ptr<VulkanFrameBuffer> buildFrameBuffer(std::shared_ptr<VulkanDevice> device,
														const FGResourceList& resources) const;
	std::vector<VkClearValue> buildClearValues() const;
	FrameGraph* fg;
	PassType type;
	int index;
	std::vector<int> subpassIndexes;
	std::unordered_map<int, int> imageIndexToAttachIndex;
	std::shared_ptr<VulkanRenderPass> renderPass = nullptr;
};

class FGSubpass {
public:
	friend class FrameGraph;
	friend class FGPass;
	friend class FGBuilder;
	friend class FGResourceList;
	std::shared_ptr<VulkanSubpass> getSubpass() const { return subpass; }
	std::shared_ptr<VulkanRenderPass> getRenderPass() const;
	void execute(std::shared_ptr<VulkanDevice> device,
				 FGResourceList& resources,
				 std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
				 const std::vector<std::shared_ptr<VulkanSemaphore>>& prevPassSemaphores) const;

private:
	void transitResources(const FGResourceList& resources,
						  std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const;
	FrameGraph* fg;
	int passIndex;
	std::shared_ptr<VulkanSubpass> subpass = nullptr;
	std::shared_ptr<FGPassExecutorBase> executor = nullptr;
	bool externalSync;
	std::unordered_map<std::string, FGImageBinding*> nameToImageBindings;
	std::unordered_map<std::string, FGBufferBinding*> nameToBufferBindings;
};

class FGResourceList {
public:
	// TODO: design a more convenient way for passes to store its own data whose life cycle beyond
	// the pass lambda
	friend class FGPass;
	friend class FGSubpass;
	friend class FrameGraph;
	FGResourceList(const FGResourceList& other) = delete;
	FGResourceList(FGResourceList&& other) noexcept = delete;
	FGResourceList& operator=(const FGResourceList& other) = delete;
	FGResourceList& operator=(FGResourceList&& other) noexcept = delete;
	~FGResourceList() = default;

	void bindImage(const std::string& subpassName,
				   const std::string& bindingName,
				   std::shared_ptr<TextureVk> image,
				   VkPipelineStageFlags stageMask,
				   VkImageLayout currentLayout,
				   VkAccessFlags accessMask);
	void bindBuffer(const std::string& subpassName,
					const std::string& bindingName,
					std::shared_ptr<VulkanBuffer> buffer,
					VkPipelineStageFlags stageMask,
					VkAccessFlags accessMask);
	std::shared_ptr<TextureVk> getImage(const std::string& subpassName,
										const std::string& bindingName) const;
	std::shared_ptr<VulkanBuffer> getBuffer(const std::string& subpassName,
											const std::string& bindingName) const;
	bool hasValue(const std::string& name) const { return passStorage.contains(name); }
	template <typename T>
	T getValue(const std::string& name) const
	{
		return std::get<T>(passStorage.at(name));
	}
	template <typename T>
	void storeValue(const std::string& name, T value)
	{
		passStorage[name] = std::forward<T>(value);
	}
	void addExternalWaitingSemaphore(const std::string& passName,
									 std::shared_ptr<VulkanSemaphore> semaphore,
									 VkPipelineStageFlags waitingStage);
	void addExternalSignalingSemaphore(const std::string& passName,
									   std::shared_ptr<VulkanSemaphore> semaphore);

private:
	FGResourceList(std::shared_ptr<VulkanDevice> device,
				   const FrameGraph* fg,
				   int imageCount,
				   int bufferCount,
				   std::shared_ptr<VulkanCommandPool> graphicPool,
				   std::shared_ptr<VulkanCommandPool> computePool,
				   std::shared_ptr<VulkanCommandPool> rayTracingPool);
	void prepareResources();
	void preparePerPassResources();
	std::shared_ptr<VulkanCommandBuffer> createCmdBuffer(PassType type) const;
	std::shared_ptr<VulkanDevice> device;
	const FrameGraph* fg;
	std::vector<std::shared_ptr<TextureVk>> images;	 // one-one to fg->images
	struct ImageState {
		VkPipelineStageFlags stageMask;
		VkImageLayout layout;
		VkAccessFlags accessMask;
	};
	struct BufferState {
		VkPipelineStageFlags stageMask;
		VkAccessFlags accessMask;
	};
	std::unordered_map<int, ImageState> imageIndexToImportState;	// for all imported images
	std::unordered_map<int, BufferState> bufferIndexToImportState;	// for all imported buffers
	std::vector<std::shared_ptr<VulkanBuffer>> buffers;				// one-one to fg->buffers
	std::shared_ptr<VulkanCommandPool> graphicPool;
	std::shared_ptr<VulkanCommandPool> computePool;
	std::shared_ptr<VulkanCommandPool> rayTracingPool;
	std::shared_ptr<VulkanFence> fence;
	struct PerPassResource {
		std::shared_ptr<VulkanCommandBuffer> cmdBuffer;
		std::shared_ptr<VulkanSemaphore> semaphore;
		std::shared_ptr<VulkanFrameBuffer> frameBuffer;
		std::vector<std::shared_ptr<VulkanSemaphore>> externalWaitingSemaphores;
		std::vector<VkPipelineStageFlags> externalWaitingStages;
		std::vector<std::shared_ptr<VulkanSemaphore>> externalSignalingSemaphores;
	};
	std::vector<PerPassResource> perPassResources;
	std::shared_ptr<VulkanFrameBuffer> frameBuffer;
	std::unordered_map<std::string, std::variant<std::shared_ptr<VulkanSemaphore>>> passStorage;
};

class FrameGraph {
public:
	friend class FGBuilder;
	friend class FGPass;
	friend class FGSubpass;
	friend class FGResourceList;
	FrameGraph() = default;
	void execute(std::shared_ptr<FGResourceList> resources) const;
	std::shared_ptr<FGResourceList> createResourceList(
		std::shared_ptr<VulkanCommandPool> graphicPool,
		std::shared_ptr<VulkanCommandPool> computePool,
		std::shared_ptr<VulkanCommandPool> rayTracingPool) const;
	const FGSubpass& getSubpass(const std::string& name) const;

private:
	std::shared_ptr<VulkanDevice> device;
	std::vector<FGImage> images;
	std::vector<FGBuffer> buffers;
	std::vector<FGPass> passes;
	std::vector<FGSubpass> subpasses;
	std::unordered_map<std::string, int> nameToSubpassIndex;
	std::unordered_map<int, std::vector<const FGImageBinding*>> imageIndexToImageBindingOrders;
	std::unordered_map<int, std::vector<const FGBufferBinding*>> bufferIndexToBufferBindingOrders;
};

template <typename TLambda>
FGNode& FGGraphicsPassNode::addSubpass(const std::string& name, TLambda&& lambda, bool externalSync)
{
	const auto nodeIdx = owner->nodes.size();
	auto& ret = owner->nodes.emplace_back(
		owner, name, PassType::GRAPHICS, static_cast<int>(owner->nodes.size()), index,
		std::make_shared<FGPassExecutor<TLambda>>(std::forward<TLambda>(lambda)), externalSync);
	nodeIndexes.emplace(nodeIdx);
	return ret;
}
}  // namespace xd

#endif	// XD_RT_FRAMEGRAPH_H
