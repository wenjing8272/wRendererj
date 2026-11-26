#include"ResourcesPool.h"
#include"Common.hpp"
#include"DeferedDeleter.h"

#include<cassert>
#include<stdexcept>
#include<iostream>

namespace RHI {
	ResourcesPool::ResourcesPool(VkDevice device, 
								 VkCommandPool commandPool, 
								 VkQueue graphicsQueue,
								 VmaAllocator allocator,
								 DeferedDeleter& deferedDeleter):
	m_device(device),m_commandPool(commandPool),m_graphicsQueue(graphicsQueue),m_allocator(allocator),
	m_deferedDeleter(&deferedDeleter)
	{

	}
	ResourcesPool::~ResourcesPool()
	{
		for (auto& buffer : m_buffers)
		{
			if (buffer.alive)
				vmaDestroyBuffer(m_allocator, buffer.buffer, buffer.allocation);
		}
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
			std::cout<<"device local index: "<<index<<'\n';
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		}
		else
		{
			std::cout<<"host visible index: "<<index<<'\n';
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		}

		VmaAllocationInfo allocationInfo{};
		auto result = vmaCreateBuffer(m_allocator, &bufferInfo, &allocationCreateInfo, &bufferRecord.buffer, &bufferRecord.allocation, &allocationInfo);
		if (result != VK_SUCCESS)
		{
			bufferRecord.alive = false;
			throw std::runtime_error("failed to create buffer");
		}
		bufferRecord.mapped = allocationInfo.pMappedData;

		bufferRecord.size = size;

		return { index,bufferRecord.generation };
	}
	void ResourcesPool::copyBuffer(BufferHandle copyBuffer, const void* data, VkDeviceSize dataSize)
	{
		auto& buffer = m_buffers[copyBuffer.index];
		assert(dataSize == buffer.size && buffer.alive && buffer.generation == copyBuffer.generation && "buffer has not vailde");
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
	void ResourcesPool::destoryBuffer(BufferHandle bufferHandle, std::uint64_t retireValue)
	{
		if (!isBufferVaild(bufferHandle))return;
		auto& buffer = m_buffers[bufferHandle.index];
		// 句柄立即失效，槽位可被 acquireIndex 复用（generation 自增）；
		// 真实对象按值捕获捕获进删除队列，不会误删复用后的新 buffer
		const VkBuffer vkBuffer = buffer.buffer;
		const VmaAllocation allocation = buffer.allocation;
		buffer.alive = false;
		buffer.buffer = VK_NULL_HANDLE;
		buffer.allocation = VK_NULL_HANDLE;
		buffer.mapped = nullptr;

		m_deferedDeleter->push(retireValue, [allocator = m_allocator, vkBuffer, allocation]() {
			vmaDestroyBuffer(allocator, vkBuffer, allocation);
		});
	}
}
