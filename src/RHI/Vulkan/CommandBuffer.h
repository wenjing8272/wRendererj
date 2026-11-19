#pragma once
#include"RenderConfig.h"
#include"../Types.h"
#include<vulkan/vulkan.h>

#include<cstdint>
#include<array>

namespace RHI {
	class CommandBuffer
	{
	public:
		CommandBuffer();
		~CommandBuffer();
		CommandBuffer(const CommandBuffer& otherCommandBuffer) = delete;
		CommandBuffer& operator=(const CommandBuffer& otherCommandList) = delete;
		CommandBuffer(CommandBuffer&& otherCommandList) = delete;
		CommandBuffer& operator=(CommandBuffer&& otherCommandList) = delete;
		void initCommandBuffer(VkDevice device,std::uint32_t graphicsQueueFamlily,VkCommandBuffer commandBuffer);
		void beginCommandBuffer();
		void beginRendering(VkImage swapchainImage, VkImageView swapchainImageView, Area2D renderArea);
		void bindGraphicsPipeline(VkPipeline pipeline);
		void setViewport(VkViewport viewport);
		void setScissor(VkRect2D rect);
		void bindVertexBuffer(VkBuffer vertexBuffer);
		void draw();
		void endRendering(VkImage swapchainImage,VkImageView swapchainImageView,Area2D renderArea);
		VkCommandBuffer endCommandBuffer();
	private:
		void transitionBufferRecord(VkImage buffer,const transitionInfomation& transInfo);
		VkDevice m_device{ VK_NULL_HANDLE };
		std::uint32_t m_graphicsQueueFamlily{ 0 };

		VkCommandBuffer m_commandBuffer{ VK_NULL_HANDLE };
	};
}