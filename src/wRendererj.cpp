// wRendererj.cpp: 定义应用程序的入口点。
//
#include"Window/WindowManger.h"
#include"RHI/Types.h"
#include"RHI/RenderContext.h"

#include <VkBootstrap.h>
#include <iostream>

int main()
{
	try {
		bool swapchainDirty = false;
		wrj::WindowManger windowManger(800, 600);
		wrj::RenderContext renderContext;
		windowManger.createWindow();
		renderContext.initRenderContext(windowManger.getNativeResources(), windowManger.getRequiredInstanceExtensions());
		while (!windowManger.windowShouldClose())
		{
			windowManger.closeWindowEvent();
			windowManger.pollEvents();
			if (swapchainDirty || windowManger.consumeFramebufferResized())
			{
				auto framebufferSize = windowManger.getFramebufferSize();
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