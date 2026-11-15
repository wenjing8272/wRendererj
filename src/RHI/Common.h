#pragma once
#include<vulkan/vulkan.h>
#include"Types.h"

namespace wrj {
	VkBufferUsageFlags userToVK(BufferUsage bufferUsage);
	VkMemoryPropertyFlags userToVK(MemoryProperty memoryUsage);
	VkFormat userToVK(Format format);
	VkPrimitiveTopology userToVK(PrimitiveTopology topology);
}