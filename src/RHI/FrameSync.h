#pragma once
#include"RenderConfig.h"
#include<vulkan/vulkan.h>

#include<vector>

namespace wrj {
	class FrameSync {
	public:
		FrameSync() = default;
		~FrameSync();
		FrameSync(const FrameSync& otherFrameSync) = delete;
		FrameSync& operator=(const FrameSync& otherFrameSync) = delete;
		FrameSync(FrameSync&& otherFrameSync) = delete;
		FrameSync& operator=(FrameSync&& otherFrameSync) = delete;
		void initFrameSync(VkDevice device,VkQueue graphicsQueue,std::uint32_t swapchainTotal);
	private:
		std::uint32_t m_swapchainTotal{ 0 };
		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		VkSemaphore m_timeSemaphore = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		VkQueue m_graphicsQueue = VK_NULL_HANDLE;
	};
}