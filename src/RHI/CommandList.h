#pragma once
#include"RenderConfig.h"
#include"Types.h"
#include<vulkan/vulkan.h>

#include<cstdint>
#include<array>

namespace wrj {
	class CommandList
	{
	public:
		CommandList();
		~CommandList();
		CommandList(const CommandList& otherCommandList) = delete;
		CommandList& operator=(const CommandList& otherCommandList) = delete;
		CommandList(CommandList&& otherCommandList) = delete;
		CommandList& operator=(CommandList&& otherCommandList) = delete;
		void initCommandList(VkDevice device,std::uint32_t graphicsQueueFamlily);
		void createCommandBuffers();
		void beginCommandBuffer(std::uint32_t frameIndex);
		void recordCommand(std::uint32_t frameIndex,VkImage swapchainImage,VkImageView swapchainImageView,Area2D renderArea);
		VkCommandBuffer endCommandBuffer(std::uint32_t frameIndex);
	private:
		void transitionBufferRecord(std::uint32_t frameIndex,VkImage buffer,const transitionInfomation& transInfo);
		VkDevice m_device{ VK_NULL_HANDLE };
		std::uint32_t m_graphicsQueueFamlily{ 0 };
		VkCommandPool m_commandPool{ VK_NULL_HANDLE };
		std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> m_commandBuffers{ VK_NULL_HANDLE };
	};
}