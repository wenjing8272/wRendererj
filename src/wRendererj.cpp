// wRendererj.cpp: 定义应用程序的入口点。
//

#include <VkBootstrap.h>
#include <iostream>

int main()
{
    vkb::InstanceBuilder builder;
    auto instRet = builder
        .set_app_name("Test")
        .request_validation_layers()
        .use_default_debug_messenger()
        .set_headless()        // 测试阶段先不需要窗口和 Surface
        .build();

    if (!instRet)
    {
        std::cerr << "Instance failed: " << instRet.error().message() << "\n";
        return -1;
    }
    vkb::Instance instance = instRet.value();

    vkb::PhysicalDeviceSelector selector{ instance };
    auto physRet = selector.set_minimum_version(1, 3).select();
    if (!physRet)
    {
        std::cerr << "No suitable GPU: " << physRet.error().message() << "\n";
        vkb::destroy_instance(instance);
        return -1;
    }

    std::cout << "GPU: " << physRet.value().name << "\n";

    vkb::destroy_instance(instance);
    return 0;
}