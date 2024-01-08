//
// Created by Frank on 2023/10/1.
//

#ifndef XD_RT_GLTFSCENELOADER_H
#define XD_RT_GLTFSCENELOADER_H
#include "Enums.h"
#include "Mapping.h"
#include "SceneLoader.h"
#include "filter/ImageFilter2D.h"
#include "texture/ConstantTexture.h"
#include "texture/ImageTexture.h"
#include "texture/TextureTypes.h"
#include "tiny_gltf.h"

namespace xd {
class GLTFSceneLoader : public SceneLoader {
public:
	std::shared_ptr<Scene> load(const std::string& path, const LoadSceneOptions& options) override;
	void loadToSceneBuilder(const std::string& path,
							const LoadSceneOptions& options,
							SceneBuilder& sceneBuilder) const override;

private:
	std::vector<std::shared_ptr<Material>> loadMaterials(tinygltf::Model& gltfModel) const;
	SceneBuilder loadNodes(const tinygltf::Model& gltfModel,
						   const std::vector<std::shared_ptr<Material>>& materials,
						   SceneBuilder& sceneBuilder) const;
	template <class DefaultType,
			  typename Traits = std::enable_if_t<std::is_same_v<DefaultType, float> ||
												 std::is_same_v<DefaultType, ColorRGB> ||
												 std::is_same_v<DefaultType, ColorRGBA>>>
	std::shared_ptr<Texture> loadTextureTemplate(const tinygltf::Model& gltfModel,
													uint32_t textureIndex,
													DefaultType defaultValue,
													bool isSrgb) const
	{
		if (textureIndex == -1) {
			return std::make_shared<ConstantTexture>(std::move(defaultValue));
		}
		else {
			return loadImageTexture(gltfModel, gltfModel.textures[textureIndex], isSrgb);
		}
	}
	std::shared_ptr<ImageTexture> loadImageTexture(const tinygltf::Model& gltfModel,
												   const tinygltf::Texture& gltfTexture,
												   bool isSrgb) const
	{
		std::shared_ptr<ImageTexture> res = nullptr;
		const auto& gltfSampler = gltfModel.samplers[gltfTexture.sampler];
		const auto& gltfImage = gltfModel.images[gltfTexture.source];
		const auto filter = buildFilter(gltfSampler);
		const auto mapping = buildMapping();
		const auto image = buildImage(gltfModel, gltfImage, isSrgb);
		res = std::make_shared<ImageTexture>(image, filter, mapping);
		return res;
	}
	std::shared_ptr<ImageFilter2D> buildFilter(const tinygltf::Sampler& gltfSampler) const;
	std::shared_ptr<Mapping2D> buildMapping() const;
	std::shared_ptr<Image2D> buildImage(const tinygltf::Model& gltfModel,
										const tinygltf::Image& gltfImage,
										bool isSrgb) const;
	PixelFormat getPixelFormat(const tinygltf::Image& image, bool isSrgb) const;
};
}  // namespace xd
#endif	// XD_RT_GLTFSCENELOADER_H
