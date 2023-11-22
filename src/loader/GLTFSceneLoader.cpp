//
// Created by Frank on 2023/10/1.
//
#include "GLTFSceneLoader.h"
#include "Texture.h"
using namespace xd;

std::shared_ptr<UVTextureF> GLTFTextureLoader::loadTexture2DF(const tinygltf::Texture& texture,
															  const tinygltf::Image& image)
{
	// Note: for now, tinygltf only support 8bit and 16bit unsigned int file
	// so we don't need to consider floating type image(exr, hdr, etc) for now
	// Note: the current implmentation of tinygltf cast 16 bit unsigned into 8-bit containers
	assert(image.component == 1);
	return std::make_shared<UVTextureF>(getImageData(texture, image), (uint32_t)image.width,
										(uint32_t)image.height);
}
std::shared_ptr<UVTextureF> GLTFTextureLoader::loadTexture2DF(const tinygltf::Texture& texture,
															  const tinygltf::Image& image,
															  const tinygltf::BufferView& view,
															  const tinygltf::Buffer& buffer)
{
	// TODO: implement me
	assert(false);
	return std::shared_ptr<UVTextureF>();
}
std::shared_ptr<UVTextureRGB> GLTFTextureLoader::loadTexture2DRGB(const tinygltf::Texture& texture,
																  const tinygltf::Image& image)
{
	// Note: for now, tinygltf only support 8bit and 16bit unsigned int file
	// so we don't need to consider floating type image(exr, hdr, etc) for now
	// Note: the current implmentation of tinygltf cast 16 bit unsigned into 8-bit containers
	assert(image.component == 3);
	return std::make_shared<UVTextureRGB>(getImageData(texture, image), (uint32_t)image.width,
										  (uint32_t)image.height);
}
std::shared_ptr<UVTextureRGB> GLTFTextureLoader::loadTexture2DRGB(const tinygltf::Texture& texture,
																  const tinygltf::Image& image,
																  const tinygltf::BufferView& view,
																  const tinygltf::Buffer& buffer)
{
	// TODO: implement me
	assert(false);
	return std::shared_ptr<UVTextureRGB>();
}
std::shared_ptr<UVTextureRGBA> GLTFTextureLoader::loadTexture2DRGBA(
	const tinygltf::Texture& texture,
	const tinygltf::Image& image)
{
	assert(image.component == 4);
	return std::make_shared<UVTextureRGBA>(getImageData(texture, image), (uint32_t)image.width,
										   (uint32_t)image.height);
}
std::shared_ptr<UVTextureRGBA> GLTFTextureLoader::loadTexture2DRGBA(
	const tinygltf::Texture& texture,
	const tinygltf::Image& image,
	const tinygltf::BufferView& view,
	const tinygltf::Buffer& buffer)
{
	// TODO: implement me
	assert(false);
	return std::shared_ptr<UVTextureRGBA>();
}

GLTFTextureLoader::RGBALoadSplitRes GLTFTextureLoader::loadTexture2DRGBASplit(
	const tinygltf::Texture& texture,
	const tinygltf::Image& image)
{
	auto splitRes = splitRGBAImage(texture, image);
	return {
		std::make_shared<UVTextureRGB>(splitRes.rgb, (uint32_t)image.width, (uint32_t)image.height),
		std::make_shared<UVTextureF>(splitRes.alpha, (uint32_t)image.width,
									 (uint32_t)image.height)};
}
GLTFTextureLoader::RGBALoadSplitRes GLTFTextureLoader::loadTexture2DRGBASplit(
	const tinygltf::Texture& texture,
	const tinygltf::Image& image,
	const tinygltf::BufferView& view,
	const tinygltf::Buffer& buffer)
{
	return GLTFTextureLoader::RGBALoadSplitRes();
}
std::vector<float> GLTFTextureLoader::getImageData(const tinygltf::Texture& texture,
												   const tinygltf::Image& image)
{
	std::vector<float> container;
	const auto containerSize = image.width * image.height * image.component;
	container.reserve(containerSize);
	switch (image.bits) {
		case 8: {
			for (const auto ch : image.image) {
				container.emplace_back((float)ch / 255.f);
			}
			break;
		}
		case 16: {
			const auto* ptr = reinterpret_cast<const uint16_t*>(image.image.data());
			for (auto i = 0u; i < containerSize; ++i) {
				container.emplace_back((float)ptr[i] / 65535.f);
			}
			break;
		}
		default: {
			throw std::runtime_error{"Unsupported image bits\n"};
		}
	}
	return container;
}
GLTFTextureLoader::RGBATextureSplitRes GLTFTextureLoader::splitRGBAImage(
	const tinygltf::Texture& texture,
	const tinygltf::Image& image)
{
	RGBATextureSplitRes res;
	const auto pixelSize = image.width * image.height;
	res.rgb.reserve(pixelSize * 3);
	res.alpha.reserve(pixelSize);
	switch (image.bits) {
		case 8: {
			constexpr float norm = 1.f / 255.f;
			for (auto pixelIdx = 0; pixelIdx < pixelSize; ++pixelIdx) {
				res.rgb.emplace_back((float)image.image[4 * pixelIdx] * norm);
				res.rgb.emplace_back((float)image.image[4 * pixelIdx + 1] * norm);
				res.rgb.emplace_back((float)image.image[4 * pixelIdx + 2] * norm);
				res.alpha.emplace_back((float)image.image[4 * pixelIdx + 3] * norm);
			}
			break;
		}
		case 16: {
			constexpr float norm = 1.f / 65535.f;
			const auto* ptr = reinterpret_cast<const uint16_t*>(image.image.data());
			for (auto pixelIdx = 0; pixelIdx < pixelSize; ++pixelIdx) {
				res.rgb.emplace_back((float)ptr[4 * pixelIdx] * norm);
				res.rgb.emplace_back((float)ptr[4 * pixelIdx + 1] * norm);
				res.rgb.emplace_back((float)ptr[4 * pixelIdx + 2] * norm);
				res.alpha.emplace_back((float)ptr[4 * pixelIdx + 3] * norm);
			}
			break;
		}
		default: {
			throw std::runtime_error{"Unsupported image bits\n"};
		}
	}
	return res;
}

#include "../core/Material.h"
GLTFMaterialLoader::GLTFMaterialLoader(
	const std::shared_ptr<std::unordered_map<int, std::shared_ptr<Texture2DF>>>& floatTextures,
	const std::shared_ptr<std::unordered_map<int, std::shared_ptr<Texture2DRGB>>>& rgbTextures,
	const std::shared_ptr<std::unordered_map<int, std::shared_ptr<Texture2DRGBA>>>& rgbaTextures)
	: floatTextures(floatTextures), rgbTextures(rgbTextures), rgbaTextures(rgbaTextures)
{
}
std::shared_ptr<Material> GLTFMaterialLoader::loadMaterial(const tinygltf::Material& material)
{
	// TODO: we create matte material for all. Fix me as soon as pbr material is implemented
	const auto& pbr = material.pbrMetallicRoughness;
	const auto textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
	std::shared_ptr<Material> ret = nullptr;
	if (textureIndex == -1) {
		ret = std::make_shared<MatteMaterial>(Vector3f{(float)pbr.baseColorFactor[0],
													   (float)pbr.baseColorFactor[1],
													   (float)pbr.baseColorFactor[2]});
	}
	else {
		auto texture = (*rgbTextures)[textureIndex];
		ret = std::make_shared<MatteMaterial>(texture);
	}
	return ret;
}

#include "../core/Primitive.h"
#include "../core/Scene.h"
#include "../core/Triangle.h"
#include "GLTFMeshLoader.h"
std::shared_ptr<Scene> GLTFSceneLoader::load(const std::string& path,
											 const LoadSceneOptions& options)
{
	tinygltf::Model gltfModel;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, path.c_str());
	if (!ret) {
		throw std::runtime_error{"Load gltf failed!\n"};
	}
	auto RGBTextures = std::make_shared<std::unordered_map<int, std::shared_ptr<Texture2DRGB>>>();
	auto RGBATextures = std::make_shared<std::unordered_map<int, std::shared_ptr<Texture2DRGBA>>>();
	auto floatTextures = std::make_shared<std::unordered_map<int, std::shared_ptr<Texture2DF>>>();
	GLTFTextureLoader textureLoader;

	for (int i = 0; i < gltfModel.textures.size(); ++i) {
		const auto& gltfTexture = gltfModel.textures[i];
		const auto& image = gltfModel.images[gltfTexture.source];
		switch (image.component) {
			case 1: {
				const auto texture = textureLoader.loadTexture2DF(gltfTexture, image);
				floatTextures->insert({i, texture});
				break;
			}
			case 3: {
				const auto texture = textureLoader.loadTexture2DRGB(gltfTexture, image);
				RGBTextures->insert({i, texture});
				break;
			}
			case 4: {
				const auto textures = textureLoader.loadTexture2DRGBASplit(gltfTexture, image);
				floatTextures->insert({i, textures.alpha});
				RGBTextures->insert({i, textures.rgb});
				break;
			}
			default: {
				throw std::runtime_error{"Unsupported image component!\n"};
			}
		}
	}
	GLTFMaterialLoader materialLoader{floatTextures, RGBTextures, RGBATextures};
	std::vector<std::shared_ptr<Material>> materials;
	for (const auto& material : gltfModel.materials) {
		materials.emplace_back(materialLoader.loadMaterial(material));
	}

	std::shared_ptr<Material> defaultMaterial =
		std::make_shared<MatteMaterial>(Vector3f{100, 100, 100});

	auto scene = std::make_shared<Scene>();
	struct PairHasher {
		std::size_t operator()(const std::pair<uint32_t, uint32_t>& obj) const noexcept
		{
			std::size_t seed = 0x0A5AB277;
			seed ^= (seed << 6) + (seed >> 2) + 0x2FAE8CDF + static_cast<std::size_t>(obj.first);
			seed ^= (seed << 6) + (seed >> 2) + 0x71730EED + static_cast<std::size_t>(obj.second);
			return seed;
		}
	};
	std::unordered_map<std::pair<uint32_t, uint32_t>, std::shared_ptr<TriangleMesh>, PairHasher>
		meshCache;
	GLTFMeshLoader meshLoader;
	std::function<void(const tinygltf::Node&, const Transform&)> loadNodes =
		[&](const tinygltf::Node& gltfNode, const Transform& ancestorTransform) -> void {
		Transform transform = ancestorTransform;
		if (gltfNode.matrix.size() == 16) {
			Matrix4f mat;
			for (auto row = 0; row < 4; ++row) {
				for (auto col = 0; col < 4; ++col) {
					mat(row, col) = (float)gltfNode.matrix[col * 4 + row];
				}
			}
			transform *= mat;
		}
		else {
			if (gltfNode.translation.size() == 3) {
				transform *= Eigen::Translation3f{(float)gltfNode.translation[0],
												  (float)gltfNode.translation[1],
												  (float)gltfNode.translation[2]};
			}
			if (gltfNode.rotation.size() == 4) {
				transform *= Eigen::Quaternion<float>{
					(float)gltfNode.rotation[3], (float)gltfNode.rotation[0],
					(float)gltfNode.rotation[1], (float)gltfNode.rotation[2]};
			}
			if (gltfNode.scale.size() == 3) {
				transform *= Eigen::Scaling((float)gltfNode.scale[0], (float)gltfNode.scale[1],
											(float)gltfNode.scale[2]);
			}
		}
		if (gltfNode.mesh > -1) {
			const auto& gltfMesh = gltfModel.meshes[gltfNode.mesh];
			for (auto i = 0u; i < gltfMesh.primitives.size(); ++i) {
				const auto& gltfPrim = gltfMesh.primitives[i];
				const std::pair<uint32_t, uint32_t> meshKey{gltfNode.mesh, i};
				const auto findIter = meshCache.find(meshKey);
				std::shared_ptr<TriangleMesh> mesh = nullptr;
				if (findIter != meshCache.end()) {
					mesh = findIter->second;
				}
				else {
					mesh = meshLoader.load(gltfPrim, gltfModel, {});
					meshCache.insert({meshKey, mesh});
				}
				std::shared_ptr<Material> material = nullptr;
				if (gltfPrim.material > -1) {
					material = materials[gltfPrim.material];
				}
				else {
					// fall back to default material
					material = defaultMaterial;
				}
				const auto primitive = std::make_shared<Primitive>(mesh, material, transform);
				scene->addPrimitive(primitive);
			}
			for (const auto& gltfPrim : gltfMesh.primitives) {
				// TODO: we need a cache for load mesh(std::unordered_map<index,
				// shared_ptr<TriangleMesh>>)
			}
		}

		for (const auto childIdx : gltfNode.children) {
			loadNodes(gltfModel.nodes[childIdx], transform);
		}
	};
	for (const auto rootNodeIdx : gltfModel.scenes[0].nodes) {
		loadNodes(gltfModel.nodes[rootNodeIdx], Transform::Identity());
	}

	// TODO: handle lights
	return scene;
}
