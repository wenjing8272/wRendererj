#pragma once
#include"RenderConfig.h"
#include"../Types.h"
#include<vulkan/vulkan.h>

#include<cstdint>

namespace RHI {
	class ResourcesPool;

	class Pipeline;

	// 单个 VkCommandBuffer 的录制接口。
	// 生命周期由 RenderContext 按 frame-in-flight 槽位持有，这里只做录制和句柄解析。
	class CommandList
	{
	public:
		CommandList(VkDevice device, std::uint32_t graphicsQueueFamily, VkCommandBuffer commandBuffer,
					ResourcesPool& resources, Pipeline& pipelines);
		CommandList(const CommandList& otherCommandList) = delete;
		CommandList& operator=(const CommandList& otherCommandList) = delete;
		CommandList(CommandList&& otherCommandList) = delete;
		CommandList& operator=(CommandList&& otherCommandList) = delete;

		void begin();
		void beginRendering(VkImage image, VkImageView imageView, Area2D renderArea);
		void bindPipeline(PipelineHandle pipelineHandle);
		void setViewport(Viewport viewport);
		void setScissor(Area2D area);
		void bindVertexBuffer(BufferHandle bufferHandle);
		void bindIndexBuffer(BufferHandle bufferHandle);
		void draw(std::uint32_t vertexCount, std::uint32_t instanceCount, std::uint32_t firstVertex, std::uint32_t firstInstance);
		void drawIndex(std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t firstIndex, std::uint32_t vertexOffset, std::uint32_t firstInstance);
		void endRendering(VkImage image);
		VkCommandBuffer end();
	private:
		void transitionImage(VkImage image, const transitionInfomation& transInfo);
		VkDevice m_device{ VK_NULL_HANDLE };
		std::uint32_t m_graphicsQueueFamily{ 0 };

		VkCommandBuffer m_commandBuffer{ VK_NULL_HANDLE };
		ResourcesPool* m_resources{ nullptr };
		Pipeline* m_pipelines{ nullptr };
	};
}
