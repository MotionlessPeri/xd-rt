//
// Created by Frank on 2023/10/1.
//
#include "GLTFSceneLoader.h"
#include "GLTFMeshLoader.h"
#include "Primitive.h"
#include "Scene.h"
#include "SceneBuilder.h"
#include "filter/NearestFilter.h"
#include "filter/TentFilter.h"
#include "mapping/UVMapping.h"
#include "material/MatteMaterial.h"
#include "model/Triangle.h"
#include "texture/ImageTexture.h"
using namespace xd;

std::shared_ptr<Scene> GLTFSceneLoader::load(const std::string& path,
											 const LoadSceneOptions& options)
{
	SceneBuilder builder;
	loadToSceneBuilder(path, options, builder);
	return builder.build();
}

SceneBuilder GLTFSceneLoader::loadNodes(const tinygltf::Model& gltfModel,
										const std::vector<std::shared_ptr<Material>>& materials,
										SceneBuilder& sceneBuilder) const
{
	std::shared_ptr<Material> defaultMaterial =
		std::make_shared<MatteMaterial>(Vector3f{100, 100, 100});
	sceneBuilder.setHitSolverType(HitSolverType::EMBREE);
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
				sceneBuilder.addPrimitive(primitive);
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
	return sceneBuilder;
}

void GLTFSceneLoader::loadToSceneBuilder(const std::string& path,
										 const LoadSceneOptions& options,
										 SceneBuilder& sceneBuilder) const
{
	tinygltf::Model gltfModel;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, path.c_str());
	if (!ret) {
		throw std::runtime_error{"Load gltf failed!\n"};
	}

	auto materials = loadMaterials(gltfModel);

	loadNodes(gltfModel, materials, sceneBuilder);
}

std::vector<std::shared_ptr<Material>> GLTFSceneLoader::loadMaterials(
	tinygltf::Model& gltfModel) const
{
	std::vector<std::shared_ptr<Material>> materials;
	for (const auto& gltfMtl : gltfModel.materials) {
		// TODO: maybe image should be separated with underlying
		const std::shared_ptr<Texture> normal =
			gltfMtl.normalTexture.index == -1
				? nullptr
				: loadImageTexture(gltfModel, gltfModel.textures[gltfMtl.normalTexture.index],
								   false);
		const auto& pbr = gltfMtl.pbrMetallicRoughness;
		const auto baseColorTexture =
			loadTextureTemplate(gltfModel, gltfMtl.pbrMetallicRoughness.baseColorTexture.index,
								Vector3f{static_cast<float>(pbr.baseColorFactor[0]),
										 static_cast<float>(pbr.baseColorFactor[1]),
										 static_cast<float>(pbr.baseColorFactor[2])},
								true);
		materials.emplace_back(std::make_shared<MatteMaterial>(normal, baseColorTexture));
	}
	return materials;
}

std::shared_ptr<ImageFilter2D> GLTFSceneLoader::buildFilter(
	const tinygltf::Sampler& gltfSampler) const
{
	// TODO: because we have not implement mip-map yet, build the filter with respect to magFilter
	const auto tinyGltfWrapToWrapEnum = [](int wrap) -> WrapMode {
		switch (wrap) {
			case TINYGLTF_TEXTURE_WRAP_REPEAT:
				return WrapMode::REPEAT;
			case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
				return WrapMode::CLAMP;
			default:
				return WrapMode::UNKNOWN;
		}
	};
	switch (gltfSampler.magFilter) {
		case TINYGLTF_TEXTURE_FILTER_NEAREST: {
			const auto wrapS = tinyGltfWrapToWrapEnum(gltfSampler.wrapS);
			const auto wrapT = tinyGltfWrapToWrapEnum(gltfSampler.wrapT);
			return std::make_shared<NearestFilter>(wrapS, wrapT);
		}
		case TINYGLTF_TEXTURE_FILTER_LINEAR: {
			const auto wrapS = tinyGltfWrapToWrapEnum(gltfSampler.wrapS);
			const auto wrapT = tinyGltfWrapToWrapEnum(gltfSampler.wrapT);
			return std::make_shared<TentFilter>(wrapS, wrapT);
		}
		default: {
			return nullptr;
		}
	}
}

std::shared_ptr<Mapping2D> GLTFSceneLoader::buildMapping() const
{
	static const auto uvMapping = std::make_shared<UVMapping>();
	return uvMapping;
}

std::shared_ptr<Image2D> GLTFSceneLoader::buildImage(const tinygltf::Model& gltfModel,
													 const tinygltf::Image& gltfImage,
													 bool isSrgb) const
{
	std::vector<uint8_t> data;
	uint32_t stride = 0u;
	if (gltfImage.bufferView == -1) {
		assert(!gltfImage.uri.empty());
		// change gltfImage to non-const ref and move to data if necessary
		data = gltfImage.image;
		stride = 0u;
	}
	else {
		const auto& bufferView = gltfModel.bufferViews[gltfImage.bufferView];
		const auto& buffer = gltfModel.buffers[bufferView.buffer];
		if (gltfImage.as_is) {
			// TODO: image stored in buffer as file format, need to resolve it
			assert(false);
			return nullptr;
		}
		data = {buffer.data.data(), buffer.data.data() + bufferView.byteLength};
		stride = static_cast<uint32_t>(bufferView.byteStride);
	}

	const auto pixelFormat = getPixelFormat(gltfImage, isSrgb);
	return std::make_shared<Image2D>(pixelFormat, static_cast<uint32_t>(gltfImage.width),
									 static_cast<uint32_t>(gltfImage.height), stride,
									 std::move(data));
}

PixelFormat GLTFSceneLoader::getPixelFormat(const tinygltf::Image& image, bool isSrgb) const
{
	switch (image.pixel_type) {
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
			if (image.component == 3)
				return isSrgb ? PixelFormat::FORMAT_R8G8B8_SRGB : PixelFormat::FORMAT_R8G8B8_UNORM;
			if (image.component == 4)
				return isSrgb ? PixelFormat::FORMAT_R8G8B8A8_SRGB
							  : PixelFormat::FORMAT_R8G8B8A8_UNORM;
			return PixelFormat::FORMAT_UNKNOWN;
		}
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
			return PixelFormat::FORMAT_UNKNOWN;
		}
		case TINYGLTF_COMPONENT_TYPE_FLOAT: {
			if (image.component == 3)
				return PixelFormat::FORMAT_R32G32B32_SFLOAT;
			if (image.component == 4)
				return PixelFormat::FORMAT_R32G32B32A32_SFLOAT;
			return PixelFormat::FORMAT_UNKNOWN;
		}
		default:
			return PixelFormat::FORMAT_UNKNOWN;
	}
}
