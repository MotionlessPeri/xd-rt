//
// Created by Frank on 2024/1/13.
//

#include "TriangleMeshVk.h"

#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "model/Triangle.h"
using namespace xd;
uint32_t TriangleMeshVk::getIndexCount() const
{
	return ref->getIndices().size();
}

void TriangleMeshVk::bind(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const
{
	std::vector<VkBuffer> buffers;
	buffers.emplace_back(positions->buffer);
	buffers.emplace_back(uvs->buffer);
	buffers.emplace_back(normals->buffer);
	buffers.emplace_back(tangents->buffer);
	const std::vector<VkDeviceSize> offsets(4, 0u);
	cmdBuffer->bindVertexBuffers(0, buffers, offsets);
	cmdBuffer->bindIndexBuffer(indices->buffer, VK_INDEX_TYPE_UINT32);
}

void TriangleMeshVk::draw(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const
{
	cmdBuffer->drawIndexed(getIndexCount(), 1, 0, 0, 0);
}

TriangleMeshVk::TriangleMeshVk(std::shared_ptr<TriangleMesh> ref,
							   std::shared_ptr<VulkanBuffer> positions,
							   std::shared_ptr<VulkanBuffer> uvs,
							   std::shared_ptr<VulkanBuffer> normals,
							   std::shared_ptr<VulkanBuffer> tangents,
							   std::shared_ptr<VulkanBuffer> indices)
	: ref(std::move(ref)),
	  positions(std::move(positions)),
	  uvs(std::move(uvs)),
	  normals(std::move(normals)),
	  tangents(std::move(tangents)),
	  indices(std::move(indices))
{
}
