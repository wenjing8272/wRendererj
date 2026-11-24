#pragma once
#include"../Types.h"
#include<cstdint>
#include<vulkan/vulkan.h>

namespace RHI {
	constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	class CommandList;

	// 一帧的数据：同步对象由 FrameSync 填，渲染目标和录制入口由 RenderContext 填
	struct Frame
	{
		std::uint32_t frameIndex{ 0 };
		std::uint32_t imageIndex{ 0 };
		std::uint64_t signalValue{ 0 };
		VkSemaphore imageAvailable{ VK_NULL_HANDLE };
		VkSemaphore renderFinished{ VK_NULL_HANDLE };
		VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };

		VkImage image{ VK_NULL_HANDLE };
		VkImageView imageView{ VK_NULL_HANDLE };
		Area2D area{};
		CommandList* commandList{ nullptr };
	};
	struct transitionInfomation
	{
		VkImageLayout oldLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkImageLayout newLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkAccessFlags srcAccessMask{};
		VkAccessFlags dstAccessMask{};
		VkPipelineStageFlags srcPipelineMask{};
		VkPipelineStageFlags dstPipelineMask{};
	};
}