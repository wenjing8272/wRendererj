#pragma once
#include<vulkan/vulkan.h>
#include"../Types.h"

#include<vector>

namespace RHI {
	struct ShaderRecord 
	{
		VkShaderModule shaderModule{ VK_NULL_HANDLE };

		bool alive{ false };
		std::uint32_t generation{ UINT32_MAX };
	};

	struct PipelineRecord
	{
		VkPipeline pipeline{ VK_NULL_HANDLE };
		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };

		std::uint32_t generation{ UINT32_MAX };
		bool alive{ false };
	};

	class Pipeline
	{
	public:
		Pipeline() = default;
		~Pipeline();

		void init(VkDevice device);

		ShaderHandle createShader(const std::vector<std::uint32_t>& shaderCode);
		PipelineHandle createGraphicsPipeline(const GraphicsPipeline& graphicsPipeline,VkFormat format);

		bool pipelineIsVaild(PipelineHandle pipelineHandle);
		VkPipeline getPipeline(PipelineHandle);
	private:
		template<class Record>
		std::uint32_t acquireIndex(std::vector<Record>& record);
		VkDevice m_device{ VK_NULL_HANDLE };

		std::vector<ShaderRecord> m_shaders;
		std::vector<PipelineRecord> m_pipelines;
	};
}