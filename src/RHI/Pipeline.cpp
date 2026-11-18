#include"Pipeline.h"
#include"Common.h"

#include"SetDebugName.h"

#include<stdexcept>
#include<iostream>

namespace wrj {
	Pipeline::~Pipeline()
	{
		for (auto& shader : m_shaders)
		{
			shader.alive = false;
			vkDestroyShaderModule(m_device, shader.shaderModule,nullptr);
		}
		for (auto& pipeline : m_pipelines)
		{
			std::cout << "pipeline" << "\n";
			pipeline.alive = false;
			vkDestroyPipeline(m_device, pipeline.pipeline, nullptr);
			vkDestroyPipelineLayout(m_device, pipeline.pipelineLayout, nullptr);
		}
	}
	void Pipeline::init(VkDevice device)
	{
		m_device = device;
	}
	template<class Record>
	std::uint32_t Pipeline::acquireIndex(std::vector<Record>& records)
	{
		for (std::uint32_t i = 0;i < records.size();++i)
		{
			if (records[i].alive)continue;
			records[i].alive = true;
			records[i].generation += 1;
			return i;
		}
		auto& record = records.emplace_back();
		record.alive = true;
		record.generation = 1;
		return static_cast<std::uint32_t>(records.size() - 1);
	}
	ShaderHandle Pipeline::createShader(const std::vector<std::uint32_t>& shaderCode)
	{
		const auto shaderIndex = acquireIndex(m_shaders);
		auto& shader = m_shaders[shaderIndex];
		VkShaderModuleCreateInfo shaderInfo{};
		shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderInfo.codeSize = static_cast<std::uint32_t>(4 * shaderCode.size());
		shaderInfo.pCode = shaderCode.data();

		if (vkCreateShaderModule(m_device, &shaderInfo, nullptr, &shader.shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module");
		}
		return { shaderIndex,shader.generation };
	}
	PipelineHandle Pipeline::createGraphicsPipeline(const GraphicsPipeline& graphicsPipeline,VkFormat format)
	{
		if (graphicsPipeline.fragmentShader.index >= m_shaders.size() ||
			graphicsPipeline.fragmentShader.generation != m_shaders[graphicsPipeline.fragmentShader.index].generation ||
			!m_shaders[graphicsPipeline.fragmentShader.index].alive)
		{
			throw std::runtime_error("fragment shader no has");
		}
		if (graphicsPipeline.vertexShader.index >= m_shaders.size() ||
			graphicsPipeline.vertexShader.generation != m_shaders[graphicsPipeline.vertexShader.index].generation ||
			!m_shaders[graphicsPipeline.vertexShader.index].alive)
		{
			throw std::runtime_error("vertex shader no has");
		}
		const auto pipelineIndex = acquireIndex(m_pipelines);
		auto& pipeline = m_pipelines[pipelineIndex];

		VkPipelineShaderStageCreateInfo vertexShaderInfo{};
		vertexShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexShaderInfo.pName = graphicsPipeline.vertexName;
		vertexShaderInfo.module = m_shaders[graphicsPipeline.vertexShader.index].shaderModule;

		VkPipelineShaderStageCreateInfo fragmentShaderInfo{};
		fragmentShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentShaderInfo.pName = graphicsPipeline.fragmentName;
		fragmentShaderInfo.module = m_shaders[graphicsPipeline.fragmentShader.index].shaderModule;

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vertexShaderInfo,fragmentShaderInfo };

		std::vector<VkVertexInputBindingDescription> bindingDescriptions;


		if (!graphicsPipeline.vertexBinding.isVaild())
		{
			std::cout << graphicsPipeline.vertexBinding.binding << "\n";
			std::cout << graphicsPipeline.vertexBinding.stride << "\n";
			throw std::runtime_error("vertex binding no vaild");
		}
		auto& bindingDescription = bindingDescriptions.emplace_back();
		bindingDescription.binding = graphicsPipeline.vertexBinding.binding;
		bindingDescription.stride = graphicsPipeline.vertexBinding.stride;
		if (graphicsPipeline.vertexBinding.isPerInstance)
		{
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
		}
		else
		{
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		}

		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		for (const auto& i : graphicsPipeline.vertexAttribute)
		{
			if (!i.isVaild())throw std::runtime_error("vertex attribute no vaild");
			auto& attributeDescription = attributeDescriptions.emplace_back();
			attributeDescription.binding = i.binding;
			attributeDescription.location = i.location;
			attributeDescription.offset = i.offset;
			attributeDescription.format = userToVK(i.format);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputState{};
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.vertexBindingDescriptionCount = static_cast<std::uint32_t>(bindingDescriptions.size());
		vertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();
		vertexInputState.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributeDescriptions.size());
		vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo assemblyState{};
		assemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assemblyState.topology = userToVK(graphicsPipeline.primitiveTopology);

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		std::vector<VkDynamicState> dynamics;
		for (const auto& i : graphicsPipeline.dynamicStates)
		{
			auto& dynamic = dynamics.emplace_back();
			dynamic = userToVK(i);
		}
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<std::uint32_t>(dynamics.size());
		dynamicState.pDynamicStates = dynamics.data();

		VkPipelineRasterizationStateCreateInfo rasterizationState{};
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.cullMode = userToVK(graphicsPipeline.cullMode);
		rasterizationState.frontFace = userToVK(graphicsPipeline.frontFace);
		rasterizationState.depthBiasEnable = userBoolToVK(graphicsPipeline.depthBaisEnable);
		rasterizationState.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.rasterizationSamples = userToVK(graphicsPipeline.sampleCount);
		multisampleState.sampleShadingEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState attachmentState{};
		attachmentState.blendEnable = userBoolToVK(graphicsPipeline.blendEnable);
		attachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
										 VK_COLOR_COMPONENT_G_BIT |
										 VK_COLOR_COMPONENT_B_BIT |
										 VK_COLOR_COMPONENT_A_BIT;
		VkPipelineColorBlendStateCreateInfo blendState{};
		blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blendState.logicOpEnable = VK_FALSE;
		blendState.logicOp = VK_LOGIC_OP_COPY;
		blendState.attachmentCount = 1;
		blendState.pAttachments = &attachmentState;

		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		if (vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &pipeline.pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed create pipeline layout");
		}
		VkPipelineRenderingCreateInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachmentFormats = &format;

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.pColorBlendState = &blendState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.pInputAssemblyState = &assemblyState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pVertexInputState = &vertexInputState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.stageCount = static_cast<std::uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();
		pipelineCreateInfo.layout = pipeline.pipelineLayout;
		pipelineCreateInfo.renderPass = nullptr;
		pipelineCreateInfo.pNext = &renderingInfo;

		if (vkCreateGraphicsPipelines(m_device, {}, 1, &pipelineCreateInfo, nullptr, &pipeline.pipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline");
		}
		DebugName::set(pipeline.pipelineLayout, graphicsPipeline.debugName);
		DebugName::set(pipeline.pipeline, graphicsPipeline.debugName);
		return { pipelineIndex,pipeline.generation };
	}
	VkPipeline Pipeline::getPipeline(PipelineHandle pipelineHandle)
	{
		return m_pipelines[pipelineHandle.index].pipeline;
	}
	bool Pipeline::pipelineIsVaild(PipelineHandle pipelineHandle)
	{
		if (pipelineHandle.index >= m_pipelines.size() || pipelineHandle.index < 0)return false;
		return m_pipelines[pipelineHandle.index].alive && m_pipelines[pipelineHandle.index].generation == pipelineHandle.generation;
	}
}