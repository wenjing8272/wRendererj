project("RHI")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# RHI 是模块库，由顶层 CMakeLists.txt 里的主程序链接
add_library(${PROJECT_NAME} STATIC
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

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC
    Vulkan::Vulkan
    vk-bootstrap::vk-bootstrap
    vma
)
