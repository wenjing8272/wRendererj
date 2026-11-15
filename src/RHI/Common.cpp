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
	VkFormat userToVK(Format format)
	{
		switch (format)
		{
		case Format::R8G8B8A8_UNorm:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case Format::R8G8B8A8_SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case Format::B8G8R8A8_UNorm:
			return VK_FORMAT_B8G8R8A8_UNORM;
		case Format::B8G8R8A8_SRGB:
			return VK_FORMAT_B8G8R8A8_SRGB;
		case Format::R16G16B16A16_SFloat:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case Format::R8G8_UNorm:
			return VK_FORMAT_R8G8_UNORM;
		case Format::D32_SFloat:
			return VK_FORMAT_D32_SFLOAT;
		case Format::Undefined:
			return VK_FORMAT_UNDEFINED;
		}
	}
	VkPrimitiveTopology userToVK(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case PrimitiveTopology::LineList:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case PrimitiveTopology::TriangleList:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case PrimitiveTopology::TriangleStrip:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		}
	}
}