#pragma once
#include "../Types.h"
#include "RenderConfig.h"

#include <VkBootstrap.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <cstdint>
#include <string>
#include <vector>

namespace RHI {
	// The Device object creates and owns the instance, surface, physicalDevice,
    // device, allocator, and queue info.
 	// The object has the longest lifetime — it must outlive all resource objects.
	class Device {
	public:
		
		struct Queue {
			//vulkan queue handle
			VkQueue handle{ VK_NULL_HANDLE };
			//the queue family index from which the queue was retrieved
			std::uint32_t family{ 0 };
			//Create the command pool for that queue family.
			VkCommandPool pool{ VK_NULL_HANDLE };
		};

		Device(const InitResource& initRes, const std::vector<std::string>& instanceExtensions);
		~Device();

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		Device(Device&&) = delete;
		Device& operator=(Device&&) = delete;

		VkInstance instance() const { return m_instance.instance; }
		VkSurfaceKHR surface() const { return m_surface; }
		VkDevice get() const { return m_device.device; }
		VmaAllocator allocator() const { return m_allocator; }

		const vkb::Device& vkbDevice() const { return m_device; }
		const vkb::PhysicalDevice& physicalDevice() const { return m_physicalDevice; }

		const Queue& graphics() const { return m_graphics; }
		const Queue& transfer() const { return m_transfer; }
		const Queue& compute() const { return m_compute; }

	private:
		Queue createQueue(vkb::QueueType type, VkCommandPoolCreateFlags poolFlags);
		void destroyQueue(Queue& queue);
		void destory();

		vkb::Instance m_instance;
		
		VkSurfaceKHR m_surface{ VK_NULL_HANDLE };
		vkb::PhysicalDevice m_physicalDevice;
		vkb::Device m_device;

		VmaAllocator m_allocator{ VK_NULL_HANDLE };

		Queue m_graphics;
		Queue m_transfer;
		Queue m_compute;
	};
}
