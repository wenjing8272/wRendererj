#include"ResourcesPool.h"

#include<cassert>
#include<stdexcept>

namespace wrj {
	ResourcesPool::~ResourcesPool()
	{
		for (auto& buffer : m_buffers)
		{
			if (buffer.alive)
				vmaDestroyBuffer(m_allocator, buffer.buffer, buffer.allocation);
		}
	}
	void ResourcesPool::init(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue,VmaAllocator allocator)
	{
		m_device = device;
		m_commandPool = commandPool;
		m_graphicsQueue = graphicsQueue;
		m_allocator = allocator;
	}
	std::uint32_t ResourcesPool::acquireIndex(std::vector<BufferRecord>& buffers)
	{
		for (size_t i = 0;i < buffers.size();++i)
		{
			if (buffers[i].alive)continue;
			buffers[i].alive = true;
			buffers[i].generation += 1;
			return static_cast<std::uint32_t>(i);
		}
		auto& buffer = buffers.emplace_back();    //C++17以后emplace_back直接返回引用
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
		if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		}
		else
		{
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		}

		VmaAllocationInfo allocationInfo{};
		auto result = vmaCreateBuffer(m_allocator, &bufferInfo, &allocationCreateInfo, &bufferRecord.buffer, &bufferRecord.allocation, &allocationInfo);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create buffer");
		}
		bufferRecord.mapped = allocationInfo.pMappedData;

		bufferRecord.size = size;

		return { index,bufferRecord.generation };
	}
	void ResourcesPool::copyBuffer(BufferHandle copyBuffer, const void* data, VkDeviceSize dataSize)
	{
		auto& buffer = m_buffers[copyBuffer.index];
		assert(dataSize == buffer.size && buffer.alive && buffer.generation == copyBuffer.generation, "buffer has not vailde");
		if (buffer.mapped == nullptr)
		{
			throw std::runtime_error("no mapped memory");
		}
		memcpy(m_buffers[copyBuffer.index].mapped, data, dataSize);
	}
	bool ResourcesPool::isBufferVaild(BufferHandle bufferHandle)
	{
		if (bufferHandle.index >= m_buffers.size())return false;
		return m_buffers[bufferHandle.index].alive && m_buffers[bufferHandle.index].generation == bufferHandle.generation;
	}
	VkBuffer ResourcesPool::getBuffer(BufferHandle bufferHandle)
	{
		return m_buffers[bufferHandle.index].buffer;
	}
	void ResourcesPool::destoryBuffer(BufferHandle bufferHandle)
	{
		if (!isBufferVaild(bufferHandle))return;
		auto buffer = m_buffers[bufferHandle.index];
		vmaDestroyBuffer(m_allocator, buffer.buffer, buffer.allocation);
		buffer.alive = false;
		buffer.buffer = VK_NULL_HANDLE;
		buffer.allocation = VK_NULL_HANDLE;
		buffer.mapped = nullptr;
	}
}