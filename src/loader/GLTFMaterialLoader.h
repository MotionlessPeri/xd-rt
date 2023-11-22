//
// Created by Frank on 2023/10/1.
//

#ifndef XD_RT_GLTFMATERIALLOADER_H
#define XD_RT_GLTFMATERIALLOADER_H
#include "MaterialLoader.h"
#include "tiny_gltf.h"
namespace xd {
class GLTFMaterialLoader : public MaterialLoader {
public:
	std::shared_ptr<Material> load(const std::string& path,
								   const LoadMaterialOptions& options) override;
	std::shared_ptr<Material> load(const tinygltf::Material& material,
								   const tinygltf::Model& model,
								   const LoadMaterialOptions& options);
};
}  // namespace xd
#endif	// XD_RT_GLTFMATERIALLOADER_H
