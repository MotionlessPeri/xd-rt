//
// Created by Frank on 2023/10/1.
//
#include "GLTFMaterialLoader.h"
using namespace xd;
std::shared_ptr<Material> GLTFMaterialLoader::load(const std::string& path,
												   const LoadMaterialOptions& options)
{
	return nullptr;
}
std::shared_ptr<Material> GLTFMaterialLoader::load(const tinygltf::Material& material,
												   const tinygltf::Model& model,
												   const LoadMaterialOptions& options)
{
	return std::shared_ptr<Material>();
}
