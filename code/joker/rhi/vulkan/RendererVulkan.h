#pragma once

#include "RendererTypes.h"

namespace joker::rhi::vulkan
{

struct alignas(64) vkRendererContext
{
    u32                    m_uLayerSupportCount;
    u32                    m_uExtensionCount;
    VkLayerProperties*     m_pVkLayers;
    VkExtensionProperties* m_pVkExtensions;
};

struct alignas(64) vkRenderer
{
};

extern RendererContext* vkInitRendererContext(const RendererContextDesc* pDesc);
extern void             vkExitRendererContext(RendererContext* pContext);
extern Renderer*        vkInitRenderer(const RendererDesc* pDesc);
extern void             vkExitRenderer(Renderer* pRenderer);

}