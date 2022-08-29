#include "JokerRHIPCH.h"
#include "JokerRHIRendererVulkan.h"

namespace joker
{

RHIRenderer* RHIInitRendererVulkan(const RHIRendererDesc& desc)
{
    return new RHIRendererVulkan();
}

void RHIExitRendererVulkan(RHIRenderer* pRenderer)
{
    delete pRenderer;
}

RHIRendererVulkan::RHIRendererVulkan()
{
}

RHIRendererVulkan::~RHIRendererVulkan()
{
}

}