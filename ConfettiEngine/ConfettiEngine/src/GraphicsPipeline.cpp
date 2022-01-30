#include "GraphicsPipeline.h"
#include "Shaders/Shader.h"
#include <iostream>

GraphicsPipeline::GraphicsPipeline(VkInstance& i, VkDevice& d, VkExtent2D& e, std::string v, std::string f)
	: instance(i), device(d), extents(e)
{
	//************* SHADER SETUP *************//
	std::vector<char> vertCode = Shader::read(v);
	//std::vector<char> geomCode = Shader::read(geom);
	std::vector<char> fragCode = Shader::read(f);

	VkShaderModule vert = createModule(vertCode);
	// VkShaderModule geom = createModule(geomCode);;
	VkShaderModule frag = createModule(fragCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vert;
	vertShaderStageInfo.pName = "main";
	// check .pSpecializationInfo to set constants in shader
	// useful as compiler can perform optimizations like removing if statements using consts
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = frag;
	fragShaderStageInfo.pName = "main";

	shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

	//************* FIXED PIPELINE SETUP *************//
	// Vertex input
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	// Optional, point to array of structs that describe details for loading vert data
	vertexInputInfo.pVertexBindingDescriptions = nullptr; 
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extents.width;
	viewport.height = (float)extents.height;
	// frame buffer depth values
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	// viewport clipping - no clipping in this case
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = extents;
	// viewport state
	VkPipelineViewportStateCreateInfo viewportState{};
	// some GPU's can use multiple viewports so they are referenced as arrays and need a count
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// depth clamp sets fragments to clamp within depth range, useful for shadowmaps
	rasterizer.depthClampEnable = VK_FALSE;
	// will basically disable output if true
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	// describes how geometry is filled, can use lines or point too but requires gpu features
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	// line thickness for drawn lines, requires widelines if > 1
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	// modifications to depth values
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	// Multisampling/anti-aliasing - disabled & requires gpu features
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	// Depth and Stencil testing goes here later

	// Color blending
	// per frame buffer settings
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
	// global settings/constants for all framebuffers 
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	// set true for bitwise combination blending (will disable other settings)
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	// Dynamic State
	// used for making some settings dynamically changeable
	VkDynamicState dynamicStates[] = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	// Pipeline Layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create pipeline layout!");
	}
	else std::cout << "Graphics Pipeline Layout created successfully\n";

	//************* CLEANUP *************//
	vkDestroyShaderModule(device, vert, nullptr);
	// vkDestroyShaderModule(device, geom, nullptr);
	vkDestroyShaderModule(device, frag, nullptr);
}

GraphicsPipeline::~GraphicsPipeline()
{
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}

GraphicsPipeline& GraphicsPipeline::operator=(const GraphicsPipeline& other)
{
	if (&other != this) {
		instance = other.instance;
		device = other.device;
		extents = other.extents;
		shaderStages = other.shaderStages;
	}
	return *this;
}

const VkPipelineLayout& GraphicsPipeline::getLayout()
{
	return pipelineLayout;
}

VkShaderModule GraphicsPipeline::createModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	// have to reinterpret to uint32_t as vulkan expects it, luckily vector will allow us to do this
	createInfo.pCode = reinterpret_cast<const uint32_t*>(&code[0]);

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("ERROR: Failed to create shader module!");
	}

	return shaderModule;
}
