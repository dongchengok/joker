#include "JokerRHIPCH.h"
#include "JokerRHI.h"
#include "vulkan/JokerDeviceVulkan.h"

namespace joker::rhi
{

Device* InitDevice(const DeviceDesc& desc)
{
    return joker::rhi::vulkan::InitDeviceVulkan(desc);
}

void ExitDevice(Device* pRenderer)
{
    joker::rhi::vulkan::ExitDeviceVulkan(pRenderer);
}

JRHI_IMPL_FUNC(SwapChain*, AddSwapChain, const SwapChainDesc&);
JRHI_IMPL_FUNC(void, RemoveSwapChain, SwapChain*);

}