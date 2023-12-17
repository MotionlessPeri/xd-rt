//
// Created by Frank on 2023/10/1.
//
#include "GLTFMeshLoader.h"
#include "3rdParty/tinygltf/TinyGLTFUtil.h"
#include "model/Triangle.h"
using namespace xd;
std::shared_ptr<TriangleMesh> GLTFMeshLoader::load(const std::string& path,
												   const LoadMeshOptions& options)
{
	return nullptr;
}
std::shared_ptr<TriangleMesh> GLTFMeshLoader::load(const tinygltf::Primitive& primitive,
												   const tinygltf::Model& model,
												   const LoadMeshOptions& options)
{
	if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
		return nullptr;
	std::vector<uint32_t> indices;
	std::vector<float> positions;
	std::vector<float> uvs;
	std::vector<float> normals;
	std::vector<float> tangents;

	{
		const auto& accessor = model.accessors[primitive.indices];
		const auto& bufferView = model.bufferViews[accessor.bufferView];
		const auto& buffer = model.buffers[bufferView.buffer];
		switch (accessor.componentType) {
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
				indices = gltfBufferToStdVec<uint8_t, uint32_t>(accessor, bufferView, buffer);
				break;
			}
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
				indices = gltfBufferToStdVec<uint16_t, uint32_t>(accessor, bufferView, buffer);
				break;
			}
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
				indices = gltfBufferToStdVec<uint32_t, uint32_t>(accessor, bufferView, buffer);
				break;
			}
		}
	}

	{
		const auto it = primitive.attributes.find("POSITION");
		if (it != primitive.attributes.end()) {
			const auto& accessor = model.accessors[it->second];
			const auto& bufferView = model.bufferViews[accessor.bufferView];
			const auto& buffer = model.buffers[bufferView.buffer];
			positions = gltfBufferToStdVec<float, float>(accessor, bufferView, buffer);
		}
		else {
			throw std::runtime_error{"No position attribute is not allowed.\n"};
		}
	}

	{
		const auto it = primitive.attributes.find("TEXCOORD_0");
		if (it != primitive.attributes.end()) {
			const auto& accessor = model.accessors[it->second];
			const auto& bufferView = model.bufferViews[accessor.bufferView];
			const auto& buffer = model.buffers[bufferView.buffer];
			assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
			uvs = gltfBufferToStdVec<float, float>(accessor, bufferView, buffer);
		}
	}

	{
		const auto it = primitive.attributes.find("NORMAL");
		if (it != primitive.attributes.end()) {
			const auto& accessor = model.accessors[it->second];
			const auto& bufferView = model.bufferViews[accessor.bufferView];
			const auto& buffer = model.buffers[bufferView.buffer];
			normals = gltfBufferToStdVec<float, float>(accessor, bufferView, buffer);
		}
	}

	{
		const auto it = primitive.attributes.find("TANGENT");
		if (it != primitive.attributes.end()) {
			const auto& accessor = model.accessors[it->second];
			const auto& bufferView = model.bufferViews[accessor.bufferView];
			const auto& buffer = model.buffers[bufferView.buffer];
			auto handnessTangents = gltfBufferToStdVec<float, float>(accessor, bufferView, buffer);
			// TODO: handle handness
			for (auto i = 0; i < handnessTangents.size(); i += 4) {
				tangents.emplace_back(handnessTangents[i]);
				tangents.emplace_back(handnessTangents[i + 1]);
				tangents.emplace_back(handnessTangents[i + 2]);
			}
		}
	}

	return std::make_shared<TriangleMesh>(positions, uvs, normals, tangents, indices);
}
