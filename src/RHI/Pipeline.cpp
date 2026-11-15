#include"Pipeline.h"
#include"Common.h"

#include<stdexcept>

namespace wrj {
	Pipeline::~Pipeline()
	{
		for (auto& shader : m_shaders)
		{
			shader.alive = false;
			vkDestroyShaderModule(m_device, shader.shaderModule,nullptr);
		}
	}
	void Pipeline::init(VkDevice device)
	{
		m_device = device;
	}
	std::uint32_t Pipeline::acquireIndex(std::vector<ShaderRecord>& shaderRecord)
	{
		for (std::uint32_t i = 0;i < shaderRecord.size();++i)
		{
			if (shaderRecord[i].alive)continue;
			shaderRecord[i].alive = true;
			shaderRecord[i].generation += 1;
			return i;
		}
		auto& shader = shaderRecord.emplace_back();
		shader.alive = true;
		shader.generation = 1;
		return static_cast<std::uint32_t>(shaderRecord.size() - 1);
	}
	ShaderHandle Pipeline::createShader(const std::vector<std::uint32_t>& shaderCode)
	{
		const auto shaderIndex = acquireIndex(m_shaders);
		auto& shader = m_shaders[shaderIndex];
		VkShaderModuleCreateInfo shaderInfo{};
		shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderInfo.codeSize = static_cast<std::uint32_t>(4 * shaderCode.size());
		shaderInfo.pCode = shaderCode.data();

		vkCreateShaderModule(m_device, &shaderInfo, nullptr, &shader.shaderModule);
		return { shaderIndex,shader.generation };
	}
	PipelineHandle Pipeline::createGraphicsPipeline(const GraphicsPipeline& graphicsPipeline)
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

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderInfo,fragmentShaderInfo };

		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		for (auto& i : graphicsPipeline.vertexBinding)
		{
			if (i.isVaild())throw std::runtime_error("vertex binding no vaild");
			auto& bindingDescription = bindingDescriptions.emplace_back();
			bindingDescription.binding = i.binding;
			bindingDescription.stride = i.stride;
			if (i.isPerInstance)
			{
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
			}
			else
			{
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			}
		}
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		for (auto& i : graphicsPipeline.vertexAttribute)
		{
			if (i.isVaild())throw std::runtime_error("vertex attribute no vaild");
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
	}
}