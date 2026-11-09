#include"ResourcesManger.h"

namespace wrj {
	void ResourcesManger::init(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
	{
		m_device = device;
		m_physicalDevice = physicalDevice;
		m_commandPool = commandPool;
		m_graphicsQueue = graphicsQueue;
	}
	void ResourcesManger::cleanup()
	{
	}
}