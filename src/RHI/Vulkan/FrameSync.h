#pragma once
#include"RenderConfig.h"
#include<vulkan/vulkan.h>

#include<vector>
#include<array>
#include<optional>

namespace RHI {
	class Swapchain;

	class FrameSync {
	public:
		FrameSync(VkDevice device,VkQueue graphicsQueue);
		~FrameSync();
		FrameSync(const FrameSync& otherFrameSync) = delete;
		FrameSync& operator=(const FrameSync& otherFrameSync) = delete;
		FrameSync(FrameSync&& otherFrameSync) = delete;
		FrameSync& operator=(FrameSync&& otherFrameSync) = delete;
		std::optional<Frame> frameBegin(Swapchain& swapchain);
		bool frameEnd(const Frame& frame, Swapchain& swapchain);

		// 已提交到图形队列的最大 timeline 值：此刻之前提交的 work 都 ≤ 这个值
		std::uint64_t currentTimelineValue() const { return m_timeValue; }
		VkSemaphore timelineSemaphore() const { return m_graphicsSemaphore; }
	private:
		const bool waitTimeline(std::uint64_t value)const;
		void destory();
		struct Slot
		{
			std::uint32_t retire{ 0 };
			VkSemaphore acquied{ VK_NULL_HANDLE };
		};
		std::array<Slot,MAX_FRAMES_IN_FLIGHT> m_imageAvailableSemaphores;
		VkSemaphore m_graphicsSemaphore = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		VkQueue m_graphicsQueue = VK_NULL_HANDLE;
		std::uint64_t m_timeValue{ 0 };
		std::uint32_t m_frameIndex{ 0 };
	};
}