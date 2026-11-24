#include"FrameSync.h"
#include"Swapchain.h"
#include"Common.hpp"

#include<stdexcept>

namespace RHI {

	void FrameSync::initFrameSync(VkDevice device, VkQueue graphicsQueue)
	{
		//See Device::Device
		try
		{
			m_device = device;
			m_graphicsQueue = graphicsQueue;
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			for (std::uint32_t i = 0;i < MAX_FRAMES_IN_FLIGHT;++i)
			{
				if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i].acquied) != VK_SUCCESS)
				{
					throw std::runtime_error("failed to create image available semaphore");
				}
				DebugName::set(m_imageAvailableSemaphores[i].acquied,"imageAvailableSemaphore");
			}

			VkSemaphoreTypeCreateInfo typeInfo{};
			typeInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
			typeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
			typeInfo.initialValue = 0;

			VkSemaphoreCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			info.pNext = &typeInfo;
			if (vkCreateSemaphore(m_device, &info, nullptr, &m_graphicsSemaphore) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create timeline semaphore");
			}
			DebugName::set(m_graphicsSemaphore,"graphicsTimeSemaphore");
		}
		catch(...)
		{
			destory();
			throw;
		}
	}

	FrameSync::~FrameSync()
	{
		destory();
	}

	//Remember to null-check to avoid a double free.
	void FrameSync::destory()
	{
		if(!m_imageAvailableSemaphores.empty())
		{
			for (auto& imageAvailable : m_imageAvailableSemaphores)
			{
				if (imageAvailable.acquied != VK_NULL_HANDLE)vkDestroySemaphore(m_device, imageAvailable.acquied, nullptr);
			}
		}
		if (m_graphicsSemaphore != VK_NULL_HANDLE)vkDestroySemaphore(m_device, m_graphicsSemaphore, nullptr);
	}

	std::optional<Frame> FrameSync::frameBegin(Swapchain& swapchain)
	{
		auto& slot = m_imageAvailableSemaphores[m_frameIndex];
		if (slot.retire)
		{
			if (!waitTimeline(slot.retire))
			{
				throw std::runtime_error("slot: retire timeout at");
			}
		}
		std::uint32_t imageIndex{ 0 };
		if (swapchain.acquire(slot.acquied, imageIndex) != Swapchain::AcquireResult::Success)
		{
			return std::nullopt;
		}
		slot.retire = ++m_timeValue;
		return Frame{
			.frameIndex = m_frameIndex,
			.imageIndex = imageIndex,
			.signalValue = m_timeValue,
			.imageAvailable = slot.acquied,
			.renderFinished = swapchain.renderFinished(imageIndex)
		};
	}
	bool FrameSync::frameEnd(const Frame& frame, Swapchain& swapchain)
	{
		VkSemaphoreSubmitInfo waitInfo{};
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		waitInfo.semaphore = frame.imageAvailable;
		waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkCommandBufferSubmitInfo commandInfo{};
		commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		commandInfo.commandBuffer = frame.commandBuffer;

		VkSemaphoreSubmitInfo signalTimelineInfo{};
		signalTimelineInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signalTimelineInfo.semaphore = m_graphicsSemaphore;
		signalTimelineInfo.value = frame.signalValue;
		signalTimelineInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

		VkSemaphoreSubmitInfo signalPresentInfo{};
		signalPresentInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signalPresentInfo.semaphore = frame.renderFinished;
		signalPresentInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

		std::array<VkSemaphoreSubmitInfo, 2> submitInfos{  signalTimelineInfo, signalPresentInfo };
		VkSubmitInfo2 submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.waitSemaphoreInfoCount = 1;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &commandInfo;
		submitInfo.pWaitSemaphoreInfos = &waitInfo;
		submitInfo.signalSemaphoreInfoCount = static_cast<std::uint32_t>(submitInfos.size());
		submitInfo.pSignalSemaphoreInfos = submitInfos.data();

		vkQueueSubmit2(m_graphicsQueue,1, &submitInfo, VK_NULL_HANDLE);

		const bool presented = swapchain.present(frame.imageIndex,m_graphicsQueue);

		m_frameIndex = (m_frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

		return presented;
	}
	const bool FrameSync::waitTimeline(std::uint64_t value)const
	{
		std::uint64_t v = value;
		VkSemaphoreWaitInfo waitInfo{};
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		waitInfo.semaphoreCount = 1;
		waitInfo.pSemaphores = &m_graphicsSemaphore;
		waitInfo.pValues = &v;
		const auto result = vkWaitSemaphores(m_device, &waitInfo, UINT64_MAX);
		return result == VK_SUCCESS;
	}
}