#pragma once
#include<vulkan/vulkan.h>
#include"Types.h"

#include<vector>

namespace wrj {
	struct ShaderRecord 
	{
		VkShaderModule shaderModule{ VK_NULL_HANDLE };

		bool alive{ false };
		std::uint32_t generation{ UINT32_MAX };
	};

	class Pipeline
	{
	public:
		Pipeline() = default;
		~Pipeline();

		void init(VkDevice device);

		ShaderHandle createShader(const std::vector<std::uint32_t>& shaderCode);
	private:
		std::uint32_t acquireIndex(std::vector<ShaderRecord>& shaderRecord);
		VkDevice m_device{ VK_NULL_HANDLE };

		std::vector<ShaderRecord> m_shader;
	};
}