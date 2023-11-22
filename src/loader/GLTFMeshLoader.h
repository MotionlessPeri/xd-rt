//
// Created by Frank on 2023/10/1.
//

#ifndef XD_RT_GLTFMESHLOADER_H
#define XD_RT_GLTFMESHLOADER_H
#include "MeshLoader.h"
#include "tiny_gltf.h"
namespace xd {
class GLTFMeshLoader : public MeshLoader {
public:
	std::shared_ptr<TriangleMesh> load(const std::string& path,
									   const LoadMeshOptions& options) override;
	std::shared_ptr<TriangleMesh> load(const tinygltf::Primitive& primitive,
									   const tinygltf::Model& model,
									   const LoadMeshOptions& options);
};
}  // namespace xd
#endif	// XD_RT_GLTFMESHLOADER_H
