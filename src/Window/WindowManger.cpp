#include"WindowManger.h"

#include<stdexcept>
#define GLFW_EXPOSE_NATIVE_WIN32
#include<GLFW/glfw3native.h>

namespace wrj {
	WindowManger::WindowManger(std::uint32_t width, std::uint32_t height) : m_width(width), m_height(height)
	{
	}
	WindowManger::~WindowManger()
	{
		if (m_window)glfwDestroyWindow(m_window);
		glfwTerminate();
	}
	//初始化窗口，传递给 Vulkan 的原生资源和扩展列表必须在窗口创建之后才能获取
	void WindowManger::createWindow()
	{
		if (!glfwInit())
		{
			throw std::runtime_error("Failed to initialize GLFW");
		}
		m_isWindowCreated = true;
		glfwInitHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwInitHint(GLFW_RESIZABLE, GLFW_TRUE);
		m_window = glfwCreateWindow(m_width, m_height, "wRendererj", nullptr, nullptr);
		if (m_window == nullptr)
		{
			throw std::runtime_error("Failed to create GLFW window");
		}
		glfwSetWindowUserPointer(m_window, this);
		glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
	}
	const NativeResources WindowManger::getNativeResources() const
	{
		if (!m_isWindowCreated)
		{
			throw std::runtime_error("Window has not been created yet");
		}
		NativeResources native;
		native.display = GetModuleHandle(nullptr);
		native.window = glfwGetWin32Window(m_window);
		int width, height;
		glfwGetFramebufferSize(m_window, &width, &height);
		native.frameBufferWidth = static_cast<std::uint32_t>(width);
		native.frameBufferHeight = static_cast<std::uint32_t>(height);
		return native;
	}
	const std::vector<std::string> WindowManger::getRequiredInstanceExtensions() const
	{
		if (!m_isWindowCreated)
		{
			throw std::runtime_error("Window has not been created yet");
		}
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		if (glfwExtensions == nullptr)
		{
			throw std::runtime_error("Failed to get required instance extensions from GLFW");
		}
		std::vector<std::string> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		return extensions;
	}
	void WindowManger::closeWindowEvent()
	{
		if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(m_window, true);
		}
	}
	const bool WindowManger::windowShouldClose() const
	{
		return glfwWindowShouldClose(m_window);
	}
	void WindowManger::pollEvents() const
	{
		glfwPollEvents();
	}
	void WindowManger::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto windowManger = reinterpret_cast<WindowManger*>(glfwGetWindowUserPointer(window));
		if (windowManger)
		{
			windowManger->m_framebufferResized = true;
		}
	}
}