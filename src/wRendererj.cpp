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
		wrj::RenderContext renderContext;
		wrj::WindowManger windowManger(800, 600);
		windowManger.createWindow();
		renderContext.initRenderContext(windowManger.getNativeResources(), windowManger.getRequiredInstanceExtensions());
		while (!windowManger.windowShouldClose())
		{
			windowManger.closeWindowEvent();
			windowManger.pollEvents();
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
		return -1;
	}
    return 0;
}