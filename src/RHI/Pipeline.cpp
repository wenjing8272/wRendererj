#include"Pipeline.h"

namespace wrj {
	Pipeline::~Pipeline()
	{
		for (auto& shader : m_shader)
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
		const auto shaderIndex = acquireIndex(m_shader);
		auto& shader = m_shader[shaderIndex];
		VkShaderModuleCreateInfo shaderInfo{};
		shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderInfo.codeSize = static_cast<std::uint32_t>(4 * shaderCode.size());
		shaderInfo.pCode = shaderCode.data();

		vkCreateShaderModule(m_device, &shaderInfo, nullptr, &shader.shaderModule);
		return { shaderIndex,shader.generation };
	}
}