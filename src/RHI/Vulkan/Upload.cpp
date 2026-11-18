#include"Upload.h"

namespace RHI {
	Upload::~Upload()
	{
		vkDestroySemaphore(m_device, m_transferCompletedSemaphore, nullptr);
	}
	void Upload::init(VkDevice device,VkCommandPool transferPool, VkQueue transferQueue)
	{
		m_device = device;
		m_transferPool = transferPool;
		m_transferQueue = transferQueue;

		VkCommandBufferAllocateInfo transferCommandInfo{};
		transferCommandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		transferCommandInfo.commandBufferCount = 1;
		transferCommandInfo.commandPool = m_transferPool;
		transferCommandInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		vkAllocateCommandBuffers(m_device, &transferCommandInfo, &m_transferCommandBuffer);

		VkSemaphoreTypeCreateInfo typeInfo{};
		typeInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		typeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		typeInfo.initialValue = 0;

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = &typeInfo;

		vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_transferCompletedSemaphore);
	}
	void Upload::uploadBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,VkDeviceSize bufferSize)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(m_transferCommandBuffer, &beginInfo);
		const auto bufferCopy = VkBufferCopy(0, 0, bufferSize);
		vkCmdCopyBuffer(m_transferCommandBuffer, srcBuffer, dstBuffer, 1,&bufferCopy);
		vkEndCommandBuffer(m_transferCommandBuffer);

		VkCommandBufferSubmitInfo commandInfo{};
		commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		commandInfo.commandBuffer = m_transferCommandBuffer;
		VkSubmitInfo2 submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &commandInfo;
		
		vkQueueSubmit2(m_transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_transferQueue);
	}
}
