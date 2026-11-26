#include "CommandList.h"
#include "Device.h"
#include "Swapchain.h"
#include "Common.hpp"
#include "FrameSync.h"
#include "Pipeline.h"
#include "RenderContext.h"
#include "ResourcesPool.h"
#include "Upload.h"
#include "DeferedDeleter.h"

#include <iostream>

namespace RHI {

    RenderContext::RenderContext() = default;
    // Destruction order must be child-before-creator:
    // image views -> swapchain -> device -> surface -> instance
    RenderContext::~RenderContext() {
        // Once RenderContext is fully constructed, any exception from a member
        // function will trigger the destructor. If the exception was thrown before
        // `m_device`(Device) was created, the destructor can just return early.
        if (!m_device) return;
        // Call vkDeviceWaitIdle before destroying resources, ensuring the GPU
        // is done with them and nothing is freed while still in use.
        vkDeviceWaitIdle(m_device->get());
        // Drain the deferred-deletion queue, releasing all pending resources.
        if(m_deferedDeleter)
        {
            m_deferedDeleter->flushAll();
        }
        // Destroy all child objects before the Device is released.
        // Calling .reset() destroys the managed object immediately
        // (invokes its destructor) and nulls the pointer.
        m_pipeline.reset();
        std::cout << "renderContext" << "\n";
        m_upload.reset();
        m_frameSync.reset();
        for (auto &commandList : m_commandLists) {
          commandList.reset();
        }
        m_resourcesPool.reset();
        m_deferedDeleter.reset();
        m_swapchain.reset();
        m_device.reset();
    }

    void RenderContext::initRenderContext(const InitResource &initRes,
                                          const std::vector<std::string> &instanceExtensions) {

        m_device = std::make_unique<Device>(initRes, instanceExtensions);
        // Window info lives in Swapchain. Anything that needs the window size reads it
        // through this object, so each frame sees a single, consistent snapshot.
        m_swapchain = std::make_unique<Swapchain>(*m_device);

        // FrameSync owns the per-frame-in-flight synchronization objects (image-available
        // semaphores and the graphics timeline); image acquire/present is delegated to
        // Swapchain, which manages its own state internally.
        m_frameSync = std::make_unique<FrameSync>(m_device->get(), m_device->graphics().handle);

        // DeferedDeleter must be created before ResourcesPool: the pool does not destroy
        // Vulkan objects directly, but defers the actual destruction to it, preventing
        // resources from being freed while the GPU may still be reading them.
        m_deferedDeleter = std::make_unique<DeferedDeleter>();

        // ResourcesPool creates resources, returns handles, and resolves
        // generation-checked handles to VkBuffer/VmaAllocation, while pushing the actual
        // destruction off to the deferred deleter.
        m_resourcesPool = std::make_unique<ResourcesPool>(m_device->get(), m_device->graphics().pool,
                                                    m_device->graphics().handle, m_device->allocator(),
                                                    *m_deferedDeleter);

        // Upload records its copies on the transfer queue, whose pool was created
        // with the TRANSIENT flag since it only holds short-lived one-shot submits.
        m_upload = std::make_unique<Upload>(m_device->get(), m_device->transfer().pool,
                                            m_device->transfer().handle);                        

        m_pipeline = std::make_unique<Pipeline>(m_device->get());

        /*CommandList resolves PipelineHandle/BufferHandle internally, so it must be
        created after ResourcesPool and Pipeline.                                   
        One primary command buffer per frame-in-flight slot, allocated from the
        graphics pool (created with the RESET bit): each slot re-records the same
        buffer every frame, and recording starts with an explicit reset.
        */
        std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> buffers{};

        VkCommandBufferAllocateInfo commandBufferInfo{};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
        commandBufferInfo.commandPool = m_device->graphics().pool;
        commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        if (vkAllocateCommandBuffers(m_device->get(), &commandBufferInfo, buffers.data()) !=
            VK_SUCCESS) {
          throw std::runtime_error("vkAllocateCommandBuffers failed");
        }

        m_commandLists.resize(MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
          m_commandLists[i] = std::make_unique<CommandList>(m_device->get(), m_device->graphics().family,
                                                            buffers[i], *m_resourcesPool, *m_pipeline);
        }
    }

    void RenderContext::recreateSwapchain(Extent2D extent) {
        // The old swapchain's images and semaphores may still be referenced by
        // in-flight frames, so drain the graphics queue before swapping it out.
        vkQueueWaitIdle(m_device->graphics().handle);

        // Build the new swapchain first (passing the old handle so the driver can
        // reuse resources) and only then let the old object destruct. If the
        // constructor throws, the old swapchain is still intact and the caller can
        // simply retry on the next frame.
        auto next = std::make_unique<Swapchain>(*m_device, extent, m_swapchain->get());
        m_swapchain = std::move(next);
    }
    BufferHandle RenderContext::createBuffer(std::uint64_t size,
                                            BufferUsage bufferUsage,
                                            MemoryProperty memoryProperty) {
        return m_resourcesPool->createBuffer(size, userToVK(bufferUsage),
                                          userToVK(memoryProperty));
    }
    void RenderContext::copyBuffer(BufferHandle copyBuffer, const void *data,
                                  std::uint64_t dataSize) {
        m_resourcesPool->copyBuffer(copyBuffer, data, dataSize);
    }
    void RenderContext::uploadBuffer(BufferHandle srcBufferHandle,
                                    BufferHandle dstBufferHandle,
                                    std::uint64_t bufferSize) {
        // Check the handle for validity before uploading the resource.
        if (!m_resourcesPool->isBufferVaild(srcBufferHandle) ||
            !m_resourcesPool->isBufferVaild(dstBufferHandle)) {
            throw std::runtime_error("src or dst buffer no vaild");
        }
        m_upload->uploadBuffer(m_resourcesPool->getBuffer(srcBufferHandle),
                              m_resourcesPool->getBuffer(dstBufferHandle),
                              bufferSize);
    }
    
    ShaderHandle RenderContext::createShader(const std::vector<std::uint32_t> &shaderCode) {
        return m_pipeline->createShader(shaderCode);
    }

    PipelineHandle RenderContext::createGraphicsPipeline(const GraphicsPipeline &pipelineRes) {
        // If recreate swapchain,cause format of swapchain change,pipeline need to recreate
        return m_pipeline->createGraphicsPipeline(pipelineRes,
                                                  m_swapchain->format());
    }
    // Return null means the swapchain is out of date, need to recreate swapchain
    std::optional<Frame> RenderContext::begin() {
        // At the start of each frame, 
        // free the resources in the deferred deletion queue that are safe to release.
        m_deferedDeleter->deferedDeletion(m_device->get(),
                                        m_frameSync->timelineSemaphore());
        // Acquire the next image and claim this frame's slot/value. A nullopt means
        // acquire failed, so the swapchain need be recreated
        // before any recording starts.
        auto frame = m_frameSync->frameBegin(*m_swapchain);
        if (!frame) {
          return std::nullopt;
        }

        frame->image = m_swapchain->image(frame->imageIndex);
        frame->imageView = m_swapchain->view(frame->imageIndex);
        frame->area = m_swapchain->area();
        frame->commandList = m_commandLists[frame->frameIndex].get();


        frame->commandList->begin();
        return frame;
    }
    // Return false means present failure, likely the swapchain is out of date;
    // need to recreate swapchain.
    bool RenderContext::end(Frame &frame) {
        // Recording must be closed before submit; FrameSync submits whatever handle
        // is stored in the frame, so write it back after vkEndCommandBuffer.
        frame.commandBuffer = frame.commandList->end();

        return m_frameSync->frameEnd(frame, *m_swapchain);
    }

    void RenderContext::destoryBuffer(BufferHandle bufferHandle) {

        m_resourcesPool->destoryBuffer(bufferHandle,
                                      m_frameSync->currentTimelineValue());
    }
} // namespace RHI
