#pragma once
#include<vulkan/vulkan.h>
#include"Types.h"

namespace wrj {
	VkBufferUsageFlags userToVK(BufferUsage bufferUsage);
	VkMemoryPropertyFlags userToVK(MemoryProperty memoryUsage);
}