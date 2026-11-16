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
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

add_library(vma STATIC external/VMA/vk_mem_alloc.cpp)

find_package(Vulkan REQUIRED)
if(NOT SLANGC_EXECUTABLE)
    find_program(SLANGC_EXECUTABLE slangc HINTS "${Vulkan_SDK_PATH}/bin" "$ENV{VULKAN_SDK}/bin")
endif()
if(NOT SLANGC_EXECUTABLE)
    message(FATAL_ERROR "failed to find sanglc: please set slang executable")
endif()
 
# 处理第三方库
add_subdirectory(external/vk-bootstrap)
add_subdirectory(external/glm)


# 创建自己的目标
add_executable(${PROJECT_NAME} 
    src/wRendererj.cpp
    src/Window/WindowManger.cpp
    src/RHI/Types.h
    src/RHI/RenderContext.cpp
    src/RHI/FrameSync.cpp
    src/RHI/CommandList.cpp
    src/RHI/ResourcesPool.cpp
    src/RHI/Common.cpp
    src/RHI/Upload.cpp
    src/RHI/Pipeline.cpp
    src/Asset/Test.h
)


# 给目标加配置
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
    vma
)

# 编译着色器
set(test_shader "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/test.slang")
set(tt_shader "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/tt.slang")

function(compile_shader target output_name)
    cmake_parse_arguments(PARSE_ARGV 2 shader "" "" "sources;entry")
    set(out_shader_dir "${CMAKE_CURRENT_BINARY_DIR}/Shaders/compileShaders")   # ${CMAKE_CURRENT_BINARY_DIR}，可以定位到cmake的构建目录，在这个项目是.exe所在的目录
    file(MAKE_DIRECTORY ${out_shader_dir})

    set(entry_args "")
    foreach(E ${shader_entry})
        list(APPEND entry_args -entry ${E})
    endforeach()
    if(NOT entry_args)
        message(FATAL_ERROR "please set entry name")
    endif()
    if(NOT shader_sources)
        message(FATAL_ERROR "please set compile goal")
    endif()

    add_custom_command(
        OUTPUT ${out_shader_dir}/${output_name}
        COMMAND ${SLANGC_EXECUTABLE} ${shader_sources} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${entry_args} -o "${output_name}"
        WORKING_DIRECTORY ${out_shader_dir}
        DEPENDS ${shader_sources}
        COMMENT "compiling ${output_name}"
        VERBATIM
    )
    add_custom_target(${target} DEPENDS "${out_shader_dir}/${output_name}")
endfunction()
compile_shader(test_shader_compile test_shader.spv 
               sources ${test_shader}
               entry mainVert mainFrag)
compile_shader(tt_shader_compile tt_shader.spv
               sources ${tt_shader}
               entry vertMain)
add_dependencies(${PROJECT_NAME} test_shader_compile tt_shader_compile)

# 源文件是 UTF-8（无 BOM）且使用 LF 换行。MSVC 默认按系统代码页（中文系统为 GBK）
# 解析源文件，中文注释的尾字节可能被当作双字节字符的首字节并吞掉换行，
# 导致下一行被并入注释（头文件 include、函数签名凭空消失）。必须显式指定 /utf-8。
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()