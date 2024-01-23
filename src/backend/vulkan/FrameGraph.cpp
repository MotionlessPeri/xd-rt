//
// Created by Frank on 2024/1/17.
//

#include "FrameGraph.h"

#include <functional>
#include <stack>
#include <stdexcept>
#include <unordered_set>

#include "VulkanDevice.h"
#include "VulkanImageView.h"
#include "VulkanRenderPass.h"
using namespace xd;
template <typename RefProviderType,
		  typename Traits = typename std::is_base_of<SubpassDescBase, RefProviderType>::type>
const VkAttachmentReference2& getAttachmentRef(const RefProviderType& refProvider,
											   const FrameGraphResourceHandle& from)
{
	if (from.inputIndex != FrameGraphResourceHandle::INVALID_INDEX) {
		return refProvider.inputRefs[from.inputIndex];
	}
	else if (from.colorIndex != FrameGraphResourceHandle::INVALID_INDEX) {
		return refProvider.colorRefs[from.colorIndex];
	}
	else if (from.depthIndex != FrameGraphResourceHandle::INVALID_INDEX) {
		return refProvider.depthStencilRefs[from.depthIndex];
	}
	else {
		throw std::runtime_error{"invalid from handle!\n"};
	}
}

FrameGraphResourceHandle FrameGraphBuilder::FrameGraphNode::addInput(

	VkAttachmentReference2&& ref,
	const FrameGraphResourceHandle& from,
	VkSubpassDependency2&& dep)
{
	addExistingResource(ref, from, dep, inputRefs);
	const FrameGraphResourceHandle to{index, (uint32_t)inputRefs.size() - 1,
									  FrameGraphResourceHandle::INVALID_INDEX,
									  FrameGraphResourceHandle::INVALID_INDEX};
	const ResourceTransitionEdge edge{from, to};
	recordEdges(edge);
	// trackPreserveAttaches(edge);
	return to;
}

FrameGraphResourceHandle FrameGraphBuilder::FrameGraphNode::addColorAttach(

	VkAttachmentReference2&& ref,
	const FrameGraphResourceHandle& from,
	VkSubpassDependency2&& dep)
{
	addExistingResource(ref, from, dep, colorRefs);
	const FrameGraphResourceHandle to{index, FrameGraphResourceHandle::INVALID_INDEX,
									  (uint32_t)colorRefs.size() - 1,
									  FrameGraphResourceHandle::INVALID_INDEX};
	const ResourceTransitionEdge edge{from, to};
	recordEdges(edge);
	// trackPreserveAttaches(edge);
	return to;
}

FrameGraphResourceHandle FrameGraphBuilder::FrameGraphNode::addColorAttach(

	VkAttachmentReference2&& ref,
	VkAttachmentDescription2&& desc,
	VkSubpassDependency2&& extDep)
{
	addNewResource(ref, desc, extDep, colorRefs);
	return {index, FrameGraphResourceHandle::INVALID_INDEX, (uint32_t)colorRefs.size() - 1,
			FrameGraphResourceHandle::INVALID_INDEX};
}

FrameGraphResourceHandle FrameGraphBuilder::FrameGraphNode::addDepthAttach(

	VkAttachmentReference2&& ref,
	const FrameGraphResourceHandle& from,
	VkSubpassDependency2&& dep)
{
	addExistingResource(ref, from, dep, depthStencilRefs);
	const FrameGraphResourceHandle to{index, FrameGraphResourceHandle::INVALID_INDEX,
									  FrameGraphResourceHandle::INVALID_INDEX,
									  (uint32_t)depthStencilRefs.size() - 1};
	const ResourceTransitionEdge edge{from, to};
	recordEdges(edge);
	// trackPreserveAttaches(edge);
	return to;
}

FrameGraphResourceHandle FrameGraphBuilder::FrameGraphNode::addDepthAttach(

	VkAttachmentReference2&& ref,
	VkAttachmentDescription2&& desc,
	VkSubpassDependency2&& extDep)
{
	addNewResource(ref, desc, extDep, depthStencilRefs);
	return {index, FrameGraphResourceHandle::INVALID_INDEX, FrameGraphResourceHandle::INVALID_INDEX,
			(uint32_t)depthStencilRefs.size() - 1};
}

void FrameGraphBuilder::FrameGraphNode::addNullDepth()
{
	depthStencilRefs.emplace_back(VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr,
								  VK_ATTACHMENT_UNUSED);
}

SubpassDesc FrameGraphBuilder::FrameGraphNode::getSubpassDescription()
{
	SubpassDesc subpassDesc;
	subpassDesc.inputRefs = std::move(inputRefs);
	subpassDesc.colorRefs = std::move(colorRefs);
	subpassDesc.depthStencilRefs = std::move(depthStencilRefs);
	subpassDesc.preserveAttaches = std::move(preserveAttaches);
	subpassDesc.desc.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
	subpassDesc.desc.pNext = nullptr;
	subpassDesc.desc.flags = 0;
	subpassDesc.desc.pipelineBindPoint = bindPoint;
	subpassDesc.desc.viewMask = 0;
	subpassDesc.desc.inputAttachmentCount = subpassDesc.inputRefs.size();
	subpassDesc.desc.pInputAttachments =
		subpassDesc.inputRefs.empty() ? nullptr : subpassDesc.inputRefs.data();
	subpassDesc.desc.colorAttachmentCount = subpassDesc.colorRefs.size();
	subpassDesc.desc.pColorAttachments =
		subpassDesc.colorRefs.empty() ? nullptr : subpassDesc.colorRefs.data();
	subpassDesc.desc.pResolveAttachments = nullptr;
	subpassDesc.desc.pDepthStencilAttachment =
		subpassDesc.depthStencilRefs.empty() ? nullptr : subpassDesc.depthStencilRefs.data();
	subpassDesc.desc.preserveAttachmentCount = subpassDesc.preserveAttaches.size();
	subpassDesc.desc.pPreserveAttachments =
		subpassDesc.preserveAttaches.empty() ? nullptr : subpassDesc.preserveAttaches.data();
	return subpassDesc;
}

void FrameGraphBuilder::FrameGraphNode::addExistingResource(
	VkAttachmentReference2& ref,
	const FrameGraphResourceHandle& from,
	VkSubpassDependency2& dep,
	std::vector<VkAttachmentReference2>& emplaceVec)
{
	const auto& nodes = owner->nodes;
	const auto attachmentIndex = getAttachmentRef(nodes[from.passIndex], from).attachment;
	ref.attachment = attachmentIndex;
	emplaceVec.emplace_back(ref);

	// update attachment's final layout
	owner->attachments[attachmentIndex].finalLayout = ref.layout;

	auto& deps = owner->dependencies;
	dep.srcSubpass = from.passIndex;
	dep.dstSubpass = index;
	deps.emplace_back(dep);
}

void FrameGraphBuilder::FrameGraphNode::addNewResource(
	VkAttachmentReference2& ref,
	VkAttachmentDescription2& desc,
	VkSubpassDependency2& dep,
	std::vector<VkAttachmentReference2>& emplaceVec)
{
	auto& attaches = owner->attachments;
	attaches.emplace_back(desc);
	firstDeclaredAttaches.insert(attaches.size() - 1);

	ref.attachment = attaches.size() - 1;
	emplaceVec.emplace_back(ref);

	auto& deps = owner->dependencies;
	dep.srcSubpass = VK_SUBPASS_EXTERNAL;
	dep.dstSubpass = index;
	deps.emplace_back(dep);
}

void FrameGraphBuilder::FrameGraphNode::recordEdges(const ResourceTransitionEdge& edge)
{
	ins.emplace_back(edge);
	owner->nodes[edge.from.passIndex].outs.emplace_back(edge);
}

FrameGraphBuilder::FrameGraphNode& FrameGraphBuilder::addSubpass(const std::string& name)
{
	nodes.emplace_back();
	auto& ret = nodes.back();
	ret.owner = this;
	ret.index = static_cast<uint32_t>(nodes.size() - 1);
	ret.name = name;
	return nodes.back();
}

std::shared_ptr<FrameGraph> FrameGraphBuilder::build(std::shared_ptr<VulkanDevice> device)
{
	// track all preserved attachments here
	for (auto& node : nodes) {
		for (const auto& edge : node.outs) {
			trackPreserveAttaches(edge);
		}
	}

	RenderPassDesc renderPassDesc;
	renderPassDesc.attachments = std::move(attachments);
	std::vector<VkSubpassDescription2> vkSubpasses;
	for (auto& subpass : nodes) {
		renderPassDesc.subpasses.push_back(subpass.getSubpassDescription());
		vkSubpasses.emplace_back(renderPassDesc.subpasses.back().desc);
	}
	renderPassDesc.dependencys = std::move(dependencies);

	renderPassDesc.ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
	renderPassDesc.ci.pNext = nullptr;
	renderPassDesc.ci.flags = 0;
	renderPassDesc.ci.attachmentCount = renderPassDesc.attachments.size();
	renderPassDesc.ci.pAttachments = renderPassDesc.attachments.data();
	renderPassDesc.ci.subpassCount = vkSubpasses.size();
	renderPassDesc.ci.pSubpasses = vkSubpasses.data();
	renderPassDesc.ci.dependencyCount = renderPassDesc.dependencys.size();
	renderPassDesc.ci.pDependencies =
		renderPassDesc.dependencys.empty() ? nullptr : renderPassDesc.dependencys.data();
	renderPassDesc.ci.correlatedViewMaskCount = 0;
	renderPassDesc.ci.pCorrelatedViewMasks = nullptr;
	const auto renderPass = device->createRenderPass(renderPassDesc);

	auto ret = std::make_shared<FrameGraph>(renderPass);
	const auto& subpasses = renderPass->getSubpasses();
	ret->subpasses.resize(nodes.size());  // make sure that vector does not re-allocate and move
	// note: we can use enumerate_view in c++23
	for (const auto i : std::views::iota(0ull, nodes.size())) {
		auto& node = nodes[i];
		ret->subpasses[i] = FrameGraphPass{ret, subpasses[i]};
		ret->subpasses[i].firstDeclaredAttaches = std::move(node.firstDeclaredAttaches);
		ret->subpassDict.insert({std::move(node.name), &ret->subpasses[i]});
	}
	ret->boundBuffers.resize(renderPassDesc.attachments.size());
	nodes.clear();
	return ret;
}

void FrameGraphBuilder::trackPreserveAttaches(const ResourceTransitionEdge& edge)
{
	const auto startIndex = edge.from.passIndex;
	const auto targetIndex = edge.to.passIndex;
	const auto attachmentIndex = getAttachmentRef(nodes[edge.from.passIndex], edge.from).attachment;
	std::vector<bool> visited(nodes.size(), false);
	visited[targetIndex] = true;
	std::unordered_set<uint32_t> tagged;
	tagged.insert(targetIndex);
	std::function<bool(uint32_t)> dfs{[&](uint32_t nodeIndex) -> bool {
		if (visited[nodeIndex])
			return tagged.contains(nodeIndex);
		const auto& edges = nodes[nodeIndex].outs;
		bool foundPath = false;
		for (const auto& edge : edges) {
			const auto& toNodeIndex = edge.to.passIndex;
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
			nodes[nodeIndex].preserveAttaches.emplace_back(attachmentIndex);
			tagged.insert(nodeIndex);
		}
		return foundPath;
	}};
	dfs(edge.from.passIndex);
}

void FrameGraphPass::bindAttachmentBuffer(FrameGraphResourceHandle pos,
										  std::shared_ptr<VulkanImageView> imageView)
{
	const auto& attachDesc = getAttachmentRef(subpass->desc, pos);
	const auto attachIndex = attachDesc.attachment;
	if (!firstDeclaredAttaches.contains(attachIndex)) {
		throw std::runtime_error{"Invalid pos\n"};
	}
	owner->boundBuffers[attachIndex] = imageView;
}

void FrameGraph::bindAttachmentBuffer(FrameGraphResourceHandle pos,
									  std::shared_ptr<VulkanImageView> imageView)
{
	auto& pass = subpasses[pos.passIndex];
	pass.bindAttachmentBuffer(pos, std::move(imageView));
}

void FrameGraph::buildFrameBuffer(uint32_t width, uint32_t height)
{
	const auto imageViewHandleView =
		boundBuffers | std::views::transform([](const auto& view) { return view->imageView; });
	const std::vector<VkImageView> attachmentHandles{imageViewHandleView.begin(),
													 imageViewHandleView.end()};
	VkFramebufferCreateInfo ci;
	ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	ci.pNext = nullptr;
	ci.flags = 0;
	ci.renderPass = renderPass->pass;
	ci.attachmentCount = attachmentHandles.size();
	ci.pAttachments = attachmentHandles.data();
	ci.width = width;
	ci.height = height;
	ci.layers = 1;
	frameBuffer = renderPass->createFrameBuffer(std::move(ci));
}
