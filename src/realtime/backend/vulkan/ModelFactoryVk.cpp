//
// Created by Frank on 2024/1/13.
//

#include "ModelFactoryVk.h"

#include "TriangleMeshVk.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "glm/glm.hpp"
#include "model/Triangle.h"
using namespace xd;
ModelFactoryVk* ModelFactoryVk::singleton = nullptr;
void ModelFactoryVk::init(std::shared_ptr<VulkanDevice> device)
{
	singleton = new ModelFactoryVk{std::move(device)};
}

void ModelFactoryVk::terminate()
{
	delete singleton;
}

std::shared_ptr<TriangleMeshVk> ModelFactoryVk::buildTriangleMesh(
	std::shared_ptr<TriangleMesh> mesh,
	BuildTriangleMeshOptions options) const
{
	std::shared_ptr<VulkanBuffer> posBuffer{nullptr}, uvBuffer{nullptr}, nBuffer{nullptr},
		tBuffer{nullptr};
	std::shared_ptr<VulkanBuffer> indexBuffer{nullptr};
	const auto& pos = mesh->getPositions();
	const long long vCount = pos.size() / 3ll;
	const auto buildBuffer = [&](VkBufferUsageFlags usage, uint32_t size) {
		VkBufferCreateInfo ci;
		ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = 0;
		ci.size = size;
		ci.usage = usage;
		ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		return device->createBuffer(ci, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	};
	{
		// build positions
		const auto size = sizeof(pos[0]) * pos.size();
		posBuffer =
			buildBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size);
		posBuffer->setData(0, (void*)pos.data(), size);
	}

	{
		// build uvs
		const auto size = sizeof(float) * vCount * 2;
		uvBuffer =
			buildBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size);
		if (mesh->hasUV()) {
			const auto& uvs = mesh->getUVs();
			uvBuffer->setData(0, (void*)uvs.data(), size);
		}
		else {
			switch (options.fallbackUVType) {
				case UVType::Zero: {
					std::vector<float> zeros(vCount * 2, 0.f);
					uvBuffer->setData(0, (void*)zeros.data(), size);
					break;
				}
				default:
					break;
			}
		}
	}

	std::vector<float> geomNormals;
	const std::vector<float>* normalVecPtr = nullptr;
	{
		// build normals
		const auto size = sizeof(float) * vCount * 3;
		nBuffer =
			buildBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size);
		if (mesh->hasNormal()) {
			const auto& normals = mesh->getNormals();
			normalVecPtr = &normals;
		}
		else {
			// using geom normal instead

			geomNormals = std::vector<float>(vCount * 3, 0.f);
			Eigen::Map<Eigen::Matrix3Xf> accessor{geomNormals.data(), 3ll, vCount};
			for (const auto& triangle : mesh->getTriangles()) {
				const auto gNormal = triangle.getGeomNormal();
				const auto indices = triangle.getIndices();
				for (const auto index : indices) {
					accessor.col(index) += gNormal;
				}
			}
			for (const auto i : std::views::iota(0ll, vCount)) {
				accessor.col(i).normalize();
			}
			normalVecPtr = &geomNormals;
		}
		nBuffer->setData(0, (void*)normalVecPtr->data(), size);
	}

	{
		// build tangents
		const auto size = sizeof(float) * vCount * 3;
		tBuffer =
			buildBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size);
		const std::vector<float>* tangentVecPtr = nullptr;
		std::vector<float> geomTangents;

		if (mesh->hasTangent()) {
			tangentVecPtr = &mesh->getTangents();
		}
		else {
			// build tangent via normals
			geomTangents = std::vector<float>(vCount * 3, 0.f);
			Eigen::Map<Eigen::Matrix3Xf> tangentAcc{geomTangents.data(), 3ll, vCount};
			Eigen::Map<const Eigen::Matrix3Xf> normalAcc{normalVecPtr->data(), 3ll, vCount};
			for (const auto i : std::views::iota(0ll, vCount)) {
				const Vector3f normal = normalAcc.col(i);
				const auto [x, y] = coordSystem(normal, true);
				tangentAcc.col(i) = x;
			}
			tangentVecPtr = &geomTangents;
		}
		tBuffer->setData(0u, (void*)tangentVecPtr->data(), size);
	}

	{
		// build indices
		const auto& indices = mesh->getIndices();
		const auto size = sizeof(indices[0]) * indices.size();
		indexBuffer =
			buildBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, size);
		indexBuffer->setData(0, (void*)indices.data(), size);
	}
	return std::shared_ptr<TriangleMeshVk>{
		new TriangleMeshVk{mesh, posBuffer, uvBuffer, nBuffer, tBuffer, indexBuffer}};
}

ModelFactoryVk::ModelFactoryVk(std::shared_ptr<VulkanDevice> device) : device(std::move(device)) {}
