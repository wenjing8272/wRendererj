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
#include<string>
#include<unordered_map>

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

//Official opcode flags
	// 操作码：指令第一个 word 的低 16 位
enum class Op : std::uint32_t
{
    OpNop         = 0,
    OpName        = 5,
    OpEntryPoint  = 15,
    OpTypeInt     = 21,
    OpTypeFloat   = 22,
    OpTypeVector  = 23,
    OpTypePointer = 32,
    OpVariable    = 59,
    OpDecorate    = 71,
};
 
// 修饰类型：OpDecorate 的第 2 个操作数
enum class Decoration : std::uint32_t
{
    BuiltIn  = 11,
    Location = 30,
};
 
// 存储类别：OpVariable 和 OpTypePointer 里的存储类别操作数
enum class StorageClass : std::uint32_t
{
    Input  = 1,
    Output = 3,
};

struct TypeInfo{
  std::uint32_t opcode,a,b;
};

struct Var{
  std::uint32_t typeID,storage;
};

void printSpvReflectResult(const std::vector<std::uint32_t>& code)
{
  if(code.size() < 5 || code[0] != 0x07230203)
  {
    std::cout<<"code no is spv file"<<'\n';
    return;
  }

  std::unordered_map<std::uint32_t,std::string> names;
  std::unordered_map<std::uint32_t,std::uint32_t> locations;
  std::unordered_map<std::uint32_t,bool> isBuiltIn;
  std::unordered_map<std::uint32_t,TypeInfo> types;
  std::unordered_map<std::uint32_t,Var> vars;
  std::unordered_map<std::string,std::vector<std::uint32_t>> entryInterface;
  
  //In the first word of every instruction,the upper 16 bits denote the length,
  //and the lower 16 bits denote the opcode
  for(size_t i = 5;i < code.size();)
  {
    //Can extract the length by shift the word right by 16 bits
    std::uint32_t wordCount = code[i] >> 16;
    std::cout<<"wordCount: "<<wordCount<<'\n';
    //Can extract the opcode by apply a bitmark of 0xFFFF,by bitwise AND
    auto opcode = static_cast<Op>(code[i] & 0xFFFF);
    //Since the last instruction is a single word,
    //dereferencing &[i + 1] would lead to undefined behavior.
    //In contrast,.data() + i + 1 merely performs pointer without actually accessing the memory
    const std::uint32_t* op = code.data() + i + 1;
    std::cout<<"opcode"<<'\n';
    switch(opcode)
    {
    case Op::OpEntryPoint:
      std::cout<<"OpEntryPoint"<<'\n';
      break;
    }
    i += wordCount;
  }
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

    printSpvReflectResult(shaderCode);

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
