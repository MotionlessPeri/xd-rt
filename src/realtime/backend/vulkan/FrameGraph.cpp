//
// Created by Frank on 2024/1/17.
//

#include "FrameGraph.h"

#include <functional>
#include <queue>
#include <ranges>
#include <stdexcept>
#include <unordered_set>

#include "TextureVk.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanFence.h"
#include "VulkanFrameBuffer.h"
#include "VulkanGlobal.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanQueue.h"
#include "VulkanRenderPass.h"
using namespace xd;

inline bool isWriteAccessBit(VkAccessFlagBits accessFlag)
{
	switch (accessFlag) {
		case VK_ACCESS_INDIRECT_COMMAND_READ_BIT:
		case VK_ACCESS_INDEX_READ_BIT:
		case VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT:
		case VK_ACCESS_UNIFORM_READ_BIT:
		case VK_ACCESS_INPUT_ATTACHMENT_READ_BIT:
		case VK_ACCESS_SHADER_READ_BIT:
		case VK_ACCESS_COLOR_ATTACHMENT_READ_BIT:
		case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT:
		case VK_ACCESS_TRANSFER_READ_BIT:
		case VK_ACCESS_HOST_READ_BIT:
		case VK_ACCESS_MEMORY_READ_BIT:
		case VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT:
		case VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT:
		case VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT:
		case VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR:
		case VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT:
		case VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR:
		case VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV:
			return false;
		case VK_ACCESS_SHADER_WRITE_BIT:
		case VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT:
		case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:
		case VK_ACCESS_TRANSFER_WRITE_BIT:
		case VK_ACCESS_HOST_WRITE_BIT:
		case VK_ACCESS_MEMORY_WRITE_BIT:
		case VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT:
		case VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT:
		case VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR:
		case VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV:
			return true;
		default: {
			assert(false);
			return false;
		}
	}
}
inline bool isWriteAccess(VkAccessFlags flags)
{
	bool ret = false;
	for (const auto i : std::views::iota(0ull, sizeof(VkAccessFlags) * 8)) {
		const unsigned long long bit = 1 << i;
		if (flags & bit) {
			ret |= isWriteAccessBit(static_cast<VkAccessFlagBits>(bit));
		}
	}
	return ret;
}

FGResource::FGResource(FGBuilder* builder, bool imported) : builder(builder), imported(imported) {}

const FGImage* FGResource::toFGImage() const
{
	return dynamic_cast<const FGImage*>(this);
}

const FGBuffer* FGResource::toFGBuffer() const
{
	return dynamic_cast<const FGBuffer*>(this);
}

FGColorAttachment* FGResourceBindingCreationHelper::createColorAttachment(
	const std::string& name,
	FGBuilder* builder,
	int ownerIndex,
	int nodeIndex,
	FGResourceBinding* prev,
	VkAttachmentLoadOp loadOp,
	VkAttachmentStoreOp storeOp,
	VkClearColorValue clearColor)
{
	auto* ret = new FGColorAttachment{builder,
									  ownerIndex,
									  nodeIndex,
									  true,
									  prev,
									  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
									  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
									  VK_IMAGE_ASPECT_COLOR_BIT,
									  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
									  loadOp,
									  storeOp,
									  clearColor};
	builder->nodes[nodeIndex].addColorAttach(name, ret);
	return ret;
}

FGDepthAttachment* FGResourceBindingCreationHelper::createDepthAttachment(
	const std::string& name,
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
	VkClearDepthStencilValue clearValue)
{
	auto* ret = new FGDepthAttachment{builder,	 ownerIndex,	nodeIndex,		isWrite,   prev,
									  stageMask, accessMask,	aspectMask,		layout,	   loadOp,
									  storeOp,	 stencilLoadOp, stencilStoreOp, clearValue};
	builder->nodes[nodeIndex].addDepthAttach(name, ret);
	return ret;
}

FGBufferBinding* FGResourceBindingCreationHelper::createBufferBinding(
	const std::string& name,
	FGBuilder* builder,
	int ownerIndex,
	int nodeIndex,
	bool isWrite,
	VkPipelineStageFlags stageMask,
	VkAccessFlags accessMask,
	FGResourceBinding* prev)
{
	auto* ret =
		new FGBufferBinding{builder, ownerIndex, nodeIndex, isWrite, prev, stageMask, accessMask};
	builder->nodes[nodeIndex].addBufferBinding(name, ret);
	return ret;
}

FGImageBinding* FGResourceBindingCreationHelper::createImageBinding(const std::string& name,
																	FGBuilder* builder,
																	int ownerIndex,
																	int nodeIndex,
																	bool isWrite,
																	VkPipelineStageFlags stageMask,
																	VkAccessFlags accessMask,
																	VkImageAspectFlags aspectMask,
																	VkImageLayout layout,
																	FGResourceBinding* prev)
{
	auto* ret = new FGImageBinding{builder,	  ownerIndex, nodeIndex,  isWrite, prev,
								   stageMask, accessMask, aspectMask, layout};
	builder->nodes[nodeIndex].addImageBinding(name, ret);
	return ret;
}
FGBufferBinding::FGBufferBinding(FGBuilder* builder,
								 int ownerIndex,
								 int nodeIndex,
								 bool write,
								 FGResourceBinding* prev,
								 VkPipelineStageFlags stageMask,
								 VkAccessFlags accessMask)
	: FGResourceBinding(builder, ownerIndex, nodeIndex, write, prev, stageMask, accessMask)
{
}

FGBufferBinding* FGBufferBinding::createBufferBinding(const std::string& name,
													  int nodeIndex,
													  VkPipelineStageFlags stageMask,
													  VkAccessFlags accessMask)
{
	const bool isWrite = isWriteAccess(accessMask);
	auto* ret = FGResourceBindingCreationHelper::createBufferBinding(
		name, builder, ownerIndex, nodeIndex, isWrite, stageMask, accessMask, getPrev());
	recordNewBinding(isWrite, ret);
	return ret;
}

FGImageBinding::FGImageBinding(FGBuilder* builder,
							   int ownerIndex,
							   int nodeIndex,
							   bool write,
							   FGResourceBinding* prev,
							   VkPipelineStageFlags stageMask,
							   VkAccessFlags accessMask,
							   VkImageAspectFlags aspectMask,
							   VkImageLayout layout)
	: FGResourceBinding(builder, ownerIndex, nodeIndex, write, prev, stageMask, accessMask),
	  aspectMask(aspectMask),
	  layout(layout)
{
}

FGColorAttachment* FGImageBinding::createColorAttach(const std::string& name,
													 FGNode& node,
													 VkAttachmentLoadOp loadOp,
													 VkAttachmentStoreOp storeOp,
													 VkClearColorValue clearValue)
{
	auto* ret = FGResourceBindingCreationHelper::createColorAttachment(
		name, builder, ownerIndex, node.index, getPrev(), loadOp, storeOp, clearValue);
	recordNewBinding(true, ret);
	return ret;
}

FGDepthAttachment* FGImageBinding::createDepthAttach(const std::string& name,
													 FGNode& node,
													 VkImageLayout layout,
													 VkPipelineStageFlags stageMask,
													 VkAccessFlags accessMask,
													 VkImageAspectFlags aspectMask,
													 VkAttachmentLoadOp loadOp,
													 VkAttachmentStoreOp storeOp,
													 VkAttachmentLoadOp stencilLoadOp,
													 VkAttachmentStoreOp stencilStoreOp,
													 VkClearDepthStencilValue clearValue)
{
	// Note: layout must be consistent with accessMask
	const bool isWrite = isWriteAccess(accessMask);
	auto* ret = FGResourceBindingCreationHelper::createDepthAttachment(
		name, layout, builder, ownerIndex, node.index, isWrite, stageMask, accessMask, aspectMask,
		getPrev(), loadOp, storeOp, stencilLoadOp, stencilStoreOp, clearValue);
	recordNewBinding(isWrite, ret);
	return ret;
}

FGInputAttachment* FGImageBinding::createInputAttach(const std::string& name,
													 FGNode& node,
													 VkImageLayout layout,
													 VkImageAspectFlags aspectMask,
													 VkPipelineStageFlags stageMask,
													 VkAccessFlags accessMask)
{
	const bool isWrite = isWriteAccess(accessMask);
	auto* ret = new FGInputAttachment{builder,	 ownerIndex, node.index, isWrite, getPrev(),
									  stageMask, accessMask, aspectMask, layout};
	recordNewBinding(isWrite, ret);
	builder->nodes[node.index].addImageBinding(name, ret);
	return ret;
}

FGImageBinding* FGImageBinding::createImageBinding(const std::string& name,
												   FGNode& node,
												   VkImageLayout layout,
												   VkImageAspectFlags aspectMask,
												   VkPipelineStageFlags stageMask,
												   VkAccessFlags accessMask)
{
	const bool isWrite = isWriteAccess(accessMask);
	auto* ret = FGResourceBindingCreationHelper::createImageBinding(
		name, builder, ownerIndex, node.index, isWrite, stageMask, accessMask, aspectMask, layout,
		getPrev());
	recordNewBinding(isWrite, ret);
	return ret;
}

const FGAttachment* FGImageBinding::toAttachment() const
{
	return dynamic_cast<const FGAttachment*>(this);
}

FGDummyImageBinding::FGDummyImageBinding(FGBuilder* builder, int ownerIndex)
	: builder(builder), ownerIndex(ownerIndex)
{
}

FGColorAttachment* FGDummyImageBinding::createColorAttach(const std::string& name,
														  FGNode& node,
														  VkAttachmentLoadOp loadOp,
														  VkAttachmentStoreOp storeOp,
														  VkClearColorValue clearValue) const
{
	auto* ret = FGResourceBindingCreationHelper::createColorAttachment(
		name, builder, ownerIndex, node.index, nullptr, loadOp, storeOp, clearValue);
	setFirstBinding(ret);
	return ret;
}

FGDepthAttachment* FGDummyImageBinding::createDepthAttach(const std::string& name,
														  FGNode& node,
														  VkPipelineStageFlags stageMask,
														  VkAccessFlags accessMask,
														  VkImageAspectFlags aspectMask,
														  VkAttachmentLoadOp loadOp,
														  VkAttachmentStoreOp storeOp,
														  VkAttachmentLoadOp stencilLoadOp,
														  VkAttachmentStoreOp stencilStoreOp,
														  VkClearDepthStencilValue clearValue) const
{
	bool isWrite = isWriteAccess(accessMask);
	auto* ret = FGResourceBindingCreationHelper::createDepthAttachment(
		name, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, builder, ownerIndex, node.index,
		isWrite, stageMask, accessMask, aspectMask, nullptr, loadOp, storeOp, stencilLoadOp,
		stencilStoreOp, clearValue);
	setFirstBinding(ret);
	return ret;
}

FGImageBinding* FGDummyImageBinding::createImageBinding(const std::string& name,
														FGNode& node,
														VkImageLayout layout,
														VkImageAspectFlags aspectMask,
														VkPipelineStageFlags stageMask,
														VkAccessFlags accessMask) const
{
	const bool isWrite = isWriteAccess(accessMask);
	assert(isWrite);
	auto* ret = FGResourceBindingCreationHelper::createImageBinding(
		name, builder, ownerIndex, node.index, true, stageMask, accessMask, aspectMask, layout,
		nullptr);
	setFirstBinding(ret);
	return ret;
}

void FGDummyImageBinding::setFirstBinding(FGImageBinding* binding) const
{
	auto& fgImage = builder->images[ownerIndex];
	assert(fgImage.firstBinding == nullptr);
	builder->images[ownerIndex].firstBinding = binding;
}

const FGColorAttachment* FGAttachment::toColorAttach() const
{
	return dynamic_cast<const FGColorAttachment*>(this);
}
const FGDepthAttachment* FGAttachment::toDepthAttach() const
{
	return dynamic_cast<const FGDepthAttachment*>(this);
}
const FGInputAttachment* FGAttachment::toInputAttach() const
{
	return dynamic_cast<const FGInputAttachment*>(this);
}
const FGResolveAttachment* FGAttachment::toResolveAttach() const
{
	return dynamic_cast<const FGResolveAttachment*>(this);
}

FGBuffer::FGBuffer(FGBuilder* builder,
				   bool imported,
				   VkBufferCreateInfo bufferCi,
				   VkBufferViewCreateInfo bufferViewCi)
	: FGResource(builder, imported),
	  bufferCi(std::move(bufferCi)),
	  bufferViewCi(std::move(bufferViewCi))
{
}

FGImage::FGImage(FGBuilder* builder,
				 VkImageCreateInfo imageCi,
				 VkImageViewCreateInfo imageViewCi,
				 std::shared_ptr<VulkanSampler>
					 sampler,  // TODO: should we change sampler to std::optional<SamplerDesc>?
				 bool imported)
	: FGResource(builder, imported), imageCi(imageCi), imageViewCi(imageViewCi), sampler(sampler)
{
}

FGResourceBinding::FGResourceBinding(FGBuilder* builder,
									 int ownerIndex,
									 int nodeIndex,
									 bool write,
									 FGResourceBinding* prev,
									 VkPipelineStageFlags stageMask,
									 VkAccessFlags accessMask)
	: builder(builder),
	  ownerIndex(ownerIndex),
	  nodeIndex(nodeIndex),
	  write(write),
	  prev(prev),
	  stageMask(stageMask),
	  accessMask(accessMask)
{
}

const FGImageBinding* FGResourceBinding::toImageBinding() const
{
	return dynamic_cast<const FGImageBinding*>(this);
}

const FGBufferBinding* FGResourceBinding::toBufferBinding() const
{
	return dynamic_cast<const FGBufferBinding*>(this);
}

void FGResourceBinding::recordNewBinding(bool isWrite, FGResourceBinding* rb)
{
	if (isWrite) {
		if (next != nullptr)
			assert(false);
		next = rb;
	}
	else {
		reads.emplace_back(rb);
	}
}

FGNode::FGNode(FGBuilder* owner,
			   std::string name,
			   PassType type,
			   int index,
			   int passIndex,
			   std::shared_ptr<FGPassExecutorBase> executor,
			   bool externalSync)
	: owner(owner),
	  name(std::move(name)),
	  type(type),
	  index(index),
	  passIndex(passIndex),
	  executor(std::move(executor)),
	  externalSync(externalSync)
{
}

FGNode::~FGNode()
{
	const auto deleteMap = [&](auto& mapObj) {
		for (const auto rb : mapObj | std::views::values) {
			delete rb;
		}
		mapObj.clear();
	};
	deleteMap(nameToBufferBindings);
	deleteMap(nameToImageBindings);
}

FGDummyImageBinding FGNode::createImage(VkImageCreateInfo imageCi,
										VkImageViewCreateInfo imageViewCi,
										std::shared_ptr<VulkanSampler> sampler)
{
	return owner->createImage(imageCi, imageViewCi, sampler);
}

FGDummyImageBinding FGNode::importImage(VkImageCreateInfo imageCi,
										VkImageViewCreateInfo imageViewCi,
										std::shared_ptr<VulkanSampler> sampler)
{
	return owner->importImage(imageCi, imageViewCi, sampler);
}

void FGNode::addImageBinding(const std::string& name, FGImageBinding* ib)
{
	nameToImageBindings.insert({name, ib});
}

void FGNode::addBufferBinding(const std::string& name, FGBufferBinding* bb)
{
	nameToBufferBindings.insert({name, bb});
}

void FGNode::addAttachment(const std::string& name, FGAttachment* attach)
{
	addImageBinding(name, attach);
	attachments.emplace_back(attach);
}
void FGNode::addColorAttach(const std::string& name, FGColorAttachment* attach)
{
	addAttachment(name, attach);
}
void FGNode::addDepthAttach(const std::string& name, FGDepthAttachment* attach)
{
	addAttachment(name, attach);
}
void FGNode::addInputAttach(const std::string& name, FGInputAttachment* attach)
{
	addAttachment(name, attach);
}
void FGNode::addResolveAttach(const std::string& name, FGResolveAttachment* attach)
{
	addAttachment(name, attach);
}

bool FGNode::isFirstNode() const
{
	const auto check = [&](const auto& mapObj) {
		for (const auto* rb : mapObj | std::views::values) {
			if (rb->prev != nullptr)
				return false;
		}
		return true;
	};
	return check(nameToImageBindings) && check(nameToBufferBindings);
}

int FGNode::getPassIndex() const
{
	return type == PassType::GRAPHICS ? passIndex : -1;
}

FGGraphicsPassNode::FGGraphicsPassNode(FGBuilder* owner, std::string name, int index)
	: owner(owner), name(std::move(name)), index(index)
{
}

FGBuilder::FGBuilder(std::shared_ptr<VulkanDevice> device) : device(std::move(device)) {}

FGGraphicsPassNode& FGBuilder::addGraphicsPass(const std::string& name)
{
	return graphicNodes.emplace_back(this, name, graphicNodes.size());
}

std::shared_ptr<FrameGraph> FGBuilder::build(std::shared_ptr<VulkanDevice> device)
{
	// TODO: cull unused passes and recognize necessary image/buffer copy operations
	// build FGSubpassRef and FGPassRef
	// create subpass refs
	auto subpassBuildRes = buildSubpassAndPassRefs();
	auto& subpassRefs = subpassBuildRes.subpassRefs;
	auto& nodeIndexToSubpassRefIndex = subpassBuildRes.nodeIndexToSubpassRefIndex;
	auto& passRefs = subpassBuildRes.passRefs;
	// record resource binding orders
	std::unordered_map<int, std::vector<const FGImageBinding*>> imageIndexToBindingOrder;
	std::unordered_map<int, std::vector<const FGBufferBinding*>> bufferIndexToBindingOrder;
	std::unordered_map<const FGResourceBinding*, int> resourceBindingToOrderIndex;
	for (const auto& subpassRef : subpassRefs) {
		const auto nodeIndex = subpassRef.nodeIndex;
		const auto& node = nodes[nodeIndex];
		const auto loop = [&](const auto& bindingMap, auto& toBindingOrder) {
			for (const auto* rb : bindingMap | std::views::values) {
				auto& bindingOrderVec = toBindingOrder[rb->ownerIndex];
				bindingOrderVec.emplace_back(rb);
				resourceBindingToOrderIndex[rb] = bindingOrderVec.size() - 1;
			}
		};
		loop(node.nameToBufferBindings, bufferIndexToBindingOrder);
		loop(node.nameToImageBindings, imageIndexToBindingOrder);
	}

	// build subpass and renderpass
	std::unordered_map<int, RenderPassDesc> passRefIndexToRenderPassDesc;
	std::unordered_map<int, int> subpassRefIndexToSubpassDescIndex;
	// build attach descs and add attach refs
	for (auto& passRef : passRefs) {
		if (passRef.type != PassType::GRAPHICS)
			continue;
		RenderPassDesc& renderPassDesc = passRefIndexToRenderPassDesc[passRef.index];
		for (const auto subpassIndex : passRef.subpassRefIndexes) {
			auto& subpassDesc = renderPassDesc.subpasses.emplace_back();
			subpassDesc.desc.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
			subpassDesc.desc.pNext = nullptr;
			subpassDesc.desc.flags = 0;
			subpassDesc.desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDesc.desc.viewMask = 0;
			const auto& subpassRef = subpassRefs[subpassIndex];
			const auto subpassDescIndex = renderPassDesc.subpasses.size() - 1;
			passRef.subpassDescIndexToSubpassRefIndex[subpassDescIndex] = subpassRef.index;
			subpassRefIndexToSubpassDescIndex[subpassRef.index] = subpassDescIndex;
			const auto& node = nodes[subpassRef.nodeIndex];
			for (const auto* attach : node.attachments) {
				const auto& fgImage = images[attach->ownerIndex];
				int attachIndex = INVALID_INDEX;
				if (!passRef.imageIndexToAttachIndex.contains(attach->ownerIndex)) {
					const auto& imageCi = fgImage.imageCi;
					VkAttachmentDescription2& attachDesc =
						renderPassDesc.attachments.emplace_back();
					attachIndex = renderPassDesc.attachments.size() - 1;
					attachDesc.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
					attachDesc.pNext = nullptr;
					attachDesc.format = imageCi.format;
					attachDesc.samples = VK_SAMPLE_COUNT_1_BIT;
					attachDesc.loadOp = attach->getLoadOp();
					attachDesc.storeOp = attach->getStoreOp();
					attachDesc.stencilLoadOp = attach->getStencilLoadOp();
					attachDesc.stencilStoreOp = attach->getStencilStoreOp();
					attachDesc.initialLayout = attach->layout;
					const FGImageBinding* pFinalLayoutBinding = nullptr;
					std::queue<const FGImageBinding*> bindingQueue;
					bindingQueue.emplace(attach);
					const auto compareAndAssign = [&](const FGImageBinding* ib) {
						const auto ibSubpassIndex = nodeIndexToSubpassRefIndex[ib->nodeIndex];
						const auto ibPassIndex = subpassRefs[ibSubpassIndex].passRefIndex;
						if (ibPassIndex > passRef.index) {
							if (pFinalLayoutBinding == nullptr) {
								pFinalLayoutBinding = ib;
							}
							else {
								const auto prevSubpassIndex =
									nodeIndexToSubpassRefIndex[pFinalLayoutBinding->nodeIndex];
								const auto prevPassIndex =
									subpassRefs[prevSubpassIndex].passRefIndex;
								if (ibPassIndex <= prevPassIndex) {
									if (ibSubpassIndex < prevSubpassIndex) {
										pFinalLayoutBinding = ib;
									}
								}
							}
						}
					};
					while (!bindingQueue.empty()) {
						const auto* ib = bindingQueue.front();
						bindingQueue.pop();
						for (const auto* read : ib->reads) {
							compareAndAssign(read->toImageBinding());
						}
						if (ib->next != nullptr)
							compareAndAssign(ib->next->toImageBinding());
					}
					attachDesc.finalLayout = pFinalLayoutBinding != nullptr
												 ? pFinalLayoutBinding->layout
												 : attachDesc.initialLayout;
					passRef.imageIndexToAttachIndex[attach->ownerIndex] = attachIndex;
				}
				else {
					attachIndex = passRef.imageIndexToAttachIndex[attach->ownerIndex];
				}
				auto attachRef = attach->getAttachmentRef(attachIndex);
				if (const auto* ca = attach->toColorAttach(); ca != nullptr) {
					subpassDesc.colorRefs.emplace_back(attachRef);
				}
				else if (const auto* da = attach->toDepthAttach(); da != nullptr) {
					subpassDesc.depthStencilRefs.emplace_back(attachRef);
				}
				else if (const auto* ia = attach->toInputAttach(); ia != nullptr) {
					subpassDesc.inputRefs.emplace_back(attachRef);
				}
				else if (const auto* ra = attach->toResolveAttach(); ra != nullptr) {
					subpassDesc.resolveRefs.emplace_back(attachRef);
				}
				else {
					assert(false);
				}
			}
			subpassDesc.desc.inputAttachmentCount = subpassDesc.inputRefs.size();
			subpassDesc.desc.pInputAttachments =
				subpassDesc.inputRefs.empty() ? nullptr : subpassDesc.inputRefs.data();
			subpassDesc.desc.colorAttachmentCount = subpassDesc.colorRefs.size();
			subpassDesc.desc.pColorAttachments =
				subpassDesc.colorRefs.empty() ? nullptr : subpassDesc.colorRefs.data();
			subpassDesc.desc.pResolveAttachments = nullptr;
			subpassDesc.desc.pDepthStencilAttachment = subpassDesc.depthStencilRefs.empty()
														   ? nullptr
														   : subpassDesc.depthStencilRefs.data();
		}
	}

	// add preserve attachments inside pass refs
	const auto addPreserveAttachesToSubpassRef = [&](FGPassRef& passRef, const FGAttachment* from,
													 const FGResourceBinding* to) {
		// Note: caller must make sure from and to is in the same render pass
		const auto startIndex = nodeIndexToSubpassRefIndex[from->nodeIndex];
		const auto targetIndex = nodeIndexToSubpassRefIndex[to->nodeIndex];
		const auto visitedView = passRef.subpassRefIndexes |
								 std::views::transform([](const int index) -> std::pair<int, bool> {
									 return {index, false};
								 });
		std::unordered_map<int, bool> visited{visitedView.begin(), visitedView.end()};
		visited[targetIndex] = true;
		std::unordered_set<int> tagged;
		tagged.insert(targetIndex);
		std::function<bool(int)> dfs{[&](int nodeIndex) -> bool {
			if (visited[nodeIndex])
				return tagged.contains(nodeIndex);
			const auto& edges = from->reads;
			bool foundPath = false;
			for (const auto& readBinding : edges) {
				const auto& toNodeIndex = nodeIndexToSubpassRefIndex[readBinding->nodeIndex];
				if (!passRef.subpassRefIndexes.contains(toNodeIndex))
					continue;
				if (visited[toNodeIndex]) {
					if (tagged.contains(toNodeIndex)) {
						foundPath = true;
					}
					else
						continue;
				}
				else {
					foundPath = dfs(toNodeIndex);
				}
			}
			visited[nodeIndex] = true;
			if (foundPath && nodeIndex != startIndex) {
				const auto attachIndex = passRef.imageIndexToAttachIndex[from->ownerIndex];
				auto& renderPassDesc = passRefIndexToRenderPassDesc[passRef.index];
				auto& subpassDesc =
					renderPassDesc.subpasses[subpassRefIndexToSubpassDescIndex[nodeIndex]];
				subpassDesc.preserveAttaches.emplace_back(attachIndex);
				tagged.insert(nodeIndex);
			}
			return foundPath;
		}};
		dfs(startIndex);
	};

	// add preserve attach refs
	for (auto& passRef : passRefs) {
		if (passRef.type != PassType::GRAPHICS)
			continue;
		for (auto& subpassRefIndex : passRef.subpassRefIndexes) {
			auto& subpassRef = subpassRefs[subpassRefIndex];
			const auto& node = nodes[subpassRef.nodeIndex];
			for (const auto* attach : node.attachments) {
				for (const auto* read : attach->reads) {
					addPreserveAttachesToSubpassRef(passRef, attach, read);
				}
				if (attach->next != nullptr)
					addPreserveAttachesToSubpassRef(passRef, attach, attach->next);
			}
		}
	}
	for (auto& renderPassDesc : passRefIndexToRenderPassDesc | std::views::values) {
		for (auto& subpassDesc : renderPassDesc.subpasses) {
			subpassDesc.desc.preserveAttachmentCount = subpassDesc.preserveAttaches.size();
			subpassDesc.desc.pPreserveAttachments = subpassDesc.preserveAttaches.empty()
														? nullptr
														: subpassDesc.preserveAttaches.data();
		}
	}
	// add pass dependencies
	struct PairHasher {
		std::size_t operator()(const std::pair<int, int>& obj) const noexcept
		{
			std::size_t seed = 0x5F140906;
			seed ^= (seed << 6) + (seed >> 2) + 0x446ACE00 + static_cast<std::size_t>(obj.first);
			seed ^= (seed << 6) + (seed >> 2) + 0x67E44614 + static_cast<std::size_t>(obj.second);
			return seed;
		}
	};

	for (const auto& passRef : passRefs) {
		if (passRef.type != PassType::GRAPHICS)
			continue;
		std::unordered_map<std::pair<int, int>, VkSubpassDependency2, PairHasher> subpassDeps;
		const auto addMasks = [&subpassDeps](int from, int to, VkPipelineStageFlags srcStageMask,
											 VkPipelineStageFlags dstStageMask,
											 VkAccessFlags srcAccessMask,
											 VkAccessFlags dstAccessMask) -> void {
			std::pair<int, int> key{from, to};
			if (!subpassDeps.contains(key)) {
				VkSubpassDependency2 dep;
				dep.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
				dep.pNext = nullptr;
				dep.srcSubpass = from;
				dep.dstSubpass = to;
				dep.srcStageMask = VK_PIPELINE_STAGE_NONE;
				dep.dstStageMask = VK_PIPELINE_STAGE_NONE;
				dep.srcAccessMask = VK_ACCESS_NONE;
				dep.dstAccessMask = VK_ACCESS_NONE;
				dep.dependencyFlags = 0;
				dep.viewOffset = 0;
				subpassDeps[key] = dep;
			}
			auto& dep = subpassDeps[key];
			dep.srcStageMask |= srcStageMask;
			dep.dstStageMask |= dstStageMask;
			dep.srcAccessMask |= srcAccessMask;
			dep.dstAccessMask |= dstAccessMask;
		};
		for (const auto& subpassRefIndex : passRef.subpassRefIndexes) {
			const auto& subpassRef = subpassRefs[subpassRefIndex];
			const auto& node = nodes[subpassRef.nodeIndex];
			const auto loop = [&](const auto& bindingMap) {
				for (const auto* rb : bindingMap | std::views::values) {
					const auto rbSubpssRefIndex = nodeIndexToSubpassRefIndex[rb->nodeIndex];
					const auto rbDescIndex = subpassRefIndexToSubpassDescIndex[rbSubpssRefIndex];
					// add external dep
					if (rb->prev == nullptr) {
						// first usage of a resource
						addMasks(VK_SUBPASS_EXTERNAL, rbDescIndex, rb->stageMask, rb->stageMask, 0,
								 rb->accessMask);
					}
					else {
						// check if the prev is from another pass
						const auto prevSubpassRefIndex =
							nodeIndexToSubpassRefIndex[rb->prev->nodeIndex];
						const auto prevPassRefIndex = subpassRefs[prevSubpassRefIndex].passRefIndex;
						if (prevPassRefIndex != passRef.index) {
							// we need an external dep from outside to this subpass
							addMasks(VK_SUBPASS_EXTERNAL, rbDescIndex, rb->prev->stageMask,
									 rb->stageMask, rb->prev->accessMask, rb->accessMask);
						}
					}
					//
					for (const auto* read : rb->reads) {
						const auto readSubpassRefIndex =
							nodeIndexToSubpassRefIndex[read->nodeIndex];
						if (!passRef.subpassRefIndexes.contains(readSubpassRefIndex))
							continue;
						const auto readDescIndex =
							subpassRefIndexToSubpassDescIndex[readSubpassRefIndex];
						addMasks(rbDescIndex, readDescIndex, rb->stageMask, read->stageMask,
								 rb->accessMask, read->accessMask);
					}
					if (rb->next != nullptr) {
						const auto nextSubpassRefIndex =
							nodeIndexToSubpassRefIndex[rb->next->nodeIndex];
						if (passRef.subpassRefIndexes.contains(nextSubpassRefIndex)) {
							const auto nextDescIndex =
								subpassRefIndexToSubpassDescIndex[nextSubpassRefIndex];
							addMasks(rbDescIndex, nextDescIndex, rb->stageMask, rb->next->stageMask,
									 rb->accessMask, rb->next->accessMask);
						}
					}
				}
			};
			loop(node.nameToImageBindings);
			loop(node.nameToBufferBindings);
		}
		auto& renderPassDesc = passRefIndexToRenderPassDesc[passRef.index];
		const auto depView = subpassDeps | std::views::values;
		renderPassDesc.dependencys = {depView.begin(), depView.end()};
	}

	// build frame graph
	std::shared_ptr<FrameGraph> fg = std::make_shared<FrameGraph>();
	fg->device = device;
	for (const auto& passRef : passRefs) {
		auto& pass = fg->passes.emplace_back();
		const int passIndex = fg->passes.size() - 1;
		pass.type = passRef.type;
		pass.fg = fg.get();
		pass.index = passIndex;
		if (passRef.type == PassType::GRAPHICS) {
			auto& renderPassDesc = passRefIndexToRenderPassDesc[passRef.index];
			const auto subpassView = renderPassDesc.subpasses |
									 std::views::transform([](const SubpassDesc& subpassDesc) {
										 return subpassDesc.desc;
									 });
			std::vector<VkSubpassDescription2> vkSubpassDescs{subpassView.begin(),
															  subpassView.end()};
			renderPassDesc.ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
			renderPassDesc.ci.pNext = nullptr;
			renderPassDesc.ci.flags = 0;
			renderPassDesc.ci.attachmentCount = renderPassDesc.attachments.size();
			renderPassDesc.ci.pAttachments = renderPassDesc.attachments.data();
			renderPassDesc.ci.subpassCount = vkSubpassDescs.size();
			renderPassDesc.ci.pSubpasses = vkSubpassDescs.data();
			renderPassDesc.ci.dependencyCount = renderPassDesc.dependencys.size();
			renderPassDesc.ci.pDependencies =
				renderPassDesc.dependencys.empty() ? nullptr : renderPassDesc.dependencys.data();
			renderPassDesc.ci.correlatedViewMaskCount = 0;
			renderPassDesc.ci.pCorrelatedViewMasks = nullptr;
			const auto renderPass = device->createRenderPass(renderPassDesc);
			const auto vulkanSubpasses = renderPass->getSubpasses();

			pass.renderPass = renderPass;
			for (const auto i : std::views::iota(0, static_cast<int>(vulkanSubpasses.size()))) {
				const auto& subpassRefIndex = passRef.subpassDescIndexToSubpassRefIndex.at(i);
				const auto& subpassRef = subpassRefs[subpassRefIndex];
				auto& node = nodes[subpassRef.nodeIndex];

				auto& subpass = fg->subpasses.emplace_back();
				const int subpassIndex =
					fg->subpasses.size() -
					1;	// This index should be consistency with FGSubpassRef's index
				assert(subpassRefIndex == subpassIndex);
				pass.subpassIndexes.emplace_back(subpassIndex);
				fg->nameToSubpassIndex[node.name] = subpassIndex;
				subpass.fg = fg.get();
				subpass.passIndex = passIndex;
				subpass.subpass = vulkanSubpasses[i];
				subpass.executor = node.executor;
				subpass.nameToBufferBindings = std::move(node.nameToBufferBindings);
				subpass.nameToImageBindings = std::move(node.nameToImageBindings);
				subpass.externalSync = node.externalSync;
			}
			pass.imageIndexToAttachIndex = std::move(passRef.imageIndexToAttachIndex);
		}
		else if (passRef.type == PassType::RAY_TRACING) {
			assert(false);	// TODO
		}
		else {
			const auto& subpassRefIndex = *passRef.subpassRefIndexes.begin();
			const auto& subpassRef = subpassRefs[subpassRefIndex];
			auto& node = nodes[subpassRef.nodeIndex];
			auto& subpass = fg->subpasses.emplace_back();
			const int subpassIndex = fg->subpasses.size() - 1;
			assert(subpassRefIndex == subpassIndex);
			pass.subpassIndexes.emplace_back(subpassIndex);
			fg->nameToSubpassIndex[node.name] = subpassIndex;
			subpass.fg = fg.get();
			subpass.passIndex = passIndex;
			subpass.subpass = nullptr;
			subpass.executor = node.executor;
			subpass.nameToBufferBindings = std::move(node.nameToBufferBindings);
			subpass.nameToImageBindings = std::move(node.nameToImageBindings);
			subpass.externalSync = node.externalSync;
		}
	}
	fg->imageIndexToImageBindingOrders = std::move(imageIndexToBindingOrder);
	fg->bufferIndexToBufferBindingOrders = std::move(bufferIndexToBindingOrder);
	fg->images = std::move(images);
	fg->buffers = std::move(buffers);
	return fg;
}

FGDummyImageBinding FGBuilder::createImage(VkImageCreateInfo imageCi,
										   VkImageViewCreateInfo imageViewCi,
										   std::shared_ptr<VulkanSampler> sampler)
{
	images.emplace_back(this, imageCi, imageViewCi, sampler, false);
	return FGDummyImageBinding{this, static_cast<int>(images.size()) - 1};
}

FGDummyImageBinding FGBuilder::importImage(VkImageCreateInfo imageCi,
										   VkImageViewCreateInfo imageViewCi,
										   std::shared_ptr<VulkanSampler> sampler)
{
	images.emplace_back(this, imageCi, imageViewCi, sampler, true);
	return FGDummyImageBinding{this, static_cast<int>(images.size()) - 1};
}

void FGPass::execute(std::shared_ptr<VulkanDevice> device,
					 FGResourceList& resources,
					 std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
					 const std::vector<std::shared_ptr<VulkanSemaphore>>& prevPassSemaphores) const
{
	switch (type) {
		case PassType::GRAPHICS: {
			executeGraphicPass(device, resources, cmdBuffer, prevPassSemaphores);
			return;
		}
		case PassType::COMPUTE: {
			executeComputePass(device, resources, cmdBuffer, prevPassSemaphores);
			return;
		}
		case PassType::RAY_TRACING: {
			executeRayTracingPass(device, resources, cmdBuffer, prevPassSemaphores);
			return;
		}
		case PassType::OTHER: {
			executeOtherPass(device, resources, cmdBuffer, prevPassSemaphores);
			return;
		}
		default:
			return;
	}
}

FGBuilder::BuildSubpassRefRetType FGBuilder::buildSubpassAndPassRefs() const
{
	// aliasing
	BuildSubpassRefRetType ret;
	auto& subpassRefs = ret.subpassRefs;
	auto& nodeIndexToSubpassRefIndex = ret.nodeIndexToSubpassRefIndex;
	auto& passRefs = ret.passRefs;
	auto& nodeIndexToPassRefIndex = ret.graphicsNodeIndexToPassRefIndex;
	// build FGSubpassRefs and record first layer FGNodes
	std::queue<int> nodeIndexQueue;
	std::unordered_set<int> recordedNodeIndexes;
	const auto emplaceNodeIndex = [&](int index) {
		if (!recordedNodeIndexes.contains(index)) {
			nodeIndexQueue.emplace(index);
			recordedNodeIndexes.emplace(index);
		}
	};
	for (const auto i : std::views::iota(0, static_cast<int>(nodes.size()))) {
		auto& node = nodes[i];
		auto& subpassRef = subpassRefs.emplace_back();
		subpassRef.nodeIndex = i;
		subpassRef.index = subpassRefs.size() - 1;
		nodeIndexToSubpassRefIndex[i] = subpassRefs.size() - 1;
		if (node.isFirstNode()) {
			if (!recordedNodeIndexes.contains(i)) {
				nodeIndexQueue.emplace(i);
				recordedNodeIndexes.emplace(i);
			}
		}
	}

	const auto addNewPassRef = [&](PassType type) -> int {
		auto& passRef = passRefs.emplace_back();
		passRef.index = passRefs.size() - 1;
		passRef.type = type;
		return passRef.index;
	};
	// add edges to subpass refs
	while (!nodeIndexQueue.empty()) {
		const auto nodeIndex = nodeIndexQueue.front();
		auto& node = nodes[nodeIndex];
		nodeIndexQueue.pop();

		const auto subpassRefIndex = nodeIndexToSubpassRefIndex[nodeIndex];
		auto& subpassRef = subpassRefs[subpassRefIndex];
		// add edges according to read and write bindings
		const auto loop = [&](const auto& bindingMap) {
			for (const auto* rb : bindingMap | std::views::values) {
				// rb=>reads to rb=>next needs an edge(reads before write)
				if (rb->next != nullptr) {
					emplaceNodeIndex(rb->next->nodeIndex);
					const auto nextSubpassRefIndex =
						nodeIndexToSubpassRefIndex[rb->next->nodeIndex];
					auto& nextSubpassRef = subpassRefs[nextSubpassRefIndex];
					subpassRef.outs.emplace(nextSubpassRefIndex);
					nextSubpassRef.ins.emplace(subpassRefIndex);
					for (const auto* read : rb->reads) {
						emplaceNodeIndex(read->nodeIndex);
						const auto readSubpassRefIndex =
							nodeIndexToSubpassRefIndex[read->nodeIndex];
						if (readSubpassRefIndex != nextSubpassRefIndex) {
							auto& readSubpassRef = subpassRefs[readSubpassRefIndex];
							readSubpassRef.outs.emplace(nextSubpassRefIndex);
							nextSubpassRef.ins.emplace(readSubpassRefIndex);
						}
					}
				}
				// if rb is a write binding, rb => rb->reads needs an edge if it's not in the same
				// node(read after write)
				// we do support read and write simultaneously in a single pass, like a tonemapping
				// pass
				if (rb->write) {
					for (const auto* read : rb->reads) {
						emplaceNodeIndex(read->nodeIndex);
						const auto readSubpassRefIndex =
							nodeIndexToSubpassRefIndex[read->nodeIndex];
						if (readSubpassRefIndex != subpassRefIndex) {
							auto& readSubpassRef = subpassRefs[readSubpassRefIndex];
							subpassRef.outs.emplace(readSubpassRefIndex);
							readSubpassRef.ins.emplace(subpassRefIndex);
						}
					}
				}
			}
		};
		loop(node.nameToImageBindings);
		loop(node.nameToBufferBindings);
	}
	// topological sort
	topologicalSort(subpassRefs);
	// build pass ref relevant to subpass ref
	for (auto& subpassRef : subpassRefs) {
		const auto& node = nodes[subpassRef.index];
		int passRefIndex;
		if (const auto passIndex = node.getPassIndex(); passIndex != -1) {
			const auto passRefIt = nodeIndexToPassRefIndex.find(passIndex);
			if (passRefIt == nodeIndexToPassRefIndex.end()) {
				passRefIndex = addNewPassRef(PassType::GRAPHICS);
			}
			else {
				passRefIndex = passRefIt->second;
			}
		}
		else {
			passRefIndex = addNewPassRef(node.type);
		}
		passRefs[passRefIndex].subpassRefIndexes.emplace(subpassRef.index);
		subpassRef.passRefIndex = passRefIndex;
	}
	// add edges to pass refs
	for (const auto& subpassRef : subpassRefs) {
		for (const auto toRefIndex : subpassRef.outs) {
			const auto& toRef = subpassRefs[toRefIndex];
			if (subpassRef.passRefIndex != toRef.passRefIndex) {
				auto& fromPass = passRefs[subpassRef.passRefIndex];
				auto& toPass = passRefs[toRef.passRefIndex];
				fromPass.outs.emplace(toPass.index);
				toPass.ins.emplace(fromPass.index);
			}
		}
	}
	// topological sort
	topologicalSort(passRefs);
	return ret;
}

bool FGPass::isExternalSync() const
{
	if (type != PassType::OTHER)
		return false;
	return fg->subpasses[subpassIndexes.front()].externalSync;
}

void FGPass::executeGraphicPass(
	std::shared_ptr<VulkanDevice> device,
	FGResourceList& resources,
	std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
	const std::vector<std::shared_ptr<VulkanSemaphore>>& prevPassSemaphores) const
{
	const auto clearValues = buildClearValues();
	auto& frameBuffer = resources.perPassResources[index].frameBuffer;
	if (frameBuffer == nullptr) {
		frameBuffer = buildFrameBuffer(device, resources);
	}
	const auto& refAttach = fg->images[imageIndexToAttachIndex.begin()->first];
	for (const auto subpassIndex : subpassIndexes) {
		const auto& subpass = fg->subpasses[subpassIndex];
		subpass.transitResources(resources, cmdBuffer);
	}
	VkRenderPassBeginInfo info;
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	info.pNext = nullptr;
	info.renderPass = renderPass->pass;
	info.framebuffer = frameBuffer->frameBuffer;
	info.renderArea.offset = {0, 0};
	info.renderArea.extent = {refAttach.imageCi.extent.width, refAttach.imageCi.extent.height};
	info.clearValueCount = clearValues.size();
	info.pClearValues = clearValues.data();
	cmdBuffer->beginRenderPass(info, VK_SUBPASS_CONTENTS_INLINE);
	for (const auto subpassIndex : subpassIndexes) {
		const auto& subpass = fg->subpasses[subpassIndex];
		subpass.execute(device, resources, cmdBuffer, prevPassSemaphores);
		if (subpassIndex != subpassIndexes.back())
			cmdBuffer->nextSubpass(VK_SUBPASS_CONTENTS_INLINE);
	}
	cmdBuffer->endRenderPass();
}

void FGPass::executeComputePass(
	std::shared_ptr<VulkanDevice> device,
	FGResourceList& resources,
	std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
	const std::vector<std::shared_ptr<VulkanSemaphore>>& prevPassSemaphores) const
{
	const auto& subpass = fg->subpasses[subpassIndexes.front()];
	subpass.transitResources(resources, cmdBuffer);
	subpass.execute(device, resources, cmdBuffer, prevPassSemaphores);
	return;
}

void FGPass::executeRayTracingPass(
	std::shared_ptr<VulkanDevice> device,
	FGResourceList& resources,
	std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
	const std::vector<std::shared_ptr<VulkanSemaphore>>& prevPassSemaphores) const
{
	assert(false);
	return;
}

void FGPass::executeOtherPass(
	std::shared_ptr<VulkanDevice> device,
	FGResourceList& resources,
	std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
	const std::vector<std::shared_ptr<VulkanSemaphore>>& prevPassSemaphores) const
{
	const auto& subpass = fg->subpasses[subpassIndexes.front()];
	subpass.transitResources(resources, cmdBuffer);
	subpass.execute(device, resources, cmdBuffer, prevPassSemaphores);
	return;
}

std::shared_ptr<VulkanFrameBuffer> FGPass::buildFrameBuffer(std::shared_ptr<VulkanDevice> device,
															const FGResourceList& resources) const
{
	std::vector<VkImageView> attachments(imageIndexToAttachIndex.size());
	for (const auto [imageIndex, attachIndex] : imageIndexToAttachIndex) {
		attachments[attachIndex] = resources.images[imageIndex]->imageView->imageView;
	}
	const auto& refAttach = fg->images[imageIndexToAttachIndex.begin()->first];
	VkFramebufferCreateInfo ci;
	ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	ci.pNext = nullptr;
	ci.flags = 0;
	ci.renderPass = renderPass->pass;
	ci.attachmentCount = attachments.size();
	ci.pAttachments = attachments.empty() ? nullptr : attachments.data();
	ci.width = refAttach.imageCi.extent.width;
	ci.height = refAttach.imageCi.extent.height;
	ci.layers = 1;
	return device->createFrameBuffer(ci);
}

std::vector<VkClearValue> FGPass::buildClearValues() const
{
	std::vector<VkClearValue> clearValues(imageIndexToAttachIndex.size());
	for (const auto subpassIndex : subpassIndexes) {
		const auto& subpass = fg->subpasses[subpassIndex];
		for (const auto* attach : subpass.nameToImageBindings | std::views::values |
									  std::views::transform([](const FGImageBinding* binding) {
										  return binding->toAttachment();
									  })) {
			if (attach == nullptr)
				continue;
			const auto imageIndex = attach->ownerIndex;
			const auto attachIndex = imageIndexToAttachIndex.at(imageIndex);
			if (const auto* ca = attach->toColorAttach(); ca != nullptr) {
				clearValues[attachIndex].color = ca->clearColor;
			}
			if (const auto* da = attach->toDepthAttach(); da != nullptr) {
				clearValues[attachIndex].depthStencil = da->depthStencilClearValue;
			}
		}
	}
	return clearValues;
}

std::shared_ptr<VulkanRenderPass> FGSubpass::getRenderPass() const
{
	return fg->passes[passIndex].getRenderPass();
}

void FGSubpass::execute(
	std::shared_ptr<VulkanDevice> device,
	FGResourceList& resources,
	std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
	const std::vector<std::shared_ptr<VulkanSemaphore>>& prevPassSemaphores) const
{
	// transit resources
	// transitResources(resources, cmdBuffer);
	// run executor
	executor->execute(resources, cmdBuffer, prevPassSemaphores);
}

void FGSubpass::transitResources(const FGResourceList& resources,
								 std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const
{
	// TODO: figure out resource transition inside render pass
	// If a resource is transited to attach, then it's handled by render pass
	// Seems there're limits inside resource transition in renderpass
	// transit resources
	struct StageMaskPairHasher {
		std::size_t operator()(
			const std::pair<VkPipelineStageFlags, VkPipelineStageFlags>& obj) const noexcept
		{
			std::size_t seed = 0x4C3409D1;
			seed ^= (seed << 6) + (seed >> 2) + 0x1C727A87 + static_cast<std::size_t>(obj.first);
			seed ^= (seed << 6) + (seed >> 2) + 0x6632570C + static_cast<std::size_t>(obj.second);
			return seed;
		}
	};
	struct BarrierVec {
		std::vector<VkImageMemoryBarrier> imageBarriers;
		std::vector<VkBufferMemoryBarrier> bufferBarriers;
	};
	std::unordered_map<std::pair<VkPipelineStageFlags, VkPipelineStageFlags>, BarrierVec,
					   StageMaskPairHasher>
		pipelineStageToBarriers;
	const auto buildImageBarrier = [&](VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
									   VkImageLayout oldLayout, VkImageLayout newLayout,
									   VkImage image,
									   VkImageAspectFlags aspectMask) -> VkImageMemoryBarrier {
		VkImageMemoryBarrier ret;
		ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		ret.pNext = nullptr;
		ret.srcAccessMask = srcAccessMask;
		ret.dstAccessMask = dstAccessMask;
		ret.oldLayout = oldLayout;
		ret.newLayout = newLayout;
		ret.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ret.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ret.image = image;
		ret.subresourceRange.aspectMask = aspectMask;
		ret.subresourceRange.baseMipLevel = 0;
		ret.subresourceRange.levelCount = 1;
		ret.subresourceRange.baseArrayLayer = 0;
		ret.subresourceRange.layerCount = 1;
		return ret;
	};
	const auto buildBufferBarrier = [&](VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
										VkBuffer buffer) {
		VkBufferMemoryBarrier ret;
		ret.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		ret.pNext = nullptr;
		ret.srcAccessMask = srcAccessMask;
		ret.dstAccessMask = dstAccessMask;
		ret.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ret.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		ret.buffer = buffer;
		ret.offset = 0;
		ret.size = VK_WHOLE_SIZE;
		return ret;
	};
	// TODO: figure out if a dst is an attach. Attachment transition is handled by render pass
	for (const auto* ib : nameToImageBindings | std::views::values) {
		const auto imageIndex = ib->ownerIndex;
		const auto vkTexture = resources.images.at(imageIndex);
		const auto& bindingOrderVec = fg->imageIndexToImageBindingOrders.at(imageIndex);
		auto thisIt = std::ranges::find(bindingOrderVec, ib);
		if (thisIt != bindingOrderVec.begin()) {
			// transit from prev state to current state
			const auto* prevIb = *std::prev(thisIt);
			// If prevIb has been an attachment in the prev pass and prev pass is graphics, then the
			// automatic layout transition is performed by vulkan
			if (fg->passes[prevIb->nodeIndex].type == PassType::GRAPHICS) {
				const auto firstIt = std::ranges::find_if(
					bindingOrderVec,
					[&](const FGImageBinding* obj) { return obj->nodeIndex == prevIb->nodeIndex; });
				if (std::ranges::any_of(firstIt, thisIt, [](const FGImageBinding* obj) {
						return obj->isAttachment();
					}))
					continue;
			}

			if (!(prevIb->accessMask == ib->accessMask && prevIb->layout == ib->layout)) {
				// need a transition
				pipelineStageToBarriers[{prevIb->stageMask, ib->stageMask}]
					.imageBarriers.emplace_back(
						buildImageBarrier(prevIb->accessMask, ib->accessMask, prevIb->layout,
										  ib->layout, vkTexture->image->image, ib->aspectMask));
			}
		}
		else {
			// check if imported
			const auto& fgImage = fg->images[imageIndex];
			if (fgImage.imported) {
				// transit from import state to current state
				const auto& importState = resources.imageIndexToImportState.at(imageIndex);
				if (!(importState.accessMask == ib->accessMask &&
					  importState.layout == ib->layout)) {
					pipelineStageToBarriers[{importState.stageMask, ib->stageMask}]
						.imageBarriers.emplace_back(buildImageBarrier(
							importState.accessMask, ib->accessMask, importState.layout, ib->layout,
							vkTexture->image->image, ib->aspectMask));
				}
			}
		}
	}
	for (const auto* bb : nameToBufferBindings | std::views::values) {
		const auto bufferIndex = bb->ownerIndex;
		const auto vulkanBuffer = resources.buffers.at(bufferIndex);
		const auto& bindingOrderVec = fg->bufferIndexToBufferBindingOrders.at(bufferIndex);
		auto it = std::ranges::find(bindingOrderVec, bb);
		if (it != bindingOrderVec.begin()) {
			const auto* prevBb = *std::prev(it);
			if (prevBb->accessMask != bb->accessMask) {
				pipelineStageToBarriers[{prevBb->stageMask, bb->stageMask}]
					.bufferBarriers.emplace_back(buildBufferBarrier(
						prevBb->accessMask, bb->accessMask, vulkanBuffer->buffer));
			}
		}
		else {
			const auto& fgBuffer = fg->buffers[bufferIndex];
			if (fgBuffer.imported) {
				const auto importState = resources.bufferIndexToImportState.at(bufferIndex);
				if (importState.accessMask != bb->accessMask) {
					pipelineStageToBarriers[{importState.stageMask, bb->stageMask}]
						.bufferBarriers.emplace_back(buildBufferBarrier(
							importState.accessMask, bb->accessMask, vulkanBuffer->buffer));
				}
			}
		}
	}
	for (const auto it : pipelineStageToBarriers) {
		const auto& stageMaskPair = it.first;
		cmdBuffer->pipelineBarrier(stageMaskPair.first, stageMaskPair.second, 0, {},
								   it.second.bufferBarriers, it.second.imageBarriers);
	}
}

void FGResourceList::bindImage(const std::string& subpassName,
							   const std::string& bindingName,
							   std::shared_ptr<TextureVk> image,
							   VkPipelineStageFlags stageMask,
							   VkImageLayout currentLayout,
							   VkAccessFlags accessMask)
{
	const auto subpassIndex = fg->nameToSubpassIndex.at(subpassName);
	const auto& subpass = fg->subpasses[subpassIndex];
	const auto* binding = subpass.nameToImageBindings.at(bindingName);
	const auto index = binding->ownerIndex;
	const auto& fgImage = fg->images[index];
	assert(fgImage.imported == true);
	images[index] = image;
	imageIndexToImportState[index] = {stageMask, currentLayout, accessMask};
}

void FGResourceList::bindBuffer(const std::string& subpassName,
								const std::string& bindingName,
								std::shared_ptr<VulkanBuffer> buffer,
								VkPipelineStageFlags stageMask,
								VkAccessFlags accessMask)
{
	const auto subpassIndex = fg->nameToSubpassIndex.at(subpassName);
	const auto& subpass = fg->subpasses[subpassIndex];
	const auto* binding = subpass.nameToBufferBindings.at(bindingName);
	const auto index = binding->ownerIndex;
	const auto& fgBuffer = fg->buffers[index];
	assert(fgBuffer.imported == true);
	buffers[index] = buffer;
	bufferIndexToImportState[index] = {stageMask, accessMask};
}

std::shared_ptr<TextureVk> FGResourceList::getImage(const std::string& subpassName,
													const std::string& bindingName) const
{
	const auto subpassIndex = fg->nameToSubpassIndex.at(subpassName);
	const auto& subpass = fg->subpasses[subpassIndex];
	const auto* binding = subpass.nameToImageBindings.at(bindingName);
	const auto index = binding->ownerIndex;
	return images[index];
}

std::shared_ptr<VulkanBuffer> FGResourceList::getBuffer(const std::string& subpassName,
														const std::string& bindingName) const
{
	const auto subpassIndex = fg->nameToSubpassIndex.at(subpassName);
	const auto& subpass = fg->subpasses[subpassIndex];
	const auto* binding = subpass.nameToBufferBindings.at(bindingName);
	const auto index = binding->ownerIndex;
	return buffers[index];
}

void FGResourceList::addExternalWaitingSemaphore(const std::string& passName,
												 std::shared_ptr<VulkanSemaphore> semaphore,
												 VkPipelineStageFlags waitingStage)
{
	const auto subpassIndex = fg->nameToSubpassIndex.at(passName);
	const auto& subpass = fg->subpasses[subpassIndex];
	const auto passIndex = subpass.passIndex;
	perPassResources[passIndex].externalWaitingSemaphores.emplace_back(semaphore);
	perPassResources[passIndex].externalWaitingStages.emplace_back(waitingStage);
}

void FGResourceList::addExternalSignalingSemaphore(const std::string& passName,
												   std::shared_ptr<VulkanSemaphore> semaphore)
{
	const auto subpassIndex = fg->nameToSubpassIndex.at(passName);
	const auto& subpass = fg->subpasses[subpassIndex];
	const auto passIndex = subpass.passIndex;
	perPassResources[passIndex].externalSignalingSemaphores.emplace_back(semaphore);
}

FGResourceList::FGResourceList(std::shared_ptr<VulkanDevice> device,
							   const FrameGraph* fg,
							   int imageCount,
							   int bufferCount,
							   std::shared_ptr<VulkanCommandPool> graphicPool,
							   std::shared_ptr<VulkanCommandPool> computePool,
							   std::shared_ptr<VulkanCommandPool> rayTracingPool)
	: device(device),
	  fg(fg),
	  images(imageCount, nullptr),
	  buffers(bufferCount, nullptr),
	  graphicPool(graphicPool),
	  computePool(computePool),
	  rayTracingPool(rayTracingPool)
{
	VkFenceCreateInfo fenceCi;
	fenceCi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCi.pNext = nullptr;
	fenceCi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	fence = device->createFence(fenceCi);
	prepareResources();
	preparePerPassResources();
}

void FGResourceList::prepareResources()
{
	for (const auto index : std::views::iota(0, static_cast<int>(fg->images.size()))) {
		auto& fgImage = fg->images[index];
		if (fgImage.imported)
			continue;
		const auto* firstBinding = fgImage.firstBinding;
		auto image = device->createImage(fgImage.imageCi, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VkImageMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = VK_ACCESS_NONE;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = firstBinding->layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = firstBinding->aspectMask;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		image->transitState(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, firstBinding->stageMask,
							std::move(barrier));
		auto imageViewCiCopy = fgImage.imageViewCi;
		auto imageView = image->createImageView(std::move(imageViewCiCopy));
		images[index] = std::make_shared<TextureVk>(image, imageView, fgImage.sampler);
	}
	// if (!std::ranges::all_of(
	//		images, [](const std::shared_ptr<TextureVk>& texture) { return texture != nullptr; })) {
	//	assert(false);
	// }
	//  TODO: add buffer creation
	for (const auto index : std::views::iota(0, static_cast<int>(fg->buffers.size()))) {
		const auto& fgBuffer = fg->buffers[index];
		if (fgBuffer.imported)
			continue;
		buffers[index] =
			device->createBuffer(fgBuffer.bufferCi, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}
	// if (!std::ranges::all_of(buffers, [](const std::shared_ptr<VulkanBuffer>& buffer) {
	//		return buffer != nullptr;
	//	})) {
	//	assert(false);
	// }
}

void FGResourceList::preparePerPassResources()
{
	const auto perPassResourceView = fg->passes | std::views::transform([&](const FGPass& pass) {
										 PerPassResource ret;
										 VkSemaphoreCreateInfo semaphoreCi;
										 semaphoreCi.sType =
											 VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
										 semaphoreCi.pNext = nullptr;
										 semaphoreCi.flags = 0;
										 ret.semaphore = device->createSemaphore(semaphoreCi);
										 ret.cmdBuffer = createCmdBuffer(pass.type);
										 return ret;
									 });
	perPassResources = {perPassResourceView.begin(), perPassResourceView.end()};
}

std::shared_ptr<VulkanCommandBuffer> FGResourceList::createCmdBuffer(PassType type) const
{
	VkCommandBufferAllocateInfo cmdBufferInfo;
	cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferInfo.pNext = nullptr;
	cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferInfo.commandBufferCount = 1;
	switch (type) {
		case PassType::GRAPHICS:
			return graphicPool->allocateCommandBuffers(std::move(cmdBufferInfo)).front();
		case PassType::COMPUTE:
			return computePool->allocateCommandBuffers(std::move(cmdBufferInfo)).front();
		case PassType::RAY_TRACING:
			return rayTracingPool->allocateCommandBuffers(std::move(cmdBufferInfo)).front();
		case PassType::OTHER:
		default:
			return graphicPool->allocateCommandBuffers(std::move(cmdBufferInfo))
				.front();  // Fallback to graphic cmd buffer
	}
}

void FrameGraph::execute(std::shared_ptr<FGResourceList> resources) const
{
	resources->fence->wait();
	resources->fence->reset();
	int lastSyncPassIndex = 0;
	for (int i = passes.size() - 1; i >= 0; --i) {
		if (!passes[i].isExternalSync()) {
			lastSyncPassIndex = i;
			break;
		}
	}
	for (const auto i : std::views::iota(0, static_cast<int>(passes.size()))) {
		const auto& pass = passes[i];
		const auto& perPassResource = resources->perPassResources[i];
		// TODO: for now we run all passes in a sequential order. Changing it to parallel ASAP.
		// We need to track the DAG to determine previous pass semaphores
		std::vector<std::shared_ptr<VulkanSemaphore>> waitingSemaphores;
		std::vector<VkPipelineStageFlags> waitingStages;
		std::vector<std::shared_ptr<VulkanSemaphore>> signalingSemaphores;
		int prevSyncPassIndex = i;
		for (int prevIdx = i - 1; prevIdx >= 0; --prevIdx) {
			if (!passes[prevIdx].isExternalSync()) {
				prevSyncPassIndex = prevIdx;
				break;
			}
		}
		if (prevSyncPassIndex != i) {
			waitingSemaphores.emplace_back(
				resources->perPassResources[prevSyncPassIndex].semaphore);
			waitingStages.emplace_back(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
		}

		waitingSemaphores.insert(waitingSemaphores.end(),
								 perPassResource.externalWaitingSemaphores.begin(),
								 perPassResource.externalWaitingSemaphores.end());
		waitingStages.insert(waitingStages.end(), perPassResource.externalWaitingStages.begin(),
							 perPassResource.externalWaitingStages.end());
		signalingSemaphores = perPassResource.externalSignalingSemaphores;
		signalingSemaphores.emplace_back(perPassResource.semaphore);
		const auto cmdBuffer = perPassResource.cmdBuffer;
		cmdBuffer->reset();
		cmdBuffer->beginCommandBuffer({VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
									   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr});
		pass.execute(device, *resources, cmdBuffer, waitingSemaphores);
		cmdBuffer->endCommandBuffer();
		if (!pass.isExternalSync()) {
			SubmitInfoContainer submitData;
			submitData.waitingSemaphores = std::move(waitingSemaphores);
			submitData.waitingStages = std::move(waitingStages);
			submitData.signalingSemaphores = std::move(signalingSemaphores);
			submitData.signalingFence = i == lastSyncPassIndex ? resources->fence : nullptr;
			cmdBuffer->submit(submitData);
		}
	}
}

std::shared_ptr<FGResourceList> FrameGraph::createResourceList(
	std::shared_ptr<VulkanCommandPool> graphicPool,
	std::shared_ptr<VulkanCommandPool> computePool,
	std::shared_ptr<VulkanCommandPool> rayTracingPool) const
{
	return std::shared_ptr<FGResourceList>(new FGResourceList{
		device, this, static_cast<int>(images.size()), static_cast<int>(buffers.size()),
		graphicPool, computePool, rayTracingPool});
}

const FGSubpass& FrameGraph::getSubpass(const std::string& name) const
{
	const auto index = nameToSubpassIndex.at(name);
	return subpasses[index];
}
