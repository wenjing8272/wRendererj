#pragma once
#include<GLFW/glfw3.h>
#include<cstdint>
#include<string>
#include<vector>

#include"../RHI/Types.h"

namespace wrj {
	class WindowManger
	{
	public:
		WindowManger(std::uint32_t width,std::uint32_t height);
		~WindowManger();
		WindowManger(const WindowManger& otherWindowManger) = delete;
		WindowManger& operator=(const WindowManger& otherWindowManger) = delete;
		WindowManger(WindowManger&& otherWindowManger) = delete;
		WindowManger& operator=(WindowManger&& otherWindowManger) = delete;
		void createWindow();
		const NativeResources getNativeResources()const;
		const std::vector<std::string> getRequiredInstanceExtensions()const;
	private:
		std::uint32_t m_width{ 0 };
		std::uint32_t m_height{ 0 };
		bool m_isWindowCreated{ false };
		GLFWwindow* m_window{ nullptr };
	};
}