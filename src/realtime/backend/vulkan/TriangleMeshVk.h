//
// Created by Frank on 2024/1/13.
//

#ifndef XD_RT_TRIANGLEMESHVK_H
#define XD_RT_TRIANGLEMESHVK_H
#include <memory>

#include "MathTypes.h"
#include "VulkanTypes.h"
#include "model/ModelTypes.h"
namespace xd {

class TriangleMeshVk {
public:
	friend class ModelFactoryVk;
	friend class VulkanGLFWAppBase;	 // TODO: remove it ASAP when FrameGraph is built
	TriangleMeshVk() = delete;
	TriangleMeshVk(const TriangleMeshVk& other) = delete;
	TriangleMeshVk(TriangleMeshVk&& other) noexcept = delete;
	TriangleMeshVk& operator=(const TriangleMeshVk& other) = delete;
	TriangleMeshVk& operator=(TriangleMeshVk&& other) noexcept = delete;
	uint32_t getIndexCount() const;
	void bind(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const;
	void draw(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const;

private:
	TriangleMeshVk(std::shared_ptr<TriangleMesh> ref,
				   std::shared_ptr<VulkanBuffer> positions,
				   std::shared_ptr<VulkanBuffer> uvs,
				   std::shared_ptr<VulkanBuffer> normals,
				   std::shared_ptr<VulkanBuffer> tangents,
				   std::shared_ptr<VulkanBuffer> indices);

	std::shared_ptr<TriangleMesh> ref = nullptr;
	std::shared_ptr<VulkanBuffer> positions, uvs, normals, tangents;
	std::shared_ptr<VulkanBuffer> indices;
};

}  // namespace xd

#endif	// XD_RT_TRIANGLEMESHVK_H
