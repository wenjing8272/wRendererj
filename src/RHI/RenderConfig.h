#pragma once
#include<cstdint>
#include<vulkan/vulkan.h>

namespace wrj {
	constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	struct FrameResources
	{
		std::uint32_t frameIndex{ 0 };
		std::uint32_t imageIndex{ 0 };
		std::uint64_t signalValue{ 0 };
		VkSemaphore imageAvailable = VK_NULL_HANDLE;
		VkSemaphore renderFinished = VK_NULL_HANDLE;
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	};
}