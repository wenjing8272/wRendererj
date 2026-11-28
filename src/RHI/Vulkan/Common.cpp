#include"Common.hpp"

namespace RHI {
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
		default:
			LOG_WARN("memory property choose default property");
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
		case Format::R32G32B32A32_SFloat:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case Format::R8G8_UNorm:
			return VK_FORMAT_R8G8_UNORM;
		case Format::D32_SFloat:
			return VK_FORMAT_D32_SFLOAT;
		case Format::Undefined:
			return VK_FORMAT_UNDEFINED;
        case Format::R32G32_SFloat:
            return VK_FORMAT_R32G32_SFLOAT;
        case Format::R32G32B32_SFloat:
            return VK_FORMAT_R32G32B32_SFLOAT;
		default:
			LOG_WARN("format choose default format: r8g8b8a8srgb");
			return VK_FORMAT_R8G8B8A8_SRGB;
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
		default:
			LOG_WARN("topology use default: trangle list");
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
	}
	VkDynamicState userToVK(DynamicState dynamic)
	{
		switch (dynamic)
		{
		case DynamicState::Viewport:
			return VK_DYNAMIC_STATE_VIEWPORT;
		case DynamicState::Scissor:
			return VK_DYNAMIC_STATE_SCISSOR;
		default:
			LOG_WARN("dynamic default use: viewport");
			return VK_DYNAMIC_STATE_VIEWPORT;
		}
	}
	VkCullModeFlags userToVK(CullMode cullMode)
	{
		switch (cullMode)
		{
		case CullMode::Front:
			return VK_CULL_MODE_FRONT_BIT;
		case CullMode::Back:
			return VK_CULL_MODE_BACK_BIT;
		case CullMode::None:
			return VK_CULL_MODE_NONE;
		default:
			LOG_WARN("cull mode default use: back");
			return VK_CULL_MODE_BACK_BIT;
		}
	}
	VkFrontFace userToVK(FrontFace frontFace)
	{
		switch (frontFace)
		{
		case FrontFace::Clockwise:
			return VK_FRONT_FACE_CLOCKWISE;
		case FrontFace::CounterClockwise:
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		default:
			LOG_WARN("front face default use: clockwise");
			return VK_FRONT_FACE_CLOCKWISE;
		}
	}
	VkSampleCountFlagBits userToVK(SampleCount sampleCount)
	{
		switch (sampleCount)
		{
		case SampleCount::e1:
			return VK_SAMPLE_COUNT_1_BIT;
		case SampleCount::e4:
			return VK_SAMPLE_COUNT_4_BIT;
		case SampleCount::e16:
			return VK_SAMPLE_COUNT_16_BIT;
		default:
			LOG_WARN("sample count default use: 1");
			return VK_SAMPLE_COUNT_1_BIT;
		}
	}
	VkBool32 userBoolToVK(bool user)
	{
		if (user)return VK_TRUE;
		return VK_FALSE;
	}
}
