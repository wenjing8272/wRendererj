#include"CommandBuffer.h"

namespace RHI {
	CommandBuffer::CommandBuffer() = default;
	CommandBuffer::~CommandBuffer()
	{
	}
	void CommandBuffer::initCommandBuffer(VkDevice device, std::uint32_t graphicsQueueFamlily,VkCommandBuffer commandBuffer)
	{
		m_device = device;
		m_graphicsQueueFamlily = graphicsQueueFamlily;
		m_commandBuffer = commandBuffer;
	}
	void CommandBuffer::beginCommandBuffer()
	{
		vkResetCommandBuffer(m_commandBuffer, 0);
		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(m_commandBuffer, &commandBufferBeginInfo);
	}
	void CommandBuffer::beginRendering(VkImage swapchainImage, VkImageView swapchainImageView, Area2D renderArea)
	{
		transitionBufferRecord(swapchainImage, { VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
										 VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
										 VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT });

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

		VkRenderingAttachmentInfo colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.clearValue = clearColor;
		colorAttachment.imageView = swapchainImageView;
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
	void CommandBuffer::bindGraphicsPipeline(VkPipeline pipeline)
	{
		vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}
	void CommandBuffer::setViewport(VkViewport viewport)
	{
		vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
	}
	void CommandBuffer::setScissor(VkRect2D rect)
	{
		vkCmdSetScissor(m_commandBuffer, 0, 1, &rect);
	}
	void CommandBuffer::bindVertexBuffer(VkBuffer vertexBuffer)
	{
		VkDeviceSize size = 0;
		vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, &vertexBuffer, &size);
	}
	void CommandBuffer::draw()
	{
		vkCmdDraw(m_commandBuffer, 3, 1, 0, 0);
	}
	void CommandBuffer::endRendering( VkImage swapchainImage,VkImageView swapchainImageView, Area2D renderArea)
	{
		
		vkCmdEndRendering(m_commandBuffer);

		transitionBufferRecord(swapchainImage, { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
												 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_NONE,
												 VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_NONE });

	}
	VkCommandBuffer CommandBuffer::endCommandBuffer()
	{
		vkEndCommandBuffer(m_commandBuffer);
		return m_commandBuffer;
	}
	void CommandBuffer::transitionBufferRecord(VkImage buffer,const transitionInfomation& transInfo)
	{
		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.image = buffer;
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
