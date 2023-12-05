//
// Created by Frank on 2023/10/1.
//

#ifndef XD_RT_GLTFSCENELOADER_H
#define XD_RT_GLTFSCENELOADER_H
#include "SceneLoader.h"
#include "tiny_gltf.h"
namespace xd {
class GLTFTextureLoader {
public:
	// for external image loaded into Image::image
	std::shared_ptr<UVTextureF> loadTexture2DF(const tinygltf::Texture& texture,
											   const tinygltf::Image& image);
	// for internal image stored in buffer
	std::shared_ptr<UVTextureF> loadTexture2DF(const tinygltf::Texture& texture,
											   const tinygltf::Image& image,
											   const tinygltf::BufferView& view,
											   const tinygltf::Buffer& buffer);
	// for external image loaded into Image::image
	std::shared_ptr<UVTextureRGB> loadTexture2DRGB(const tinygltf::Texture& texture,
												   const tinygltf::Image& image);
	// for internal image stored in buffer
	std::shared_ptr<UVTextureRGB> loadTexture2DRGB(const tinygltf::Texture& texture,
												   const tinygltf::Image& image,
												   const tinygltf::BufferView& view,
												   const tinygltf::Buffer& buffer);
	// for external image loaded into Image::image
	std::shared_ptr<UVTextureRGBA> loadTexture2DRGBA(const tinygltf::Texture& texture,
													 const tinygltf::Image& image);
	// for internal image stored in buffer
	std::shared_ptr<UVTextureRGBA> loadTexture2DRGBA(const tinygltf::Texture& texture,
													 const tinygltf::Image& image,
													 const tinygltf::BufferView& view,
													 const tinygltf::Buffer& buffer);
	struct RGBALoadSplitRes {
		std::shared_ptr<UVTextureRGB> rgb;
		std::shared_ptr<UVTextureF> alpha;
	};
	RGBALoadSplitRes loadTexture2DRGBASplit(const tinygltf::Texture& texture,
											const tinygltf::Image& image);
	RGBALoadSplitRes loadTexture2DRGBASplit(const tinygltf::Texture& texture,
											const tinygltf::Image& image,
											const tinygltf::BufferView& view,
											const tinygltf::Buffer& buffer);

protected:
	std::vector<float> getImageData(const tinygltf::Texture& texture, const tinygltf::Image& image);
	struct RGBATextureSplitRes {
		std::vector<float> rgb;
		std::vector<float> alpha;
	};
	RGBATextureSplitRes splitRGBAImage(const tinygltf::Texture& texture,
									   const tinygltf::Image& image);
};

class GLTFMaterialLoader {
public:
	GLTFMaterialLoader(
		const std::shared_ptr<std::unordered_map<int, std::shared_ptr<Texture2DF>>>& floatTextures,
		const std::shared_ptr<std::unordered_map<int, std::shared_ptr<Texture2DRGB>>>& rgbTextures,
		const std::shared_ptr<std::unordered_map<int, std::shared_ptr<Texture2DRGBA>>>&
			rgbaTextures);
	std::shared_ptr<Material> loadMaterial(const tinygltf::Material& material);

protected:
	std::shared_ptr<std::unordered_map<int, std::shared_ptr<Texture2DF>>> floatTextures;
	std::shared_ptr<std::unordered_map<int, std::shared_ptr<Texture2DRGB>>> rgbTextures;
	std::shared_ptr<std::unordered_map<int, std::shared_ptr<Texture2DRGBA>>> rgbaTextures;
};
class GLTFSceneLoader : public SceneLoader {
public:
	std::shared_ptr<Scene> load(const std::string& path,
								const LoadSceneOptions& options = {}) override;
	void loadToSceneBuilder(const std::string& path,
							const LoadSceneOptions& options,
							SceneBuilder& sceneBuilder) const override;
	void loadTextures(
		const tinygltf::Model& gltfModel,
		std::shared_ptr<std::unordered_map<int, std::shared_ptr<Texture2DRGB>>>& RGBTextures,
		std::shared_ptr<std::unordered_map<int, std::shared_ptr<Texture2DRGBA>>>& RGBATextures,
		std::shared_ptr<std::unordered_map<int, std::shared_ptr<Texture2DF>>>& floatTextures) const;
	std::vector<std::shared_ptr<Material>> loadMaterials(
		tinygltf::Model& gltfModel,
		const std::shared_ptr<std::unordered_map<int, std::shared_ptr<Texture2DRGB>>>& RGBTextures,
		const std::shared_ptr<std::unordered_map<int, std::shared_ptr<Texture2DRGBA>>>&
			RGBATextures,
		const std::shared_ptr<std::unordered_map<int, std::shared_ptr<Texture2DF>>>& floatTextures)
		const;
	SceneBuilder loadNodes(const tinygltf::Model& gltfModel,
						   const std::vector<std::shared_ptr<Material>>& materials,
						   SceneBuilder& sceneBuilder) const;
};
}  // namespace xd
#endif	// XD_RT_GLTFSCENELOADER_H
