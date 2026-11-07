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
			if (imageAvailable != VK_NULL_HANDLE)vkDestroySemaphore(m_device, imageAvailable, nullptr);
		}
		if (m_timeSemaphore != VK_NULL_HANDLE)vkDestroySemaphore(m_device, m_timeSemaphore, nullptr);
	}
	void FrameSync::initFrameSync(VkDevice device, VkQueue graphicsQueue,std::uint32_t swapchainTotal)
	{
		m_device = device;
		m_graphicsQueue = graphicsQueue;
		m_swapchainTotal = swapchainTotal;
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		m_renderFinishedSemaphores.assign(swapchainTotal, VK_NULL_HANDLE);
		m_imageAvailableSemaphores.assign(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
		for (std::uint32_t i = 0;i < swapchainTotal;++i)
		{
			if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create render finished semaphore");
			}
		}
		for (std::uint32_t i = 0;i < MAX_FRAMES_IN_FLIGHT;++i)
		{
			if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS)
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
		if (vkCreateSemaphore(m_device, &info, nullptr, &m_timeSemaphore) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create timeline semaphore");
		}
	}
}