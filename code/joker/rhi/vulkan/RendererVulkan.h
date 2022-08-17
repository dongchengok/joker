#pragma once

#include "RendererTypes.h"

namespace joker::rhi::vulkan
{

struct alignas(64) vkRendererContext
{
    VkInstance               m_hVkInstance;
    VkAllocationCallbacks*   m_pVkAllocationCallbacks;
    u32                      m_uLayerSupportCount;
    u32                      m_uExtensionCount;
    VkLayerProperties*       m_pVkLayers;
    VkExtensionProperties*   m_pVkExtensions;
    VkDebugUtilsMessengerEXT m_hVkDebugMessenger;
};

struct alignas(64) vkRenderer
{
    VkInstance                  m_hVkInstance;
    VkPhysicalDevice            m_hVkDevice;
    VkPhysicalDeviceProperties2 m_hVkDeviceProperties;
    VkDebugUtilsMessengerEXT    m_hVkDebugMessenger;
    u8                          m_uDeviceIndex;
    bool                        m_bOwnInstance;         // 是否是共享的instance，如果一个instance对多个device就是false
};

extern RendererContext* vkInitRendererContext(const RendererContextDesc* pDesc);
extern void             vkExitRendererContext(RendererContext* pContext);
extern Renderer*        vkInitRenderer(const RendererDesc* pDesc);
extern void             vkExitRenderer(Renderer* pRenderer);

}