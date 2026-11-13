// wRendererj.cpp: 定义应用程序的入口点。
//
#include"Window/WindowManger.h"
#include"RHI/Types.h"
#include"RHI/RenderContext.h"
#include"Asset/Test.h"

#include <VkBootstrap.h>
#include <iostream>

int main()
{
	try {
		bool swapchainDirty = false;
		wrj::WindowManger windowManger(1920, 1080);
		wrj::RenderContext renderContext;
		windowManger.createWindow();
		auto initFramebufferSize = windowManger.getFramebufferSize();
		wrj::InitResource initRes{};
		initRes.native = windowManger.getNativeResources();
		initRes.frameBufferWidth = initFramebufferSize.width;
		initRes.frameBufferHeight = initFramebufferSize.height;
		initRes.appName = " wRenderj";
		renderContext.initRenderContext(initRes, windowManger.getRequiredInstanceExtensions());

		auto vertices = wrj::getVertices();
		if (vertices.empty())
		{
			std::cout << "empty" << '\n';
		}
		else
		{
			std::uint64_t size = sizeof(vertices[0]) * vertices.size();
			auto stagingBufferHandle = renderContext.createBuffer(size, wrj::BufferUsage::TransferSrc, wrj::MemoryProperty::HostVisible);
			renderContext.copyBuffer(stagingBufferHandle, vertices.data(), size);
			auto bufferHandle = renderContext.createBuffer(size, wrj::BufferUsage::TransferDst, wrj::MemoryProperty::DeviceLocal);
			renderContext.uploadBuffer(stagingBufferHandle, bufferHandle,size);
		}
		while (!windowManger.windowShouldClose())
		{
			windowManger.closeWindowEvent();
			windowManger.pollEvents();
			if (swapchainDirty || windowManger.consumeFramebufferResized())
			{
				auto framebufferSize  = windowManger.getFramebufferSize();
				while (framebufferSize.width == 0 || framebufferSize.height == 0)
				{
					windowManger.waitEvents();
					if (windowManger.windowShouldClose())
					{
						return 0;
					}
					framebufferSize = windowManger.getFramebufferSize();
				}
				std::cout << "recreate swapchain" << std::endl;
				renderContext.recreateSwapchain({ framebufferSize.width, framebufferSize.height });
				swapchainDirty = false;
			}
			renderContext.begin(swapchainDirty);
			if (swapchainDirty)continue;
			renderContext.beginRendering();
			renderContext.endRendering();
			renderContext.end(swapchainDirty);
			if (swapchainDirty)continue;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
		return -1;
	}
    return 0;
}