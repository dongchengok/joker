// void* __cdecl operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
// {
// 	return new unsigned char[size];
// }

// void* __cdecl operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
// {
// 	return new unsigned char[size];
// }
#include <stdio.h>
void *operator new[](size_t size, const char *pName, int flags, unsigned debugFlags, const char *file, int line)
{
    return operator new[](size);
}

void *operator new[](size_t size, size_t /*alignment*/, size_t /*alignmentOffset*/, const char * /*pName*/, int /*flags*/, unsigned /*debugFlags*/, const char * /*file*/, int /*line*/)
{
    return operator new[](size);
}

// #include <EASTL/vector.h>

#include <vk_engine.h>
#include "JokerRHI.h"

#include <EASTL/string.h>

struct TestString
{
    char data[256];
    
    operator const char*() const
    {
        return data;
    }
};

int main(int argc, char *argv[])
{
    // volkInitialize();

    // RHIRendererContext* pContext;
    // RHIRendererContextDesc settings;
    // RHIInitRendererContext("haha", &settings, &pContext);

    spdlog::info("Welcome to spdlog!");
    spdlog::error("Some error message with arg: {}", 1);

    spdlog::warn("Easy padding in numbers like {:08d}", 12);
    spdlog::critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    spdlog::info("Support for floats {:03.2f}", 1.23456);
    spdlog::info("Positional args are {1} {0}..", "too", "supported");
    spdlog::info("{:<30}", "left aligned");
    SPDLOG_DEBUG("Some trace message 11111111111111111with param {}", 42);

    spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    spdlog::debug("This message should be displayed..");

    // spdlog::set_pattern(std::string pattern)

    // change log pattern
    spdlog::set_pattern("[%D %H:%M:%S.%e][%^%l%$][thread:%t] %v");

    // Compile time log levels
    // define SPDLOG_ACTIVE_LEVEL to desired level
    SPDLOG_TRACE("Some trace message with param {}", 42);
    SPDLOG_DEBUG("Some debug message");

    VulkanEngine engine;

    engine.init();

    joker::rhi::DeviceDesc desc;
    desc.szAppName = "test";
    desc.bCPUDebug = true;
    desc.bGPUDebug = false;
    auto pRenderer = joker::rhi::InitDevice(desc);
    joker::rhi::SwapChainDesc descSwapChain;
    descSwapChain.pWindow = engine._window;
    auto pSwapChain = joker::rhi::AddSwapChain(descSwapChain);

    joker::rhi::QueueDesc descQueue;
    descQueue.eType = joker::rhi::EQueueType::Graphics;
    joker::rhi::AddQueue(descQueue);
    descQueue.eType = joker::rhi::EQueueType::Compute;
    joker::rhi::AddQueue(descQueue);
    descQueue.eType = joker::rhi::EQueueType::Transfer;
    joker::rhi::AddQueue(descQueue);

    engine.run();

    engine.cleanup();

    joker::rhi::RemoveSwapChain(pSwapChain);
    joker::rhi::ExitDevice(pRenderer);

    // joker::rhi::ExitRenderer(pRenderer);
    // joker::rhi::ExitRendererContext(pContext);

    return 0;
}
