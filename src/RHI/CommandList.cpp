#include"CommandList.h"

namespace wrj {
	CommandList::CommandList() = default;
	CommandList::~CommandList()
	{
		vkDeviceWaitIdle(m_device);
		if (m_commandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(m_device, m_commandPool, nullptr);
		}
	}
	void CommandList::initCommandList(VkDevice device, std::uint32_t graphicsQueueFamlily)
	{
		m_device = device;
		m_graphicsQueueFamlily = graphicsQueueFamlily;
		VkCommandPoolCreateInfo commandPoolCreateInfo{};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = m_graphicsQueueFamlily;

		vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_commandPool);
	}
	void CommandList::createCommandBuffers()
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.commandPool = m_commandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = static_cast<std::uint32_t>(m_commandBuffers.size());
		vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, m_commandBuffers.data());
	}
	void CommandList::beginCommandBuffer(std::uint32_t frameIndex)
	{
		vkResetCommandBuffer(m_commandBuffers[frameIndex], 0);
		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(m_commandBuffers[frameIndex], &commandBufferBeginInfo);
	}
	void CommandList::recordCommand(std::uint32_t frameIndex, VkImage swapchainImage,VkImageView swapchainImageView, Area2D renderArea)
	{
		transitionBufferRecord(frameIndex, swapchainImage, { VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT });
		
		VkClearValue clearColor = { 1.0f, 1.0f, 1.0f, 1.0f };

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

		vkCmdBeginRendering(m_commandBuffers[frameIndex], &renderingInfo);
		
		vkCmdEndRendering(m_commandBuffers[frameIndex]);

		transitionBufferRecord(frameIndex, swapchainImage, { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_NONE,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_NONE });

	}
	VkCommandBuffer CommandList::endCommandBuffer(std::uint32_t frameIndex)
	{
		vkEndCommandBuffer(m_commandBuffers[frameIndex]);
		return m_commandBuffers[frameIndex];
	}
	void CommandList::transitionBufferRecord(std::uint32_t frameIndex,VkImage buffer,const transitionInfomation& transInfo)
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

		vkCmdPipelineBarrier2(m_commandBuffers[frameIndex], &dependencyInfo);
	}
}