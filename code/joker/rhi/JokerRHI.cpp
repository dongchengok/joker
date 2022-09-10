#include "JokerRHIPCH.h"
#include "JokerRHI.h"
#include "vulkan/JokerRendererVulkan.h"

namespace joker::rhi
{

Renderer* InitRenderer(const RendererDesc& desc)
{
    return joker::rhi::vulkan::InitRendererVulkan(desc);
}

void ExitRenderer(Renderer* pRenderer)
{
    joker::rhi::vulkan::ExitRendererVulkan(pRenderer);
}

}