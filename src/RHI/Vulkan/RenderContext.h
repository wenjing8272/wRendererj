#pragma once
#include"RenderConfig.h"
#include"../Types.h"
#include<VkBootstrap.h>
#include<vulkan/vulkan.h>
#include<vma/vk_mem_alloc.h>

#include<string>
#include<vector>
#include<memory>
#include<array>
#include<optional>

namespace RHI {
	class Device;

	class Swapchain;

	class FrameSync;

	class CommandList;

	class ResourcesPool;

	class Upload;

	class Pipeline;

	class DeferedDeleter;

	class RenderContext {
	public:
		RenderContext();
		~RenderContext();
		RenderContext(const RenderContext& otherRenderContext) = delete;
		RenderContext& operator=(const RenderContext& otherRenderContext) = delete;
		RenderContext(RenderContext&& otherRenderContext) = delete;
		RenderContext& operator=(RenderContext&& otherRenderContext) = delete;

		void initRenderContext(const InitResource& initRes,const std::vector<std::string>& instanceExtensions);
		
		void recreateSwapchain(Extent2D extent);

		BufferHandle createBuffer(std::uint64_t size,BufferUsage bufferUsage,MemoryProperty memoryProperty);
		PipelineHandle createGraphicsPipeline(const GraphicsPipeline& pipelineRes);
		void createImage();

		void destoryBuffer(BufferHandle bufferHandle);

		void copyBuffer(BufferHandle copyBuffer, const void* data, std::uint64_t dataSize);
		void uploadBuffer(BufferHandle srcBufferHandle, BufferHandle dstBufferHandle,std::uint64_t bufferSize);

		ShaderHandle createShader(const std::vector<std::uint32_t>& shaderCode);

		// Return null means the swapchain is out of date,need to recreate swapchain
		std::optional<Frame> begin();
		// Return false means present failure,likely the swapchain is out of date;need to recreate swapchain
		bool end(Frame& frame);
	private:
		std::unique_ptr<Device> m_device{ nullptr };

		std::unique_ptr<Swapchain> m_swapchain{ nullptr };

		std::unique_ptr<FrameSync> m_frameSync{ nullptr };

		// One recording entry per frame-in-flight slot; lives as long as the device.
		std::vector<std::unique_ptr<CommandList>> m_commandLists;
		
    	std::unique_ptr<DeferedDeleter> m_deferedDeleter{ nullptr };

		std::unique_ptr<ResourcesPool> m_resourcesPool{ nullptr };

		std::unique_ptr<Upload> m_upload{ nullptr };

		std::unique_ptr<Pipeline>m_pipeline{ nullptr };
	};
}
