#include"Upload.h"
#include"../Log.h"
#include"common.hpp"

#include<stdexcept>

namespace RHI {
	Upload::Upload(VkDevice device,VkCommandPool transferPool,VkQueue transferQueue):
	m_device(device),m_transferPool(transferPool),m_transferQueue(transferQueue)
	{
		//See Device::Device
		try{
			VkCommandBufferAllocateInfo transferCommandInfo{};
			transferCommandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			transferCommandInfo.commandBufferCount = 1;
			transferCommandInfo.commandPool = m_transferPool;
			transferCommandInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

			if(vkAllocateCommandBuffers(m_device, &transferCommandInfo, &m_transferCommandBuffer) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to allocate command buffer");
			}
			DebugName::set(m_transferCommandBuffer,"transferCommandBuffer");

			VkSemaphoreTypeCreateInfo typeInfo{};
			typeInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
			typeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
			typeInfo.initialValue = 0;

			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreInfo.pNext = &typeInfo;

			if(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_transferCompletedSemaphore) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create transfer completed semaphore");
			}
			DebugName::set(m_transferCompletedSemaphore,"transferCompletedSemaphore");
		}
		catch(...)
		{
			destory();
			throw;
		}
	}
	Upload::~Upload()
	{
		destory();
	}
	void Upload::destory()
	{
		if(m_transferCompletedSemaphore != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(m_device, m_transferCompletedSemaphore, nullptr);
		}
	}

	/*
	 @param srcBuffer host visible buffer
	 @param dstBuffer device local buffer
	 @param bufferSize data size
	*/
	void Upload::uploadBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,VkDeviceSize bufferSize)
	{
		// With VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT set, command buffers allocated from this pool can be reset
 		// individually. Before a command buffer is reused, it must be reset.
		if(vkResetCommandBuffer(m_transferCommandBuffer,0) != VK_SUCCESS)
		{
			LOG_WARN("failed to reset command buffer");
		}

		//Configure begin info 
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		//record command transition data of srcBuffer to dstBuffer
		vkBeginCommandBuffer(m_transferCommandBuffer, &beginInfo);
		const auto bufferCopy = VkBufferCopy(0, 0, bufferSize);
		vkCmdCopyBuffer(m_transferCommandBuffer, srcBuffer, dstBuffer, 1,&bufferCopy);
		vkEndCommandBuffer(m_transferCommandBuffer);

		//Configure submit info
		VkCommandBufferSubmitInfo commandInfo{};
		commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		commandInfo.commandBuffer = m_transferCommandBuffer;
		VkSubmitInfo2 submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.commandBufferInfoCount = 1;
		submitInfo.pCommandBufferInfos = &commandInfo;
		
		//Submit transition command
		if(vkQueueSubmit2(m_transferQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to upload resources");
		}
		vkQueueWaitIdle(m_transferQueue);
	}
}
