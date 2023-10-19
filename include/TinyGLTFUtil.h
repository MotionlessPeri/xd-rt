//
// Created by Frank on 2023/10/1.
//

#ifndef XD_RT_TINYGLTFUTIL_H
#define XD_RT_TINYGLTFUTIL_H
#include "tiny_gltf.h"
namespace xd {
template <typename PtrType>
const PtrType* getBufferPointer(const tinygltf::Accessor& accessor,
								const tinygltf::BufferView& view,
								const tinygltf::Buffer& buffer)
{
	return reinterpret_cast<const PtrType*>(&(buffer.data[accessor.byteOffset + view.byteOffset]));
}
inline uint32_t getNumConmponents(const uint32_t ty)
{
	if (ty == TINYGLTF_TYPE_SCALAR) {
		return 1;
	}
	else if (ty == TINYGLTF_TYPE_VEC2) {
		return 2;
	}
	else if (ty == TINYGLTF_TYPE_VEC3) {
		return 3;
	}
	else if (ty == TINYGLTF_TYPE_VEC4) {
		return 4;
	}
	else if (ty == TINYGLTF_TYPE_MAT2) {
		return 4;
	}
	else if (ty == TINYGLTF_TYPE_MAT3) {
		return 9;
	}
	else if (ty == TINYGLTF_TYPE_MAT4) {
		return 16;
	}
	else {
		// Unknown componenty type
		return -1;
	}
}
inline uint32_t getComponentSizeInBytes(uint32_t componentType)
{
	if (componentType == TINYGLTF_COMPONENT_TYPE_BYTE) {
		return 1;
	}
	else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
		return 1;
	}
	else if (componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
		return 2;
	}
	else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
		return 2;
	}
	else if (componentType == TINYGLTF_COMPONENT_TYPE_INT) {
		return 4;
	}
	else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
		return 4;
	}
	else if (componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
		return 4;
	}
	else if (componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
		return 8;
	}
	else {
		// Unknown componenty type
		return -1;
	}
}
template <typename ComponentType, typename ContainerCompType>
std::vector<ContainerCompType> gltfBufferToStdVec(const tinygltf::Accessor& accessor,
												  const tinygltf::BufferView& bufferView,
												  const tinygltf::Buffer& buffer)
{
	const auto* rawBuffer = getBufferPointer<ComponentType>(accessor, bufferView, buffer);
	const auto numComp = getNumConmponents(accessor.type);
	const auto compSizeInByte = getComponentSizeInBytes(accessor.componentType);
	assert(sizeof(ComponentType) == compSizeInByte);
	const bool isTightlyPacked = bufferView.byteStride == 0 || (compSizeInByte * numComp == bufferView.byteStride);
	if (isTightlyPacked) {
		if constexpr (std::is_same<ComponentType, ContainerCompType>::value)
			return {rawBuffer, rawBuffer + accessor.count * numComp};
		else {
			std::vector<ContainerCompType> container;
			for (auto i = 0u; i < accessor.count * numComp; ++i) {
				container.push_back((ContainerCompType)rawBuffer[i]);
			}
			return container;
		}
	}
	else {
		std::vector<ContainerCompType> container;
		for (auto i = 0u; i < accessor.count; ++i) {
			for (auto j = 0u; j < numComp; ++j) {
				container.push_back((ContainerCompType)rawBuffer[j]);
			}

			rawBuffer = reinterpret_cast<const ComponentType*>(
				reinterpret_cast<const uint8_t*>(rawBuffer) + bufferView.byteStride);
		}
		return container;
	}
}
}  // namespace xd
#endif	// XD_RT_TINYGLTFUTIL_H
