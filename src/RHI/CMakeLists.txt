# RHI 是模块库，由顶层 CMakeLists.txt 里的主程序链接
add_library(RHI STATIC
    Vulkan/Device.cpp
    Vulkan/Swapchain.cpp
    Vulkan/RenderContext.cpp
    Vulkan/FrameSync.cpp
    Vulkan/CommandList.cpp
    Vulkan/ResourcesPool.cpp
    Vulkan/Common.cpp
    Vulkan/Upload.cpp
    Vulkan/Pipeline.cpp
    Vulkan/VulkanPass.cpp
    Vulkan/DeferedDeleter.cpp
)

target_include_directories(RHI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(RHI PUBLIC
    Vulkan::Vulkan
    vk-bootstrap::vk-bootstrap
    vma
    spirv-reflect-static
)
