// 平台相关的 Vulkan 定义必须在任何 Vulkan 头文件之前启用：
// VkWin32SurfaceCreateInfoKHR / vkCreateWin32SurfaceKHR 在
// VK_USE_PLATFORM_WIN32_KHR 打开时才会被 vulkan.h 引入
#define VK_USE_PLATFORM_WIN32_KHR
#include<Windows.h>

// RenderContext.h 里包含 VkBootstrap.h和完整的 vulkan.h。
// 上面的宏必须先于这个包含，vulkan.h 才会再引入 vulkan_win32.h。
#include"RenderContext.h"
#include"FrameSync.h"
#include"CommandList.h"
#include"ResourcesPool.h"
#include"Upload.h"
#include"Pipeline.h"
#include"Common.h"
#ifndef NDEBUG
#include"SetDebugName.h"
#endif // !NDEBUG

#include<iostream>

namespace wrj {
	RenderContext::RenderContext() = default;
	// 销毁顺序必须是子对象早于它的创建者：
	//   image views -> swapchain -> device -> surface -> instance
	// 其中 m_swapchainImages由 swapchain 隐式创建并拥有，
	// 销毁 swapchain 时自动释放
	RenderContext::~RenderContext()
	{
		vkDeviceWaitIdle(m_device);
		m_pipeline.reset();
		std::cout << "renderContext" << "\n";
		m_upload.reset();
		m_frameSync.reset();
		for (auto& commandList : m_commandLists)
		{
			commandList.reset();
		}
		m_resourcesPool.reset();
		vmaDestroyAllocator(m_allocator);
		vkDestroyCommandPool(m_device, m_commandPool, nullptr);
		vkDestroyCommandPool(m_device, m_transferPool, nullptr);
		vkDestroyCommandPool(m_device,m_computePool, nullptr);
		m_swapchain.destroy_image_views(m_swapchainImageViews);
		vkb::destroy_swapchain(m_swapchain);
		vkb::destroy_device(m_device);
		vkb::destroy_surface(m_instance, m_surface);
		vkb::destroy_instance(m_instance);
	}

	void RenderContext::initRenderContext(const InitResource& initRes,const std::vector<std::string>& instanceExtensions)
	{
		initDevice(initRes, instanceExtensions);

		m_frameSync = std::make_unique<FrameSync>();
		m_frameSync->initFrameSync(m_device, m_graphicsQueue, static_cast<std::uint32_t>(m_swapchainImages.size()));

		std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> buffers{};

		VkCommandBufferAllocateInfo commandBufferInfo{};
		commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
		commandBufferInfo.commandPool = m_commandPool;
		commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		if (vkAllocateCommandBuffers(m_device, &commandBufferInfo, buffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("vkAllocateCommandBuffers failed");
		}

		m_commandLists.resize(MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0;i < MAX_FRAMES_IN_FLIGHT;++i)
		{
			m_commandLists[i] = std::make_unique<CommandList>();
			m_commandLists[i]->initCommandList(m_device, m_graphicsQueueFamlily, buffers[i]);
		}

		m_resourcesPool = std::make_unique<ResourcesPool>();
		m_resourcesPool->init(m_device, m_commandPool, m_graphicsQueue,m_allocator);

		m_upload = std::make_unique<Upload>();
		m_upload->init(m_device,m_transferPool, m_transferQueue);

		m_pipeline = std::make_unique<Pipeline>();
		m_pipeline->init(m_device);
	}

	void RenderContext::initDevice(const InitResource& initRes,const std::vector<std::string>& instanceExtensions)
	{
		// ---------- 1. 创建 VkInstance ----------
		// Instance 是整个 Vulkan 上下文的根对象，surface、physical device、device
		// 都挂在它下面。require_api_version(1,3,0) 表示我们只使用 1.3 的核心能力
		// （dynamic rendering、synchronization2 等，但它们是否启用还要单独开 feature）。
		vkb::InstanceBuilder instanceBuilder;
		instanceBuilder.set_app_name(initRes.appName)
			.set_engine_name(initRes.appName)
			.require_api_version(1, 3, 0)
			.enable_extensions(instanceExtensions);
#ifndef NDEBUG
		instanceBuilder.enable_validation_layers().use_default_debug_messenger();
#endif // !NDEBUG
		// vkb 的 build()/select() 统一返回 Result<T>：失败时内部存的是 Error，
		// 此时调用 .value() 会抛 std::bad_variant_access，所以必须先判 !result。
		auto instanceResult = instanceBuilder.build();
		if (!instanceResult)
		{
			throw std::runtime_error("Failed to create Vulkan instance: " + instanceResult.error().message());
		}
		m_instance = instanceResult.value();

		// ---------- 2. 用原生窗口句柄创建 VkSurfaceKHR ----------
		// vk-bootstrap 不接收原生句柄，只接收已经创建好的 VkSurfaceKHR。
		// NativeResources 用 void* 保存两个平台句柄：
		//   display -> HINSTANCE（GetModuleHandle），Win32 surface 所属的模块实例
		//   window  -> HWND（glfwGetWin32Window），实际窗口
		// sType 才是结构体类型标识，flags 是保留字段（必须为 0），不要填反。
		if (initRes.native.display == nullptr || initRes.native.window == nullptr)
		{
			throw std::runtime_error("Invalid native resources provided");
		}
		VkWin32SurfaceCreateInfoKHR surfaceInfo{};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.hinstance = static_cast<HINSTANCE>(initRes.native.display);
		surfaceInfo.hwnd = static_cast<HWND>(initRes.native.window);
		VkResult surfaceResult = vkCreateWin32SurfaceKHR(m_instance, &surfaceInfo, nullptr, &m_surface);
		// VkResult 是枚举，只有 0（VK_SUCCESS）代表成功，必须显式比较；
		// 写成 if (!surfaceResult) 会把成功当成失败、把失败当成成功。
		if (surfaceResult != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Win32 surface, VkResult = " + std::to_string(surfaceResult));
		}

		//
		VkPhysicalDeviceVulkan12Features vulkan12{};
		vulkan12.timelineSemaphore = true;

		VkPhysicalDeviceVulkan13Features vulkan13{};
		vulkan13.dynamicRendering = true;
		vulkan13.synchronization2 = true;
		// ---------- 3. 选择物理设备（GPU） ----------
		// 传入 surface 后，selector 会检查哪些队列族支持向该 surface 呈现，
		// 并以此过滤设备；set_minimum_version(1,3) 要求设备支持 Vulkan 1.3。
		// 这里没有声明额外的扩展和 feature，因此 device 只拥有 1.3 核心能力中
		// 显式开启的部分（默认全为 false，包括 dynamicRendering/synchronization2）。
		auto physicalResult = vkb::PhysicalDeviceSelector(m_instance, m_surface)
			.set_minimum_version(1, 3)
			.set_required_features_12(vulkan12)
			.set_required_features_13(vulkan13)
			.select();
		if (!physicalResult)
		{
			throw std::runtime_error("Failed to select physical device: " + physicalResult.error().message());
		}
		m_physicalDevice = physicalResult.value();
		std::cout << "GPU: " << m_physicalDevice.properties.deviceName << std::endl;

		// ---------- 4. 创建逻辑设备 VkDevice ----------
		// DeviceBuilder 在 vkCreateDevice 时会：
		//   - 为物理设备的每个队列族各创建 1 个队列（优先级 1.0）；
		//   - 因为 selector 带了 surface，自动启用 VK_KHR_swapchain；
		//   - 把 selector 中声明的扩展和 feature 结构一并带上。
		// 需要队列句柄时通过 m_device.get_queue(vkb::QueueType::graphics) 获取。
		auto deviceResult = vkb::DeviceBuilder(m_physicalDevice)
			.build();
		if (!deviceResult)
		{
			throw std::runtime_error("Failed to create logical device: " + deviceResult.error().message());
		}
		m_device = deviceResult.value();

		//初始化配置debug name函数，在DebugName里面创建配置PFN_vkSetDebugUtilsObjectNameEXT函数指针
		DebugName::init(m_device);

		//创建vma allocator
		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.instance = m_instance;
		allocatorInfo.physicalDevice = m_physicalDevice;
		allocatorInfo.device = m_device;
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		if (vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS) {
			throw std::runtime_error("vmaCreateAllocator failed");
		}

		// ---------- 5. 创建交换链 ----------
		// SwapchainBuilder 从 Device 里取出 surface 和队列信息，默认选择 surface
		// 支持的格式、颜色空间和 present mode。窗口尺寸变化时需要重建：
		// 先销毁旧 image views，再用 set_old_swapchain(...) 创建新链。
		auto swapchainResult = vkb::SwapchainBuilder(m_device).build();
		if (!swapchainResult)
		{
			throw std::runtime_error("Failed to create swapchain: " + swapchainResult.error().message());
		}
		m_swapchain = swapchainResult.value();

		// ---------- 6. 获取交换链图像 ----------
		// 这些 VkImage 由 vkCreateSwapchainKHR 隐式创建，归交换链所有，
		// 销毁 swapchain 时自动释放，不能对它们调用 vkDestroyImage。
		auto swapchainImagesResult = m_swapchain.get_images();
		if (!swapchainImagesResult)
		{
			throw std::runtime_error("Failed to get swapchain images: " + swapchainImagesResult.error().message());
		}
		m_swapchainImages = swapchainImagesResult.value();
		
		m_swapchainArea = { {0,0},{initRes.frameBufferWidth,initRes.frameBufferHeight} };
		// ---------- 7. 创建图像视图 ----------
		// 上面的 image 不同，VkImageView 是我们通过 vkCreateImageView 创建的，
		// 所有权在我们手里：必须在销毁 swapchain 之前用
		// swapchain.destroy_image_views(...) 释放（见析构函数）。
		auto swapchainImageViewsResult = m_swapchain.get_image_views();
		if (!swapchainImageViewsResult)
		{
			throw std::runtime_error("Failed to get swapchain image views: " + swapchainImageViewsResult.error().message());
		}
		m_swapchainImageViews = swapchainImageViewsResult.value();
		//graphics queue
		auto graphicsQueueResult = m_device.get_queue(vkb::QueueType::graphics);
		if (!graphicsQueueResult)
		{
			throw std::runtime_error("Failed to get graphics queue: " + graphicsQueueResult.error().message());
		}
		m_graphicsQueue = graphicsQueueResult.value();
		m_graphicsQueueFamlily = m_device.get_queue_index(vkb::QueueType::graphics).value();

		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.queueFamilyIndex = m_graphicsQueueFamlily;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

		if (vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphic command pool");
		}

		//transfer queue
		auto transferQueueResult = m_device.get_queue(vkb::QueueType::transfer);
		if (!transferQueueResult)
		{
			throw std::runtime_error("Failed to get transfer queue: " + transferQueueResult.error().message());
		}
		m_transferQueue = transferQueueResult.value();
		m_transferQueueFamlily = m_device.get_queue_index(vkb::QueueType::transfer).value();

		VkCommandPoolCreateInfo transferInfo{};
		transferInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		transferInfo.queueFamilyIndex = m_transferQueueFamlily;
		transferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

		if (vkCreateCommandPool(m_device, &transferInfo, nullptr, &m_transferPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create transfer command pool");
		}

		//compute queue
		auto computeQueueResult = m_device.get_queue(vkb::QueueType::compute);
		if (!computeQueueResult)
		{
			throw std::runtime_error("Failed to get compute queue: " + computeQueueResult.error().message());
		}
		m_computeQueue = computeQueueResult.value();
		m_computeQueueFamlily = m_device.get_queue_index(vkb::QueueType::compute).value();

		VkCommandPoolCreateInfo computeInfo{};
		computeInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		computeInfo.queueFamilyIndex = m_computeQueueFamlily;
		computeInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

		if (vkCreateCommandPool(m_device, &computeInfo, nullptr, &m_computePool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create compute command pool");
		}
	}
	void RenderContext::recreateSwapchain(Extent2D extent)
	{
		vkQueueWaitIdle(m_graphicsQueue);
		// 记录旧的交换链，销毁旧的 image views
		VkSwapchainKHR oldSwapchain = m_swapchain;
		m_swapchain.destroy_image_views(m_swapchainImageViews);
		m_swapchainImages.clear();
		// 创建新的交换链
		auto swapchainResult = vkb::SwapchainBuilder(m_device)
			.set_desired_extent(extent.width, extent.height)
			.set_old_swapchain(oldSwapchain)
			.build();
		if (!swapchainResult)
		{
			throw std::runtime_error("Failed to recreate swapchain: " + swapchainResult.error().message());
		}
		m_swapchain = swapchainResult.value();
		//清理旧交换链
		vkDestroySwapchainKHR(m_device, oldSwapchain, nullptr);
		// 获取新的交换链图像
		auto swapchainImagesResult = m_swapchain.get_images();
		if (!swapchainImagesResult)
		{
			throw std::runtime_error("Failed to get swapchain images: " + swapchainImagesResult.error().message());
		}
		m_swapchainImages = swapchainImagesResult.value();
		m_swapchainArea.extent = { extent.width, extent.height };
		// 创建新的图像视图
		auto swapchainImageViewsResult = m_swapchain.get_image_views();
		if (!swapchainImageViewsResult)
		{
			throw std::runtime_error("Failed to get swapchain image views: " + swapchainImageViewsResult.error().message());
		}
		m_swapchainImageViews = swapchainImageViewsResult.value();

		m_frameSync->recreateRenderingFinishedSemaphore(static_cast<std::uint32_t>(m_swapchainImages.size()));
	}
	BufferHandle RenderContext::createBuffer(std::uint64_t size, BufferUsage bufferUsage, MemoryProperty memoryProperty)
	{
		return m_resourcesPool->createBuffer(size, userToVK(bufferUsage), userToVK(memoryProperty));
	}
	void RenderContext::copyBuffer(BufferHandle copyBuffer, const void* data, std::uint64_t dataSize)
	{
		m_resourcesPool->copyBuffer(copyBuffer, data, dataSize);
	}
	void RenderContext::uploadBuffer(BufferHandle srcBufferHandle, BufferHandle dstBufferHandle,std::uint64_t bufferSize)
	{
		if (!m_resourcesPool->isBufferVaild(srcBufferHandle) || !m_resourcesPool->isBufferVaild(dstBufferHandle))
		{
			throw std::runtime_error("src or dst buffer no vaild");
		}
		m_upload->uploadBuffer(m_resourcesPool->getBuffer(srcBufferHandle), m_resourcesPool->getBuffer(dstBufferHandle),bufferSize);
	}
	ShaderHandle RenderContext::createShader(const std::vector<std::uint32_t>& shaderCode)
	{
		return m_pipeline->createShader(shaderCode);
	}
	PipelineHandle RenderContext::createGraphicsPipeline(const GraphicsPipeline& pipelineRes)
	{
		return m_pipeline->createGraphicsPipeline(pipelineRes,m_swapchain.image_format);
	}
	void RenderContext::begin(bool& swapchainDirty)
	{
		m_currentFrame = m_frameSync->frameBegin(m_swapchain);
		if (!m_currentFrame.has_value())
		{
			swapchainDirty = true;
			return;
		}
	}
	void RenderContext::beginRendering(RenderTarget renderTarget)
	{
		m_commandLists[m_currentFrame->frameIndex]->beginCommandBuffer();
		if (renderTarget == RenderTarget::Swapchain)
		{
			m_commandLists[m_currentFrame->frameIndex]->beginRendering(m_swapchainImages[m_currentFrame->imageIndex], m_swapchainImageViews[m_currentFrame->imageIndex], m_swapchainArea);
		}
		else if (renderTarget == RenderTarget::GeneralImage)
		{

		}
		else
		{
			throw std::runtime_error("must configure render target");
		}
	}
	void RenderContext::bindPipeline(PipelineHandle pipelineHandle)
	{
		if (!m_pipeline->pipelineIsVaild(pipelineHandle))
		{
			throw std::runtime_error("pipeline no vaild");
		}
		m_commandLists[m_currentFrame->frameIndex]->bindGraphicsPipeline(m_pipeline->getPipeline(pipelineHandle));
	}
	void RenderContext::setViewport(Viewport viewport)
	{
		m_commandLists[m_currentFrame->frameIndex]->setViewport({ viewport.x,
																  viewport.y,
																  viewport.width,
																  viewport.height,
																  viewport.minDepth,
																  viewport.maxDepth });
	}
	void RenderContext::setScissor(Area2D area)
	{
		m_commandLists[m_currentFrame->frameIndex]->setScissor({ .offset = {area.offset.x,area.offset.y},.extent = {area.extent.width,area.extent.height} });
	}
	void RenderContext::bindingVertexBuffer(BufferHandle bufferHandle)
	{
		if (!m_resourcesPool->isBufferVaild(bufferHandle))
		{
			throw std::runtime_error("vertex buffer no vaild");
		}
		m_commandLists[m_currentFrame->frameIndex]->bindVertexBuffer(m_resourcesPool->getBuffer(bufferHandle));
	}
	void RenderContext::draw()
	{
		m_commandLists[m_currentFrame->frameIndex]->draw();
	}
	void RenderContext::endRendering()
	{
		m_commandLists[m_currentFrame->frameIndex]->endRendering(m_swapchainImages[m_currentFrame->imageIndex], m_swapchainImageViews[m_currentFrame->imageIndex], m_swapchainArea);
		auto commandBuffer = m_commandLists[m_currentFrame->frameIndex]->endCommandBuffer();
		m_currentFrame->commandBuffer = commandBuffer;
	}
	void RenderContext::end(bool& swapchainDirty)
	{
		if (!m_frameSync->frameEnd(m_currentFrame.value(), m_swapchain))
		{
			swapchainDirty = true;
			return;
		}
	}
}
