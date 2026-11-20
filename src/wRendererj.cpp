// wRendererj.cpp: 定义应用程序的入口点。
//
#include"Window/WindowManger.h"
#include"RHI/Types.h"
#include"RHI/Vulkan/RenderContext.h"
#include"Asset/Test.h"

#include <iostream>
#include<filesystem>
#include<fstream>
#include<vector>
std::vector<std::uint32_t> readFile(std::filesystem::path shaderPath)
{
	std::cout << "current file directory: " << std::filesystem::current_path() << '\n';
	std::ifstream file(shaderPath,std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		throw std::runtime_error("failed to open shader file");
	}
	const auto end = file.tellg();
	const auto size = static_cast<size_t>(end);
	assert(size % sizeof(std::uint32_t) == 0 && "shader code must be a multiple of 4");
	std::vector<std::uint32_t> code(size / sizeof(std::uint32_t));
	file.seekg(0, std::ios::beg);
	if (!file.read(reinterpret_cast<char*>(code.data()),static_cast<std::streamsize>(size)))
	{
		throw std::runtime_error("failed to read shader file");
	}
	return code;
}


int main()
{
	try {
		bool swapchainDirty = false;
		wrj::WindowManger windowManger(1920, 1080);
		RHI::RenderContext renderContext;
		windowManger.createWindow();
		auto initFramebufferSize = windowManger.getFramebufferSize();
		RHI::InitResource initRes{};
		initRes.native = windowManger.getNativeResources();
		initRes.frameBufferWidth = initFramebufferSize.width;
		initRes.frameBufferHeight = initFramebufferSize.height;
		initRes.appName = " wRenderj";
		renderContext.initRenderContext(initRes, windowManger.getRequiredInstanceExtensions());

		auto vertices = wrj::getVertices();
		
		assert(!vertices.empty() && "vertice must has value");

		std::uint64_t size = sizeof(vertices[0]) * vertices.size();
		auto stagingBufferHandle = renderContext.createBuffer(size, RHI::BufferUsage::TransferSrc, RHI::MemoryProperty::HostVisible);
		renderContext.copyBuffer(stagingBufferHandle, vertices.data(), size);
		auto bufferHandle = renderContext.createBuffer(size, RHI::BufferUsage::TransferDst | RHI::BufferUsage::VertexBuffer, RHI::MemoryProperty::DeviceLocal);
		renderContext.uploadBuffer(stagingBufferHandle, bufferHandle,size);
		renderContext.destoryBuffer(stagingBufferHandle);
		auto shaderCode = readFile("Shaders/compileShaders/test_shader.spv");
		auto testShaderHandle = renderContext.createShader(shaderCode);

		auto vertexAttribute = wrj::Vertex::vertexAttribute();
		std::vector<RHI::DynamicState> dynamics = { RHI::DynamicState::Viewport,RHI::DynamicState::Scissor };

		RHI::GraphicsPipeline test{
			.vertexShader = testShaderHandle,
			.fragmentShader = testShaderHandle,
			.vertexName = "mainVert",
			.fragmentName = "mainFrag",
			.vertexBinding = wrj::Vertex::vertexBinding(),
			.vertexAttribute = vertexAttribute,
			.pushConstantRange = {.offset = 0,.size = 0},
			.primitiveTopology = RHI::PrimitiveTopology::TriangleList,
			.depthBaisEnable = false,
			.sampleCount = RHI::SampleCount::e1,
			.blendEnable = false,
			.dynamicStates = dynamics,
			.debugName = "testPipeline"
		};
		auto testPipelinHandle = renderContext.createGraphicsPipeline(test);

		while (!windowManger.windowShouldClose())
		{
			windowManger.closeWindowEvent();
			windowManger.pollEvents();
			auto framebufferSize = windowManger.getFramebufferSize();
			if (swapchainDirty || windowManger.consumeFramebufferResized())
			{
				
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
			renderContext.beginRendering(RHI::RenderTarget::Swapchain);
			renderContext.bindPipeline(testPipelinHandle);
			renderContext.setViewport({ .x = 0.0f,.y = 0.0f,.width = static_cast<float>(framebufferSize.width),.height = static_cast<float>(framebufferSize.height),.minDepth = 0.0f,.maxDepth = 1.0f });
			renderContext.setScissor({ .offset = {0,0},.extent = framebufferSize });
			renderContext.bindingVertexBuffer(bufferHandle);
			renderContext.draw();
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