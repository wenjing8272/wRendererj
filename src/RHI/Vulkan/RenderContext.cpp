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

  // FrameSync 只持有每 frame-in-flight 的同步对象（image-available 信号量和
  // 图形时间线）。图像的 acquire/present 委托给 Swapchain，因为它们操作的是
  // 交换链自己的状态。
  // FrameSync owns only the per-frame-in-flight sync objects (image-available
  // semaphores and the graphics timeline). Image acquire and present are
  // delegated to Swapchain, because they operate on swapchain-owned state.
  m_frameSync = std::make_unique<FrameSync>();
  m_frameSync->initFrameSync(m_device->get(), m_device->graphics().handle);

  // DeferedDeleter 必须先于 ResourcesPool 创建：资源池不直接销毁 vk 对象，
  // 而是把真正的销毁推迟到它里面，避免 GPU 还在读时被释放。
  // DeferedDeleter must exist before ResourcesPool: the pool defers the real
  // vk objects into it instead of destroying them while the GPU may still read.
  m_deferedDeleter = std::make_unique<DeferedDeleter>();

  // 资源池负责把带 generation 校验的句柄解析成 VkBuffer/VmaAllocation，
  // 并把真正的销毁推进延迟删除器。
  // The pool resolves generation-checked handles to VkBuffer/VmaAllocation and
  // pushes real destroys into the deferred deleter.
  m_resourcesPool = std::make_unique<ResourcesPool>();
  m_resourcesPool->init(m_device->get(), m_device->graphics().pool,
                        m_device->graphics().handle, m_device->allocator(),
                        *m_deferedDeleter);

  // Upload 在传输队列上录制拷贝，该队列的 pool 创建时带了 TRANSIENT 标记，
  // 因为它只承载生命周期很短的 one-shot 提交。
  // Upload records its copies on the transfer queue, whose pool was created
  // with the TRANSIENT flag since it only holds short-lived one-shot submits.
  m_upload = std::make_unique<Upload>();
  m_upload->init(m_device->get(), m_device->transfer().pool,
                 m_device->transfer().handle);

  // Pipeline 只需要设备；shader module 和 pipeline 对象随它一起销毁。
  // Pipeline only needs the device; shader modules and pipeline objects are
  // destroyed with it.
  m_pipeline = std::make_unique<Pipeline>();
  m_pipeline->init(m_device->get());

  // CommandList 内部要解析 PipelineHandle/BufferHandle，所以必须在
  // ResourcesPool 和 Pipeline 之后创建。
  // CommandList resolves PipelineHandle/BufferHandle internally, so it must be
  // created after ResourcesPool and Pipeline.
  //
  // 每个 frame-in-flight 槽位一个 primary 命令缓冲，从带 RESET 标记的
  // 图形队列 pool 分配：每个槽位每帧重录同一个缓冲，录制开始时先显式 reset。
  // One primary command buffer per frame-in-flight slot, allocated from the
  // graphics pool (created with the RESET bit): each slot re-records the same
  // buffer every frame, and recording starts with an explicit reset.
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
    m_commandLists[i] = std::make_unique<CommandList>();
    m_commandLists[i]->init(m_device->get(), m_device->graphics().family,
                            buffers[i], *m_resourcesPool, *m_pipeline);
  }
}

void RenderContext::recreateSwapchain(Extent2D extent) {
  // 旧交换链的图像和信号量可能还被在途帧引用，换掉之前先把图形队列排空。
  // The old swapchain's images and semaphores may still be referenced by
  // in-flight frames, so drain the graphics queue before swapping it out.
  vkQueueWaitIdle(m_device->graphics().handle);
  // 先构造新交换链（带上旧句柄让驱动尽量复用资源），成功后再让旧对象析构。
  // 构造中途抛异常时旧交换链仍然完好，调用方下一帧重试即可。
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
  // 句柄是 index+generation。槽位复用时 generation 自增，保证已销毁的句柄
  // 校验失败，而不是悄悄解析成占用同一槽位的新 buffer。
  // Handles are index+generation pairs. The generation bump on slot reuse is
  // what makes a destroyed handle fail validation instead of silently
  // resolving to whatever buffer now occupies the same slot.
  return m_resourcesPool->createBuffer(size, userToVK(bufferUsage),
                                       userToVK(memoryProperty));
}
void RenderContext::copyBuffer(BufferHandle copyBuffer, const void *data,
                               std::uint64_t dataSize) {
  // 只有 host-visible 内存的 buffer 在创建时被映射；资源池会拒绝其他情况，
  // 而不是对着空指针做 memcpy。
  // Only host-visible buffers are mapped at creation; the pool rejects
  // everything else rather than memcpy-ing through a null pointer.
  m_resourcesPool->copyBuffer(copyBuffer, data, dataSize);
}
void RenderContext::uploadBuffer(BufferHandle srcBufferHandle,
                                 BufferHandle dstBufferHandle,
                                 std::uint64_t bufferSize) {
  // 两个句柄先校验再解析：过期句柄必须在这里就报错，
  // 而不是在拷贝时悄悄变成空 VkBuffer。
  // Validate both handles before resolving them: a stale handle must fail
  // loudly here instead of turning into a null VkBuffer inside the copy.
  if (!m_resourcesPool->isBufferVaild(srcBufferHandle) ||
      !m_resourcesPool->isBufferVaild(dstBufferHandle)) {
    throw std::runtime_error("src or dst buffer no vaild");
  }
  m_upload->uploadBuffer(m_resourcesPool->getBuffer(srcBufferHandle),
                         m_resourcesPool->getBuffer(dstBufferHandle),
                         bufferSize);
}
ShaderHandle
RenderContext::createShader(const std::vector<std::uint32_t> &shaderCode) {
  return m_pipeline->createShader(shaderCode);
}
PipelineHandle
RenderContext::createGraphicsPipeline(const GraphicsPipeline &pipelineRes) {
  // 当前交换链格式会被烘进管线的颜色附件状态。若重建后的交换链协商出
  // 不同的格式，这里创建的管线就不再匹配渲染目标，必须重新创建。
  // The current swapchain format is baked into the pipeline's color attachment
  // state. If a recreated swapchain negotiates a different format, pipelines
  // built here no longer match the render target and must be rebuilt.
  return m_pipeline->createGraphicsPipeline(pipelineRes,
                                            m_swapchain->format());
}
// Return null means the swapchain is out of date, need to recreate swapchain
std::optional<Frame> RenderContext::begin() {
  // 非阻塞地查询时间线计数器：把 retire 值已被点亮的删除项执行掉，
  // 这里不会等 GPU。
  // Non-blocking poll of the timeline counter: execute the deleters whose
  // retire value has already been signaled. Nothing here waits on the GPU.
  m_deferedDeleter->deferedDeletion(m_device->get(),
                                    m_frameSync->timelineSemaphore());
  // 拿下一张图像并占用本帧的槽位和 timeline 值。返回空表示 acquire 失败
  // （通常是交换链过期），必须在开始录制之前重建交换链。
  // Acquire the next image and claim this frame's slot/value. A nullopt means
  // acquire failed (typically out of date), so the swapchain must be recreated
  // before any recording starts.
  auto frame = m_frameSync->frameBegin(*m_swapchain);
  if (!frame) {
    return std::nullopt;
  }
  // 把渲染目标快照进 Frame，录制代码只依赖这个值，不再回头碰交换链。
  // Snapshot the render target into the frame so recording code only needs the
  // Frame value and never reaches back into the swapchain.
  frame->image = m_swapchain->image(frame->imageIndex);
  frame->imageView = m_swapchain->view(frame->imageIndex);
  frame->area = m_swapchain->area();
  frame->commandList = m_commandLists[frame->frameIndex].get();
  // 立即 reset 并开始录制。beginRendering 里还会做
  // UNDEFINED -> COLOR_ATTACHMENT 的布局转换，然后才 vkCmdBeginRendering。
  // Reset and start recording now. beginRendering still performs the
  // UNDEFINED -> COLOR_ATTACHMENT layout transition before vkCmdBeginRendering.
  frame->commandList->begin();
  return frame;
}
// Return false means present failure, likely the swapchain is out of date;
// need to recreate swapchain.
bool RenderContext::end(Frame &frame) {
  // 提交前必须先结束录制；FrameSync 提交的是 Frame 里存的句柄，
  // 所以 vkEndCommandBuffer 之后要把句柄写回 Frame。
  // Recording must be closed before submit; FrameSync submits whatever handle
  // is stored in the frame, so write it back after vkEndCommandBuffer.
  frame.commandBuffer = frame.commandList->end();
  // 提交本帧并 present。返回 false 表示 present 失败（通常是交换链过期），
  // 调用方重建交换链后继续。
  // Submit the frame and present the image. A false result means present
  // failed (usually out of date); the caller recreates and continues.
  return m_frameSync->frameEnd(frame, *m_swapchain);
}

void RenderContext::destoryBuffer(BufferHandle bufferHandle) {
  // 句柄立即失效，但 VkBuffer/VmaAllocation 会活到图形时间线越过最后提交的
  // 值为止：在途帧可能还在读它们。
  // The handle dies immediately, but the VkBuffer/VmaAllocation stay alive
  // until the graphics timeline passes the last submitted value: in-flight
  // frames may still be reading from them.
  m_resourcesPool->destoryBuffer(bufferHandle,
                                 m_frameSync->currentTimelineValue());
}
} // namespace RHI
