#pragma once
#include<vulkan/vulkan.h>
#include<vma/vk_mem_alloc.h>
#include"Types.h"

#include<vector>

namespace wrj {
	struct BufferRecord
	{
		VkBuffer buffer{ VK_NULL_HANDLE };
		VmaAllocation allocation{ VK_NULL_HANDLE };
		void* mapped{ nullptr };
		VkDeviceSize size{ 0 };

		bool alive{ false };
		std::uint32_t generation{ UINT32_MAX };
	};

	struct ShaderRecord
	{
		VkShaderModule shaderModule{ VK_NULL_HANDLE };
		
		bool alive{ false };
		std::uint32_t generation{ UINT32_MAX };
	};

	class ResourcesPool
	{
	public:
		ResourcesPool() = default;
		~ResourcesPool();
		void init(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue,VmaAllocator allocator);

		BufferHandle createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		ShaderHandle createShaderModule(const std::vector<char>& code);
		VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& imageMemory);
		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		bool isBufferVaild(BufferHandle bufferHandle);
		VkBuffer getBuffer(BufferHandle bufferHandle);

		void destoryBuffer(BufferHandle bufferHandle);

		void copyBuffer(BufferHandle copyBuffer,const void* data,VkDeviceSize dataSize);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	private:
		template<class Record>
		std::uint32_t acquireIndex(std::vector<Record>& buffers);
		VkDevice m_device{ VK_NULL_HANDLE };
		VkCommandPool m_commandPool{ VK_NULL_HANDLE };
		VkQueue m_graphicsQueue{ VK_NULL_HANDLE };
		VmaAllocator m_allocator{ VK_NULL_HANDLE };

		std::vector<BufferRecord> m_buffers;
	};
}