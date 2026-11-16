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
		void initCommandList(VkDevice device,std::uint32_t graphicsQueueFamlily,VkCommandBuffer commandBuffer);
		void beginCommandBuffer();
		void beginRendering(VkImage swapchainImage, VkImageView swapchainImageView, Area2D renderArea);
		void bindGraphicsPipeline(VkPipeline pipeline);
		void setViewport(VkViewport viewport);
		void setScissor(VkRect2D rect);
		void endRendering(VkImage swapchainImage,VkImageView swapchainImageView,Area2D renderArea);
		VkCommandBuffer endCommandBuffer();
	private:
		void transitionBufferRecord(VkImage buffer,const transitionInfomation& transInfo);
		VkDevice m_device{ VK_NULL_HANDLE };
		std::uint32_t m_graphicsQueueFamlily{ 0 };

		VkCommandBuffer m_commandBuffer{ VK_NULL_HANDLE };
	};
}