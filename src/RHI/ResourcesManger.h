#pragma once
#include<vulkan/vulkan.h>

namespace wrj {
	class ResourcesManger
	{
	public:
		ResourcesManger() = default;
		~ResourcesManger() = default;
		void init(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
		void cleanup();
		VkBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& bufferMemory);
		VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& imageMemory);
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	private:
		VkDevice m_device;
		VkPhysicalDevice m_physicalDevice;
		VkCommandPool m_commandPool;
		VkQueue m_graphicsQueue;

		
	};
}