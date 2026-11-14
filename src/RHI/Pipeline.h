#pragma once
#include<vulkan/vulkan.h>

#include<vector>

namespace wrj {
	class Pipeline
	{
	public:
		Pipeline() = default;
		~Pipeline();

		void init(VkDevice device);
	private:
		VkDevice m_device{ VK_NULL_HANDLE };

		
	};
}