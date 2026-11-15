#pragma once
#include<vulkan/vulkan.h>
#include"Types.h"

namespace wrj {
	VkBufferUsageFlags userToVK(BufferUsage bufferUsage);
	VkMemoryPropertyFlags userToVK(MemoryProperty memoryUsage);
	VkFormat userToVK(Format format);
	VkPrimitiveTopology userToVK(PrimitiveTopology topology);
	VkDynamicState userToVK(DynamicState dynamic);
	VkCullModeFlags userToVK(CullMode cullMode);
	VkFrontFace userToVK(FrontFace frontFace);
	VkSampleCountFlagBits userToVK(SampleCount sampleCount);
	VkBool32 userBoolToVK(bool user);
}