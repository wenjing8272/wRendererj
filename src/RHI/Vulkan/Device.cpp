// Platform-specific Vulkan definitions must be enabled before any Vulkan header:
// VkWin32SurfaceCreateInfoKHR / vkCreateWin32SurfaceKHR are only exposed by
// vulkan.h when VK_USE_PLATFORM_WIN32_KHR is defined.
#define VK_USE_PLATFORM_WIN32_KHR
#include <Windows.h>

// 
#include "Device.h"
#include "Common.hpp"

#include <iostream>
#include <stdexcept>

namespace RHI {

	// Use the vk-bootstrap library to cut out the tedious initialization boilerpate
	// Use the VMA (Vulkan Memory Allocator) library to manage memory resources.
	Device::Device(const InitResource& initRes, const std::vector<std::string>& instanceExtensions) {
		// Guarantee the object is either fully constructed or not constructed at all,
  		// so a mid-construction exception can't leave it half-built. C++ only calls
  		// the destructor when the exception happens after construction has completed.
		try{
			// Create the instance, enable validation layers in Debug builds,
			// and use vk-bootstrap's default message callback.
			vkb::InstanceBuilder instanceBuilder;
			instanceBuilder.set_app_name(initRes.appName)
				.set_engine_name(initRes.appName)
				.require_api_version(1, 3, 0)
				.enable_extensions(instanceExtensions);
#ifndef NDEBUG
			instanceBuilder.enable_validation_layers().use_default_debug_messenger();
#endif // !NDEBUG
			auto instanceResult = instanceBuilder.build();
			if (!instanceResult) {
				throw std::runtime_error("Failed to create Vulkan instance: " + instanceResult.error().message());
			}
			//Takes the intance out
			m_instance = instanceResult.value();

			// Create a VkSurfaceKHR from the native window handle.
			if (initRes.native.display == nullptr || initRes.native.window == nullptr) {
				throw std::runtime_error("Invalid native resources provided");
			}
			VkWin32SurfaceCreateInfoKHR surfaceInfo{};
			surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surfaceInfo.hinstance = static_cast<HINSTANCE>(initRes.native.display);
			surfaceInfo.hwnd = static_cast<HWND>(initRes.native.window);
			// Create m_surface via vkCreateWin32SurfaceKHR.
			VkResult surfaceResult =
				vkCreateWin32SurfaceKHR(m_instance, &surfaceInfo, nullptr, &m_surface);
			if (surfaceResult != VK_SUCCESS) {
				throw std::runtime_error("Failed to create Win32 surface, VkResult = " +
					std::to_string(surfaceResult));
			}
			
			// Select the physical device (GPU).
			
			// Enable the required extensions: only timeline semaphores (Vulkan 1.2)
			// plus dynamic rendering and synchronization2 (Vulkan 1.3).
			VkPhysicalDeviceVulkan12Features vulkan12{};
			vulkan12.timelineSemaphore = true;

			VkPhysicalDeviceVulkan13Features vulkan13{};
			vulkan13.dynamicRendering = true;
			vulkan13.synchronization2 = true;

			auto physicalResult = vkb::PhysicalDeviceSelector(m_instance, m_surface)
				.set_minimum_version(1, 3)
				.set_required_features_12(vulkan12)
				.set_required_features_13(vulkan13)
				.select();
			if (!physicalResult) {
				throw std::runtime_error("Failed to select physical device: " +
					physicalResult.error().message());
			}
			m_physicalDevice = physicalResult.value();
			std::cout << "GPU: " << m_physicalDevice.properties.deviceName << std::endl;

			// Create the logical device (VkDevice).
			auto deviceResult = vkb::DeviceBuilder(m_physicalDevice).build();
			if (!deviceResult) {
				throw std::runtime_error("Failed to create logical device: " +
					deviceResult.error().message());
			}
			m_device = deviceResult.value();

			DebugName::init(m_device);

			// Creating the VMA allocator requires the Vulkan instance, physical device,
			// and logical device — so create it after all three are ready.
			VmaAllocatorCreateInfo allocatorInfo{};
			allocatorInfo.instance = m_instance;
			allocatorInfo.physicalDevice = m_physicalDevice;
			allocatorInfo.device = m_device;
			allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
			if (vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS) {
				throw std::runtime_error("vmaCreateAllocator failed");
			}

			// Retrieve the queue and create the command pool.
			// VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT lets each command buffer allocated from the pool be reset individually.
			// VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,Hint to the driver to optimize the memory allocation strategy within the pool.
			m_graphics = createQueue(vkb::QueueType::graphics,
				VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
			m_transfer = createQueue(vkb::QueueType::transfer,
				VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
			m_compute = createQueue(vkb::QueueType::compute,
				VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		}
		catch(...)
		{
			destory();
			throw;
		}
	}

	// The destructor only runs after construction succeeds. If an exception is
    // thrown mid-construction, members already created won't be destroyed
    // automatically,every handle needs a null check.
	Device::~Device() {
		destory();
	}

	void Device::destory()
	{
		destroyQueue(m_graphics);
		destroyQueue(m_transfer);
		destroyQueue(m_compute);
		if (m_allocator != VK_NULL_HANDLE) {
			vmaDestroyAllocator(m_allocator);
			m_allocator = VK_NULL_HANDLE;
		}
		if (m_device.device != VK_NULL_HANDLE) {
			vkb::destroy_device(m_device);
			m_device.device = VK_NULL_HANDLE;
		}
		if (m_surface != VK_NULL_HANDLE && m_instance.instance != VK_NULL_HANDLE) {
			vkb::destroy_surface(m_instance, m_surface);
			m_surface = VK_NULL_HANDLE;
		}
		if (m_instance.instance != VK_NULL_HANDLE) {
			vkb::destroy_instance(m_instance);
			m_instance.instance = VK_NULL_HANDLE;
		}
	}

	//
	Device::Queue Device::createQueue(vkb::QueueType type, VkCommandPoolCreateFlags poolFlags) {
		auto queueResult = m_device.get_queue(type);
		if (!queueResult) {
			throw std::runtime_error("Failed to get queue");
		}
		auto indexResult = m_device.get_queue_index(type);
		if (!indexResult) {
			throw std::runtime_error("Failed to get queue family index");
		}

		Queue queue{};
		queue.handle = queueResult.value();
		queue.family = indexResult.value();

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queue.family;
		poolInfo.flags = poolFlags;
		if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &queue.pool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create command pool");
		}
		return queue;
	}

	// Destroy the command pool and null out the handle.
	void Device::destroyQueue(Queue& queue) {
		if (queue.pool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(m_device, queue.pool, nullptr);
			queue.pool = VK_NULL_HANDLE;
		}
	}
}
