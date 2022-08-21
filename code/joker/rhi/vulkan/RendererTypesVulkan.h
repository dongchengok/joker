#pragma once

namespace joker::rhi::vulkan
{

struct _vkGPUInfo
{
    VkPhysicalDevice            m_hVkGPU;
    VkPhysicalDeviceProperties2 m_GPUProperties;
};

struct _vkRendererContextDesc
{
    u32          m_uNeedLayersCount;
    const char** m_ppNeedLayers;
    u32          p_uNeedExtensions;
    const char** m_ppNeedExtensions;
};

struct alignas(64) _vkRendererContext
{
    VkInstance               m_hVkInstance;
    VkAllocationCallbacks*   m_pVkAllocationCallbacks;
    u32                      m_uSupportLayersCount;
    VkLayerProperties*       m_pSupportLayers;
    u32                      m_uSupportExtensionsCount;
    VkExtensionProperties*   m_pSupportExtensions;
    u32                      m_uUsedLayersCount;
    const char**             m_ppUsedLayers;
    u32                      m_uUsedExtensionsCount;
    const char**             m_ppUsedExtensions;
    VkDebugUtilsMessengerEXT m_hVkDebugMessenger;
};

struct _vkRendererDesc
{
};

struct alignas(64) _vkRenderer
{
    VkInstance                  m_hVkInstance;
    VkPhysicalDevice            m_hVkDevice;
    VkPhysicalDeviceProperties2 m_hVkDeviceProperties;
    VkDebugUtilsMessengerEXT    m_hVkDebugMessenger;
    u8                          m_uDeviceIndex;
    bool                        m_bOwnInstance; // 是否是共享的instance，如果一个instance对多个device就是false
};

// extern void _vkInitRendererContext(const RendererContextDesc* pDesc, RendererContext** ppContext);
// extern void _vkExitRendererContext(RendererContext* pContext);
// extern void _vkInitRenderer(const RendererDesc* pDesc, Renderer** ppRenderer);
// extern void _vkExitRenderer(Renderer* pRenderer);

}