#pragma once
#include"Types.h"
#include<VkBootstrap.h>
#include<vulkan/vulkan.h>

#include<string>
#include<vector>
#include<memory>

namespace wrj {
	class FrameSync;

	class RenderContext {
	public:
		RenderContext();
		~RenderContext();
		RenderContext(const RenderContext& otherRenderContext) = delete;
		RenderContext& operator=(const RenderContext& otherRenderContext) = delete;
		RenderContext(RenderContext&& otherRenderContext) = delete;
		RenderContext& operator=(RenderContext&& otherRenderContext) = delete;
		void initRenderContext(const NativeResources& native,const std::vector<std::string>& instanceExtensions);
		void recreateSwapchain(std::pair<uint32_t, uint32_t> extent);

	private:
		vkb::Instance m_instance;
		// 必须初始化为空句柄：initRenderContext 中途失败时析构函数仍会执行销毁逻辑
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		vkb::PhysicalDevice m_physicalDevice;
		vkb::Device m_device;
		vkb::Swapchain m_swapchain;
		// 这些 VkImage 归交换链所有，不要手动销毁
		std::vector<VkImage> m_swapchainImages;
		// 这些 VkImageView 由我们创建，必须在销毁交换链之前释放
		std::vector<VkImageView> m_swapchainImageViews;

		VkQueue m_graphicsQueue = VK_NULL_HANDLE;
		VkQueue m_transferQueue = VK_NULL_HANDLE;
		VkQueue m_computeQueue = VK_NULL_HANDLE;

		std::unique_ptr<FrameSync> m_frameSync = nullptr;
	};
}