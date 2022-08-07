#include "CoreType.h"
#include "JokerRHIPCH.h"
#include "RHIFunction.h"
#include "RHIStruct.h"
#include <corecrt_malloc.h>

#if defined(JOPTION_RHI_MULTI)
#define JIMPL_RHI_FUNC_VK(ret, name, ...) ret name##VK(__VA_ARGS__)
#define JREG_RHI_FUNC_VK(name)            name = name##VK;
#else
#define JIMPL_RHI_FUNC_VK(ret, name, ...) ret name(__VA_ARGS__)
#define JREG_RHI_FUNC_VK(name)
#endif

JIMPL_RHI_FUNC_VK(void, RHIAddFence, RHIRenderer* pRenderer, RHIFence** ppFence)
{
}

JIMPL_RHI_FUNC_VK(void, RHIRemoveFence, RHIRenderer* pRenderer, RHIFence* pFence)
{
}

void RHIInitVulkanAPI()
{
    JREG_RHI_FUNC_VK(RHIAddFence)
    JREG_RHI_FUNC_VK(RHIRemoveFence)
}

bool RHIInitCommonVK(const char* szAppName, const RHIRendererDesc* pDesc, RHIRenderer* pRenderer)
{
    return false;
}

void RHIInitRendererContextVK(const char* szAppName, const RHIRendererContextDesc* pSettings, RHIRendererContext** ppContext)
{
    JASSERT(szAppName);
    JASSERT(pSettings);
    JASSERT(ppContext);
    // JASSERT(gRendererCount);

    RHIRendererDesc fakeDesc{};
    fakeDesc.Vulkan.m_uInstanceLayerCount     = pSettings->Vulkan.m_uInstanceLayerCount;
    fakeDesc.Vulkan.m_uInstanceExtensionCount = pSettings->Vulkan.m_uInstanceExtensionCount;
    fakeDesc.Vulkan.m_ppInstanceExtensions    = pSettings->Vulkan.m_ppInstanceExtensions;
    fakeDesc.Vulkan.m_ppInstanceLayers        = pSettings->Vulkan.m_ppInstanceLayers;
    fakeDesc.m_bEnableGPUBasedValidation      = pSettings->m_bEnableGPUBasedValidation;

    RHIRenderer fakeRenderer{};
    if (!RHIInitCommonVK(szAppName, &fakeDesc, &fakeRenderer))
    {
        return;
    }
    // RHIRendererContext* pContext = (RHIRendererContext*)malloc(size_t Size)
}