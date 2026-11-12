#include"Common.h"

namespace wrj {
	VkBufferUsageFlags userToVK(BufferUsage bufferUsage)
	{
		VkBufferUsageFlags result{};
		
		if (isFlag(bufferUsage, BufferUsage::VertexBuffer))result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		if (isFlag(bufferUsage, BufferUsage::IndexBuffer))result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (isFlag(bufferUsage, BufferUsage::UniformBuffer))result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (isFlag(bufferUsage, BufferUsage::StorageBuffer))result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		if (isFlag(bufferUsage, BufferUsage::TransferSrc))result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		if (isFlag(bufferUsage, BufferUsage::TransferDst))result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		return result;
	}
	VkMemoryPropertyFlags userToVK(MemoryProperty memoryProperty)
	{
		switch (memoryProperty)
		{
		case MemoryProperty::HostVisible:
			return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		case MemoryProperty::DeviceLocal:
			return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}
	}
}