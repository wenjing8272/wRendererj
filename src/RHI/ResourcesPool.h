#pragma once
#include<vulkan/vulkan.h>
#include<vma/vk_mem_alloc.h>
#include"Types.h"

#include<vector>

namespace wrj {
	struct BufferRecord
	{
		VkBuffer buffer{ VK_NULL_HANDLE };
		VkDeviceMemory bufferMemory{ VK_NULL_HANDLE };
		VmaAllocation allocation{ VK_NULL_HANDLE };
		void* mapped{ nullptr };

		bool alive{ false };
		std::uint32_t generation{ UINT32_MAX };
	};

	class ResourcesPool
	{
	public:
		ResourcesPool() = default;
		~ResourcesPool();
		void init(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue,VmaAllocator allocator);

		BufferHandle createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& imageMemory);
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		void destoryBuffer();

		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	private:
		std::uint32_t acquireIndex(std::vector<BufferRecord> buffers);
		VkDevice m_device;
		VkPhysicalDevice m_physicalDevice;
		VkCommandPool m_commandPool;
		VkQueue m_graphicsQueue;
		VmaAllocator m_allocator{ VK_NULL_HANDLE };

		std::vector<BufferRecord> m_buffers;
	};
}