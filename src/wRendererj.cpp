// wRendererj.cpp: 定义应用程序的入口点。
//
#include"Window/WindowManger.h"
#include"RHI/Types.h"
#include"RHI/RenderContext.h"

#include <VkBootstrap.h>
#include <iostream>

int main()
{
	wrj::RenderContext renderContext;
	wrj::WindowManger windowManger(800, 600);
	windowManger.createWindow();
	while (true)
	{

	}
    return 0;
}