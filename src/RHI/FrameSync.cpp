#include"FrameSync.h"

#include<stdexcept>

namespace wrj {
	FrameSync::~FrameSync()
	{
		if (m_device == VK_NULL_HANDLE)return;
		for (auto& renderFinished : m_renderFinishedSemaphores)
		{
			if (renderFinished != VK_NULL_HANDLE)vkDestroySemaphore(m_device, renderFinished, nullptr);
		}
		for (auto& imageAvailable : m_imageAvailableSemaphores)
		{
			if (imageAvailable.acquied != VK_NULL_HANDLE)vkDestroySemaphore(m_device, imageAvailable.acquied, nullptr);
		}
		if (m_graphicsSemaphore != VK_NULL_HANDLE)vkDestroySemaphore(m_device, m_graphicsSemaphore, nullptr);
	}
	void FrameSync::initFrameSync(VkDevice device, VkQueue graphicsQueue,std::uint32_t swapchainTotal)
	{
		m_device = device;
		m_graphicsQueue = graphicsQueue;
		m_swapchainTotal = swapchainTotal;
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		m_renderFinishedSemaphores.assign(swapchainTotal, VK_NULL_HANDLE);
		for (std::uint32_t i = 0;i < swapchainTotal;++i)
		{
			if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create render finished semaphore");
			}
		}
		for (std::uint32_t i = 0;i < MAX_FRAMES_IN_FLIGHT;++i)
		{
			if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i].acquied) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create image available semaphore");
			}
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
	}
	std::optional<FrameResources> FrameSync::frameBegin(VkSwapchainKHR swapchain)
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
		const auto acquireResult = vkAcquireNextImageKHR(m_device, swapchain, UINT64_MAX, slot.acquied, VK_NULL_HANDLE, &imageIndex);
		if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)return std::nullopt;
		slot.retire = ++m_timeValue;
		return FrameResources{
			m_frameIndex,
			imageIndex,
			m_timeValue,
			m_imageAvailableSemaphores[m_frameIndex].acquied,
			m_renderFinishedSemaphores[imageIndex]
		};
	}
	bool FrameSync::frameEnd(const FrameResources& frame, VkSwapchainKHR swapchain)
	{

	}
	const bool FrameSync::waitTimeline(std::uint64_t value)const
	{
		std::uint64_t v = value;
		VkSemaphoreWaitInfo waitInfo{};
		waitInfo.semaphoreCount = 1;
		waitInfo.pSemaphores = &m_graphicsSemaphore;
		waitInfo.pValues = &v;
		const auto result = vkWaitSemaphores(m_device, &waitInfo, UINT64_MAX);
		return result == VK_SUCCESS;
	}
}