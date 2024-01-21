//
// Created by Frank on 2024/1/17.
//

#include "FrameGraph.h"

#include <functional>
#include <stack>
#include <stdexcept>
#include <unordered_set>

#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
using namespace xd;

const VkAttachmentReference2& getAttachmentRef(
	const std::vector<FrameGraphBuilder::FrameGraphNode>& nodes,
	const FrameGraphBuilder::FrameGraphResourceHandle& from)
{
	const auto& fromNode = nodes[from.passIndex];
	if (from.inputIndex != FrameGraphBuilder::INVALID_INDEX) {
		return fromNode.inputRefs[from.inputIndex];
	}
	else if (from.colorIndex != FrameGraphBuilder::INVALID_INDEX) {
		return fromNode.colorRefs[from.colorIndex];
	}
	else if (from.depthIndex != FrameGraphBuilder::INVALID_INDEX) {
		return fromNode.depthRefs[from.depthIndex];
	}
	else {
		throw std::runtime_error{"invalid from handle!\n"};
	}
}

FrameGraphBuilder::FrameGraphResourceHandle FrameGraphBuilder::FrameGraphNode::addInput(
	VkAttachmentReference2&& ref,
	const FrameGraphResourceHandle& from,
	VkSubpassDependency2&& dep)
{
	addExistingResource(ref, from, dep, inputRefs);
	const FrameGraphResourceHandle to{index, (uint32_t)inputRefs.size() - 1, INVALID_INDEX,
									  INVALID_INDEX};
	const ResourceTransitionEdge edge{from, to};
	recordEdges(edge);
	// trackPreserveAttaches(edge);
	return to;
}

FrameGraphBuilder::FrameGraphResourceHandle FrameGraphBuilder::FrameGraphNode::addInput(
	VkAttachmentReference2&& ref,
	VkAttachmentDescription2&& desc,
	VkSubpassDependency2&& extDep)
{
	addNewResource(ref, desc, extDep, inputRefs);
	return {index, (uint32_t)inputRefs.size() - 1, INVALID_INDEX, INVALID_INDEX};
}

FrameGraphBuilder::FrameGraphResourceHandle FrameGraphBuilder::FrameGraphNode::addColorAttach(
	VkAttachmentReference2&& ref,
	const FrameGraphResourceHandle& from,
	VkSubpassDependency2&& dep)
{
	addExistingResource(ref, from, dep, colorRefs);
	const FrameGraphResourceHandle to{index, INVALID_INDEX, (uint32_t)colorRefs.size() - 1,
									  INVALID_INDEX};
	const ResourceTransitionEdge edge{from, to};
	recordEdges(edge);
	// trackPreserveAttaches(edge);
	return to;
}

FrameGraphBuilder::FrameGraphResourceHandle FrameGraphBuilder::FrameGraphNode::addColorAttach(
	VkAttachmentReference2&& ref,
	VkAttachmentDescription2&& desc,
	VkSubpassDependency2&& extDep)
{
	addNewResource(ref, desc, extDep, colorRefs);
	return {index, INVALID_INDEX, (uint32_t)colorRefs.size() - 1, INVALID_INDEX};
}

FrameGraphBuilder::FrameGraphResourceHandle FrameGraphBuilder::FrameGraphNode::addDepthAttach(
	VkAttachmentReference2&& ref,
	const FrameGraphResourceHandle& from,
	VkSubpassDependency2&& dep)
{
	addExistingResource(ref, from, dep, depthRefs);
	const FrameGraphResourceHandle to{index, INVALID_INDEX, INVALID_INDEX,
									  (uint32_t)depthRefs.size() - 1};
	const ResourceTransitionEdge edge{from, to};
	recordEdges(edge);
	// trackPreserveAttaches(edge);
	return to;
}

FrameGraphBuilder::FrameGraphResourceHandle FrameGraphBuilder::FrameGraphNode::addDepthAttach(
	VkAttachmentReference2&& ref,
	VkAttachmentDescription2&& desc,
	VkSubpassDependency2&& extDep)
{
	addNewResource(ref, desc, extDep, depthRefs);
	return {index, INVALID_INDEX, INVALID_INDEX, (uint32_t)depthRefs.size() - 1};
}

void FrameGraphBuilder::FrameGraphNode::addNullDepth()
{
	depthRefs.emplace_back(VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr, VK_ATTACHMENT_UNUSED);
}

void FrameGraphBuilder::FrameGraphNode::addExistingResource(
	VkAttachmentReference2& ref,
	const FrameGraphResourceHandle& from,
	VkSubpassDependency2& dep,
	std::vector<VkAttachmentReference2>& emplaceVec) const
{
	const auto& nodes = owner->nodes;
	const auto attachmentIndex = getAttachmentRef(nodes, from).attachment;
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
	std::vector<VkAttachmentReference2>& emplaceVec) const
{
	auto& attaches = owner->attachments;
	attaches.emplace_back(desc);

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
	nodes.clear();
	return std::make_shared<FrameGraph>(renderPass);
}

void FrameGraphBuilder::trackPreserveAttaches(const ResourceTransitionEdge& edge)
{
	const auto startIndex = edge.from.passIndex;
	const auto targetIndex = edge.to.passIndex;
	const auto attachmentIndex = getAttachmentRef(nodes, edge.from).attachment;
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
