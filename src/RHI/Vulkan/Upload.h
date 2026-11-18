#pragma once
#include<vulkan/vulkan.h>

namespace RHI {
	class Upload
	{
	public:
		Upload() = default;
		~Upload();
		Upload(const Upload& otherUpload) = delete;
		Upload& operator=(const Upload& otherUpload) = delete;
		Upload(Upload&& otherUpload) = delete;
		Upload&& operator=(Upload&& otherUpload) = delete;
		
		void init(VkDevice device,VkCommandPool transferPool,VkQueue transferQueue);
		void uploadBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,VkDeviceSize bufferSize);

	private:
		VkDevice m_device{ VK_NULL_HANDLE };
		VkCommandPool m_transferPool{ VK_NULL_HANDLE };
		VkQueue m_transferQueue{ VK_NULL_HANDLE };

		VkCommandBuffer m_transferCommandBuffer{ VK_NULL_HANDLE };

		VkSemaphore m_transferCompletedSemaphore{ VK_NULL_HANDLE };
	};
}