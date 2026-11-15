#pragma once
#include"RenderConfig.h"
#include"Types.h"
#include<VkBootstrap.h>
#include<vulkan/vulkan.h>
#include<vma/vk_mem_alloc.h>

#include<string>
#include<vector>
#include<memory>
#include<array>
#include<optional>

namespace wrj {
	class FrameSync;

	class CommandList;

	class ResourcesPool;

	class Upload;

	class Pipeline;

	class RenderContext {
	public:
		RenderContext();
		~RenderContext();
		RenderContext(const RenderContext& otherRenderContext) = delete;
		RenderContext& operator=(const RenderContext& otherRenderContext) = delete;
		RenderContext(RenderContext&& otherRenderContext) = delete;
		RenderContext& operator=(RenderContext&& otherRenderContext) = delete;
		void initRenderContext(const InitResource& initRes,const std::vector<std::string>& instanceExtensions);
		void recreateSwapchain(Extent2D extent);

		BufferHandle createBuffer(std::uint64_t size,BufferUsage bufferUsage,MemoryProperty memoryProperty);
		void createImage();

		void copyBuffer(BufferHandle copyBuffer, const void* data, std::uint64_t dataSize);
		void uploadBuffer(BufferHandle srcBufferHandle, BufferHandle dstBufferHandle,std::uint64_t bufferSize);

		ShaderHandle createShader(const std::vector<std::uint32_t>& shaderCode);

		void begin(bool& swapchainDirty);
		void beginRendering();
		void endRendering();
		void end(bool& swapchainDirty);
	private:
		void initDevice(const InitResource& initRes,const std::vector<std::string>& instanceExtensions);

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
		Area2D m_swapchainArea;

		VmaAllocator m_allocator{ VK_NULL_HANDLE };

		std::uint32_t m_graphicsQueueFamlily{ 0 };
		std::uint32_t m_computeQueueFamlily{ 0 };
		std::uint32_t m_transferQueueFamlily{ 0 };
		VkQueue m_graphicsQueue{ VK_NULL_HANDLE };
		VkQueue m_transferQueue{ VK_NULL_HANDLE };
		VkQueue m_computeQueue{ VK_NULL_HANDLE };

		VkCommandPool m_commandPool{ VK_NULL_HANDLE };
		VkCommandPool m_transferPool{ VK_NULL_HANDLE };
		VkCommandPool m_computePool{ VK_NULL_HANDLE };

		std::unique_ptr<FrameSync> m_frameSync{ nullptr };
		std::optional<FrameResources> m_currentFrame;

		std::vector<std::unique_ptr<CommandList>> m_commandLists;
		
		std::unique_ptr<ResourcesPool> m_resourcesPool{ nullptr };

		std::unique_ptr<Upload> m_upload{ nullptr };

		std::unique_ptr<Pipeline>m_pipeline{ nullptr };
	};
}