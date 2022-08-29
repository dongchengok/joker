#include "JokerRHIPCH.h"
#include "JokerRHI.h"
#include "vulkan/JokerRHIRendererVulkan.h"

namespace joker
{

RHIRenderer* RHIInitRenderer(const RHIRendererDesc& desc)
{
    return RHIInitRendererVulkan(desc);
}

void RHIExitRenderer(RHIRenderer* pRenderer)
{
    RHIExitRendererVulkan(pRenderer);
}

}