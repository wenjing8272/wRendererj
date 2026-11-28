add_library(Scene STATIC
    SceneManager.cpp
    external/tinyglTFloader/tiny_gltf.cpp   # tinygltf 只在这里编译一次
)

# KTX 是 DLL：ktx.lib 是导入库，ktx.dll 是本体
add_library(ktx SHARED IMPORTED GLOBAL)
set_target_properties(ktx PROPERTIES
    IMPORTED_IMPLIB            "${CMAKE_CURRENT_SOURCE_DIR}/external/KTX/lib/ktx.lib"
    IMPORTED_LOCATION          "${CMAKE_CURRENT_SOURCE_DIR}/external/KTX/bin/ktx.dll"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/external/KTX/include"
)

target_include_directories(Scene
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/external/tinyglTFloader"
)

target_compile_definitions(Scene PRIVATE
    TINYGLTF_NO_STB_IMAGE_WRITE
)

target_link_libraries(Scene
    PUBLIC
        glm::glm
        RHI           
    PRIVATE
        ktx           
)