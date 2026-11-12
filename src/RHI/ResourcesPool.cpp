#include"ResourcesPool.h"

namespace wrj {
	ResourcesPool::~ResourcesPool()
	{

	}
	void ResourcesPool::init(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue,VmaAllocator allocator)
	{
		m_device = device;
		m_physicalDevice = physicalDevice;
		m_commandPool = commandPool;
		m_graphicsQueue = graphicsQueue;
		m_allocator = allocator;
	}
	std::uint32_t ResourcesPool::acquireIndex(std::vector<BufferRecord> buffers)
	{
		for (size_t i = 0;i < buffers.size();++i)
		{
			if (buffers[i].alive)continue;
			buffers[i].alive = true;
			buffers[i].generation += 1;
			return static_cast<std::uint32_t>(i);
		}
		buffers.emplace_back();
		auto buffer = buffers.back();
		buffer.alive = true;
		buffer.generation = 1;
		return static_cast<std::uint32_t>(buffers.size() - 1);
	}

	BufferHandle ResourcesPool::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		const auto index = acquireIndex(m_buffers);
		auto& bufferRecord = m_buffers[index];

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.memoryTypeBits = properties;

		VmaAllocationInfo allocationInfo{};
		vmaCreateBuffer(m_allocator, &bufferInfo, &allocationCreateInfo, &bufferRecord.buffer, &bufferRecord.allocation, &allocationInfo);

		return { index,bufferRecord.generation };
	}
}