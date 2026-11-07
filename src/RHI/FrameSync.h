#pragma once
#include"RenderConfig.h"
#include<vulkan/vulkan.h>

#include<vector>
#include<array>
#include<optional>

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
		std::optional<FrameResources> frameBegin(VkSwapchainKHR swapchain);
		bool frameEnd(const FrameResources& frame, VkSwapchainKHR swapchain);
	private:
		const bool waitTimeline(std::uint64_t value)const;
		struct Slot
		{
			std::uint32_t retire{ 0 };
			VkSemaphore acquied{ VK_NULL_HANDLE };
		};
		std::uint32_t m_swapchainTotal{ 0 };
		std::array<Slot,MAX_FRAMES_IN_FLIGHT> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		VkSemaphore m_graphicsSemaphore = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		VkQueue m_graphicsQueue = VK_NULL_HANDLE;
		std::uint64_t m_timeValue{ 0 };
		std::uint32_t m_frameIndex{ 0 };
	};
}