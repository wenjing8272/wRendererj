#include "Swapchain.h"
#include "Device.h"

#include <stdexcept>

namespace RHI {

	Swapchain::Swapchain(const Device& device, std::optional<Extent2D> desiredExtent, VkSwapchainKHR old)
		: m_device(&device) {
		//see Device::Device;
		try {
			vkb::SwapchainBuilder builder(device.vkbDevice());
			if (desiredExtent) {
				builder.set_desired_extent(desiredExtent->width, desiredExtent->height);
			}
			
            // By supplying the previous swapchain as oldSwapchain, you let the driver reuse the underlying resources of the old
			// swapchain rather than allocating everything from scratch.
			if (old != VK_NULL_HANDLE) {
				builder.set_old_swapchain(old);
			}
			auto swapchainResult = builder.build();
			if (!swapchainResult) {
				throw std::runtime_error("Failed to create swapchain: " +
					swapchainResult.error().message());
			}
			m_swapchain = swapchainResult.value();

			auto imagesResult = m_swapchain.get_images();
			if (!imagesResult) {
				throw std::runtime_error("Failed to get swapchain images: " +
					imagesResult.error().message());
			}
			m_images = imagesResult.value();

			auto viewsResult = m_swapchain.get_image_views();
			if (!viewsResult) {
				throw std::runtime_error("Failed to get swapchain image views: " +
					viewsResult.error().message());
			}
			m_views = viewsResult.value();

			// Allocate a render-finished semaphore for each swapchain image,
			// one per m_images.size() and create them.
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			m_renderFinished.assign(m_images.size(), VK_NULL_HANDLE);
			for (auto& semaphore : m_renderFinished) {
				if (vkCreateSemaphore(device.get(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {
					throw std::runtime_error("Failed to create render finished semaphore");
				}
			}
		}
		catch (...) 
		{
			destroy();
			throw;
		}
	}

	Swapchain::~Swapchain() {
		destroy();
	}

	void Swapchain::destroy() {
		for (auto& semaphore : m_renderFinished) {
			if (semaphore != VK_NULL_HANDLE) {
				vkDestroySemaphore(m_device->get(), semaphore, nullptr);
				semaphore = VK_NULL_HANDLE;
			}
		}
		//Destroy the swapchain image views before the swapchain. Swapchain images are driver-owned and must not be destroyed
		//manually,they're reclaimed automatically when the swapchain is destroyed.
		m_renderFinished.clear();
		if (!m_views.empty()) {
			m_swapchain.destroy_image_views(m_views);
			m_views.clear();
		}
		m_images.clear();
		if (m_swapchain.swapchain != VK_NULL_HANDLE) {
			vkb::destroy_swapchain(m_swapchain);
			m_swapchain.swapchain = VK_NULL_HANDLE;
		}
	}

	//Acquire the image → the presentation engine signals imageAvailable → your queue waits on it before rendering
	// @param imageAvailable the semaphore for the current frame's image-acquire
	// @param imageIndex vkAcquireNextImageKHR returns the index of the acquired swapchain image via pImageIndex.
	Swapchain::AcquireResult Swapchain::acquire(VkSemaphore imageAvailable,std::uint32_t& imageIndex) const {

		const auto result = vkAcquireNextImageKHR(m_device->get(), 
												  m_swapchain.swapchain,
												  UINT64_MAX, 
												  imageAvailable, 
												  VK_NULL_HANDLE, 
												  &imageIndex);

		//If result if VK_SUCCESS or VK_SUBOPTIMAL_KHR return AcquireResult::Success,
		//for any other result return AcquireResult::OutOfDate.
		//Since timeout is set to UINT64_MAX, VK_TIMEOUT is practically impossible.
		if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
			return AcquireResult::Success;
		}
		return AcquireResult::OutOfDate;
	}

	//
	// @param imageIndex  Index into the swapchain images.
	// @param queue present queue
	bool Swapchain::present(std::uint32_t imageIndex,VkQueue queue) const {
		if(dirty)return false;
		VkSemaphore renderFinished = m_renderFinished[imageIndex];
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapchain.swapchain;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinished;
		const auto result = vkQueuePresentKHR(queue, &presentInfo);
		return result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR;
	}
}
