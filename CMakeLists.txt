# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.10)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()
project ("wRendererj")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_library(vma STATIC external/VMA/vk_mem_alloc.cpp)

find_package(Vulkan REQUIRED)
 
# 1. 先处理第三方库
add_subdirectory(external/vk-bootstrap)


# 2. 再创建自己的目标
add_executable(${PROJECT_NAME} 
    src/wRendererj.cpp
    src/Window/WindowManger.cpp
    src/RHI/Types.h
    src/RHI/RenderContext.cpp
)
# 3. 最后给目标加配置
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/VMA
)
add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION             ${CMAKE_CURRENT_SOURCE_DIR}/external/GLFW/glfw-3.5.1.bin.WIN64/lib-vc2022/glfw3.lib
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/external/GLFW/glfw-3.5.1.bin.WIN64/include
)
target_link_libraries(vma PUBLIC Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    vk-bootstrap::vk-bootstrap
    glfw
)
