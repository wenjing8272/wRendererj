#include"CommandList.h"
#include"ResourcesPool.h"
#include"Pipeline.h"

#include<stdexcept>

namespace RHI {
	void CommandList::init(VkDevice device, std::uint32_t graphicsQueueFamily, VkCommandBuffer commandBuffer,
		ResourcesPool& resources, Pipeline& pipelines)
	{
		m_device = device;
		m_graphicsQueueFamily = graphicsQueueFamily;
		m_commandBuffer = commandBuffer;
		m_resources = &resources;
		m_pipelines = &pipelines;
	}
	void CommandList::begin()
	{
		vkResetCommandBuffer(m_commandBuffer, 0);
		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(m_commandBuffer, &commandBufferBeginInfo);
	}
	void CommandList::beginRendering(VkImage image, VkImageView imageView, Area2D renderArea)
	{
		transitionImage(image, { VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						 VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
						 VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT });

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

		VkRenderingAttachmentInfo colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.clearValue = clearColor;
		colorAttachment.imageView = imageView;
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea.offset = { renderArea.offset.x, renderArea.offset.y };
		renderingInfo.renderArea.extent = { renderArea.extent.width, renderArea.extent.height };
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;

		vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
	}
	void CommandList::bindPipeline(PipelineHandle pipelineHandle)
	{
		if (!m_pipelines->pipelineIsVaild(pipelineHandle))
		{
			throw std::runtime_error("pipeline no vaild");
		}
		vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipelines->getPipeline(pipelineHandle));
	}
	void CommandList::setViewport(Viewport viewport)
	{
		VkViewport vkViewport{ viewport.x, viewport.y, viewport.width, viewport.height,
			viewport.minDepth, viewport.maxDepth };
		vkCmdSetViewport(m_commandBuffer, 0, 1, &vkViewport);
	}
	void CommandList::setScissor(Area2D area)
	{
		VkRect2D rect{ { area.offset.x, area.offset.y }, { area.extent.width, area.extent.height } };
		vkCmdSetScissor(m_commandBuffer, 0, 1, &rect);
	}
	void CommandList::bindVertexBuffer(BufferHandle bufferHandle)
	{
		if (!m_resources->isBufferVaild(bufferHandle))
		{
			throw std::runtime_error("vertex buffer no vaild");
		}
		VkBuffer buffer = m_resources->getBuffer(bufferHandle);
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, &buffer, &offset);
	}
	void CommandList::draw()
	{
		vkCmdDraw(m_commandBuffer, 3, 1, 0, 0);
	}
	void CommandList::endRendering(VkImage image)
	{
		vkCmdEndRendering(m_commandBuffer);

		transitionImage(image, { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
						 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_NONE,
						 VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_NONE });
	}
	VkCommandBuffer CommandList::end()
	{
		vkEndCommandBuffer(m_commandBuffer);
		return m_commandBuffer;
	}
	void CommandList::transitionImage(VkImage image, const transitionInfomation& transInfo)
	{
		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.image = image;
		barrier.oldLayout = transInfo.oldLayout;
		barrier.newLayout = transInfo.newLayout;
		barrier.srcAccessMask = transInfo.srcAccessMask;
		barrier.dstAccessMask = transInfo.dstAccessMask;
		barrier.srcStageMask = transInfo.srcPipelineMask;
		barrier.dstStageMask = transInfo.dstPipelineMask;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;

		VkDependencyInfo dependencyInfo{};
		dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependencyInfo.imageMemoryBarrierCount = 1;
		dependencyInfo.pImageMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
	}
}
