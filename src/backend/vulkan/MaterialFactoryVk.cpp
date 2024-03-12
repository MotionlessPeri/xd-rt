//
// Created by Frank on 2024/1/24.
//

#include "MaterialFactoryVk.h"
#include <fstream>
#include <ranges>
#include <string>
#include "MaterialTemplateVk.h"
#include "ModelFactoryVk.h"
#include "VulkanComputePipeline.h"
#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"
using namespace xd;
MaterialFactoryVk* MaterialFactoryVk::singleton = nullptr;

void MaterialFactoryVk::init(std::shared_ptr<VulkanDevice> device)
{
	singleton = new MaterialFactoryVk{std::move(device)};
}

void MaterialFactoryVk::terminate()
{
	delete singleton;
}

std::shared_ptr<MaterialTemplateVk> MaterialFactoryVk::createLambertianMaterial(
	std::shared_ptr<VulkanSubpass> subpass) const
{
	// create shaders
	std::vector<std::shared_ptr<VulkanShader>> shaders;
	using namespace std::string_literals;
	shaders.emplace_back(createShader(PROJECT_ROOT + "/src/backend/vulkan/shader/main.vert.spv"s,
									  VK_SHADER_STAGE_VERTEX_BIT));
	shaders.emplace_back(
		createShader(PROJECT_ROOT + "/src/backend/vulkan/shader/lambertian.frag.spv"s,
					 VK_SHADER_STAGE_FRAGMENT_BIT));
	// declare all desc layouts
	std::unordered_map<std::string, uint32_t> layoutNameToIndex;
	std::vector<std::shared_ptr<VulkanDescriptorSetLayout>> layouts;
	// TODO: scene ubo and light ubo should be declared by other components(scene and light
	// manager), but they're not implemented yet. So we put them here temporarily
	{
		// declare scene ubo(set 0)
		DescriptorSetLayoutDesc layoutDesc;
		layoutDesc.bindings = std::vector<VkDescriptorSetLayoutBinding>{
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
			 VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}  // SceneUbos
		};
		layoutDesc.ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutDesc.ci.pNext = nullptr;
		layoutDesc.ci.flags = 0;
		layoutDesc.ci.bindingCount = layoutDesc.bindings.size();
		layoutDesc.ci.pBindings = layoutDesc.bindings.data();
		std::unordered_map<std::string, uint32_t> nameToBinding{{"sceneUbos", 0}};
		layoutNameToIndex.insert({"Scene", 0});
		layouts.emplace_back(
			device->createDescriptorSetLayout(layoutDesc, std::move(nameToBinding)));
	}
	{
		// declare material ubo(set 1)
		DescriptorSetLayoutDesc layoutDesc;
		layoutDesc.bindings = std::vector<VkDescriptorSetLayoutBinding>{
			{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
			 nullptr},	// diffuseMap
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
			 nullptr}  // normalMap
		};
		layoutDesc.ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutDesc.ci.pNext = nullptr;
		layoutDesc.ci.flags = 0;
		layoutDesc.ci.bindingCount = layoutDesc.bindings.size();
		layoutDesc.ci.pBindings = layoutDesc.bindings.data();
		std::unordered_map<std::string, uint32_t> nameToBinding{{"diffuseMap", 0},
																{"normalMap", 1}};
		layoutNameToIndex.insert({"Material", 1});
		layouts.emplace_back(
			device->createDescriptorSetLayout(layoutDesc, std::move(nameToBinding)));
	}
	{
		// declare light ubo(set 2)
		DescriptorSetLayoutDesc layoutDesc;
		layoutDesc.bindings = std::vector<VkDescriptorSetLayoutBinding>{
			{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
			 nullptr},	// LightIndexes
			{1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
			 nullptr}  // PointLightInfos
		};
		layoutDesc.ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutDesc.ci.pNext = nullptr;
		layoutDesc.ci.flags = 0;
		layoutDesc.ci.bindingCount = layoutDesc.bindings.size();
		layoutDesc.ci.pBindings = layoutDesc.bindings.data();
		std::unordered_map<std::string, uint32_t> nameToBinding{{"lightIndexes", 0},
																{"pointLightInfos", 1}};
		layoutNameToIndex.insert({"Light", 2});
		layouts.emplace_back(
			device->createDescriptorSetLayout(layoutDesc, std::move(nameToBinding)));
	}
	// build pipeline
	// build pipeline layout
	PipelineLayoutDesc layoutDesc;
	layoutDesc.setLayouts = std::move(layouts);
	layoutDesc.ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutDesc.ci.pNext = nullptr;
	layoutDesc.ci.flags = 0;
	layoutDesc.ci.pushConstantRangeCount = 0;
	layoutDesc.ci.pPushConstantRanges = nullptr;
	const auto pipelineLayout = device->createPipelineLayout(std::move(layoutDesc));
	GraphicsPipelineDesc pipelineDesc;
	pipelineDesc.dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(pipelineDesc.dynamicStates.size());
	dynamicState.pDynamicStates = pipelineDesc.dynamicStates.data();

	const auto vertexInputInfo = ModelFactoryVk::get().getVertexInputState();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// dynamic viewport and scissor state
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;	// Optional
	rasterizer.depthBiasClamp = 0.0f;			// Optional
	rasterizer.depthBiasSlopeFactor = 0.0f;		// Optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;			 // Optional
	multisampling.pSampleMask = nullptr;			 // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE;	 // Optional
	multisampling.alphaToOneEnable = VK_FALSE;		 // Optional

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;	 // Optional
	depthStencil.maxDepthBounds = 1.0f;	 // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};  // Optional
	depthStencil.back = {};	  // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
										  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	// over op
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	pipelineDesc.colorBlendStates.emplace_back(colorBlendAttachment);

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;  // Optional
	colorBlending.attachmentCount = pipelineDesc.colorBlendStates.size();
	colorBlending.pAttachments = pipelineDesc.colorBlendStates.data();

	const auto shaderStageView = shaders | std::views::transform([](const auto& shaderPtr) {
									 return shaderPtr->getShaderStageCi();
								 });
	pipelineDesc.shaderStages = {shaderStageView.begin(), shaderStageView.end()};

	pipelineDesc.ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineDesc.ci.pNext = nullptr;
	pipelineDesc.ci.flags = 0;
	pipelineDesc.ci.stageCount = pipelineDesc.shaderStages.size();
	pipelineDesc.ci.pStages = pipelineDesc.shaderStages.data();

	pipelineDesc.ci.pVertexInputState = &vertexInputInfo.ci;
	pipelineDesc.ci.pInputAssemblyState = &inputAssembly;
	pipelineDesc.ci.pViewportState = &viewportState;
	pipelineDesc.ci.pRasterizationState = &rasterizer;
	pipelineDesc.ci.pMultisampleState = &multisampling;
	pipelineDesc.ci.pDepthStencilState = &depthStencil;
	pipelineDesc.ci.pColorBlendState = &colorBlending;
	pipelineDesc.ci.pDynamicState = &dynamicState;
	// TODO: we might need to use render pass compability here. Getting render pass from some static
	// places but not pass it in.
	// TODO: We might need a RenderPassTraits class for registering render pass compability
	const auto pipeline = subpass->createGraphicsPipeline(std::move(pipelineDesc), pipelineLayout);
	return std::make_shared<MaterialTemplateVk>(device, std::move(shaders),
												std::move(layoutNameToIndex), pipeline);
}

std::shared_ptr<MaterialTemplateVk> MaterialFactoryVk::createTonemappingMaterial() const
{
	using namespace std::string_literals;
	const auto computeShader =
		createShader(PROJECT_ROOT + "/src/backend/vulkan/shader/tonemapping.comp.spv"s,
					 VK_SHADER_STAGE_COMPUTE_BIT);

	std::vector<std::shared_ptr<VulkanDescriptorSetLayout>> layouts;
	std::unordered_map<std::string, uint32_t> layoutNameToIndex;
	{
		// declare imageInfo(set 0)
		DescriptorSetLayoutDesc layoutDesc;
		layoutDesc.bindings = std::vector<VkDescriptorSetLayoutBinding>{
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
		layoutDesc.ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutDesc.ci.pNext = nullptr;
		layoutDesc.ci.flags = 0;
		layoutDesc.ci.bindingCount = layoutDesc.bindings.size();
		layoutDesc.ci.pBindings = layoutDesc.bindings.data();
		std::unordered_map<std::string, uint32_t> nameToBinding{{"imageInfo", 0}};
		layoutNameToIndex.insert({"Uniform", 0});
		layouts.emplace_back(
			device->createDescriptorSetLayout(layoutDesc, std::move(nameToBinding)));
	}
	{
		// declare input and output(set 1)
		DescriptorSetLayoutDesc layoutDesc;
		layoutDesc.bindings = std::vector<VkDescriptorSetLayoutBinding>{
			{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
			{1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
		layoutDesc.ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutDesc.ci.pNext = nullptr;
		layoutDesc.ci.flags = 0;
		layoutDesc.ci.bindingCount = layoutDesc.bindings.size();
		layoutDesc.ci.pBindings = layoutDesc.bindings.data();
		std::unordered_map<std::string, uint32_t> nameToBinding{{"InputImage", 0},
																{"OutputImage", 1}};
		layoutNameToIndex.insert({"Images", 1});
		layouts.emplace_back(
			device->createDescriptorSetLayout(layoutDesc, std::move(nameToBinding)));
	}
	// build pipeline layout
	PipelineLayoutDesc layoutDesc;
	layoutDesc.setLayouts = std::move(layouts);
	layoutDesc.ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutDesc.ci.pNext = nullptr;
	layoutDesc.ci.flags = 0;
	layoutDesc.ci.pushConstantRangeCount = 0;
	layoutDesc.ci.pPushConstantRanges = nullptr;
	const auto pipelineLayout = device->createPipelineLayout(std::move(layoutDesc));

	VkComputePipelineCreateInfo ci;
	ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	ci.pNext = nullptr;
	ci.flags = 0;
	ci.stage = computeShader->getShaderStageCi();
	ci.basePipelineHandle = VK_NULL_HANDLE;
	ci.basePipelineIndex = 0;
	auto pipeline = device->createComputePipeline(std::move(ci), pipelineLayout);
	return std::make_shared<MaterialTemplateVk>(device, std::vector{computeShader},
												std::move(layoutNameToIndex), pipeline);
}

MaterialFactoryVk::MaterialFactoryVk(std::shared_ptr<VulkanDevice> device)
	: device(std::move(device))
{
}

std::shared_ptr<VulkanShader> MaterialFactoryVk::createShader(const std::string& path,
															  VkShaderStageFlagBits stage) const
{
	std::ifstream fstream{path, std::ios::ate | std::ios::binary};
	const std::size_t fileSize = fstream.tellg();
	fstream.seekg(0);
	std::vector<char> code(fileSize);
	fstream.read(code.data(), fileSize);
	VkShaderModuleCreateInfo ci;
	ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ci.pNext = nullptr;
	ci.flags = 0;
	ci.codeSize = code.size();
	ci.pCode = reinterpret_cast<const uint32_t*>(code.data());
	return device->createShader(ci, stage);
}
