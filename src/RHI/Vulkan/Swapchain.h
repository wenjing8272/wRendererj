#pragma once
#include "../Types.h"

#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <optional>
#include <vector>

namespace RHI {
	class Device;

	// Window object: owns the VkSwapchainKHR, the swapchain images, the image views,
	// and one renderFinished semaphore per image.
	// To recreate the swapchain, construct a new object from the old handles and
	// let the old one be destroyed.
	class Swapchain {
	public:
		enum class AcquireResult{
			Success,
			OutOfDate
		};

		Swapchain(const Device& device,
			std::optional<Extent2D> desiredExtent = std::nullopt,
			VkSwapchainKHR old = VK_NULL_HANDLE);
		~Swapchain();

		Swapchain(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;
		Swapchain(Swapchain&&) = delete;
		Swapchain& operator=(Swapchain&&) = delete;

		void setDirty(){dirty = true;}

		VkSwapchainKHR get() const { return m_swapchain.swapchain; }
		Extent2D extent() const { return { m_swapchain.extent.width, m_swapchain.extent.height }; }
		Area2D area() const { return { {0, 0}, extent() }; }
		VkFormat format() const { return m_swapchain.image_format; }

		std::uint32_t imageCount() const { return static_cast<std::uint32_t>(m_images.size()); }
		VkImage image(std::uint32_t index) const { return m_images[index]; }
		VkImageView view(std::uint32_t index) const { return m_views[index]; }
		VkSemaphore renderFinished(std::uint32_t index) const { return m_renderFinished[index]; }

		AcquireResult acquire(VkSemaphore imageAvailable, std::uint32_t& imageIndex) const;
		bool present(std::uint32_t imageIndex,VkQueue queue) const;

	private:
		void destroy();

		bool dirty {false};

		const Device* m_device{ nullptr };
		vkb::Swapchain m_swapchain;
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_views;
		std::vector<VkSemaphore> m_renderFinished;
	};
}
