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
    u32          m_uNeedExtensionsCount;
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
    u32          m_uInstanceLayerCount;
    u32          m_uInstanceExtensionCount;
    u32          m_uDeviceExtensionCount;
    const char** m_ppInstanceLayers;
    const char** m_ppInstanceExtensions;
    const char** m_ppDeviceExtensions;
    bool         m_bRequestAllAvailableQueues; //使用所有的queue，可能会多出200mb内存
};

struct alignas(64) _vkRenderer
{
    VkPhysicalDevice             m_hVkActiveGPU;
    VkPhysicalDeviceProperties2* m_pVkActiveGPUProperties;
    VkDevice                     m_hVkDevice;
    u8                           m_uRendererIndex;
    u32**                        pAvailableQueueCount;
    u32**                        pUsedQueueCount;
    VkDescriptorPool             pEmptyDescriptorPool;
    VkDescriptorSetLayout        pEmptyDescriptorSetLayout;
    VkDescriptorSet              pEmptyDescriptorSet;
    struct VmaAllocator_T*       pVmaAllocator;
    bool                         m_bRaytracingSupported               : 1;
    bool                         m_bYCbCrExtension                    : 1;
    bool                         m_bKHRSpirv14Extension               : 1;
    bool                         m_bKHRAccelerationStructureExtension : 1;
    bool                         m_bKHRRayTracingPipelineExtension    : 1;
    bool                         m_bKHRRayQueryExtension              : 1;
    bool                         m_bAMDGCNShaderExtension             : 1;
    bool                         m_bAMDDrawIndirectCountExtension     : 1;
    bool                         m_bDescriptorIndexingExtension       : 1;
    bool                         m_bShaderFloatControlsExtension      : 1;
    bool                         m_bBufferDeviceAddressExtension      : 1;
    bool                         m_bDeferredHostOperationsExtension   : 1;
    bool                         m_bDrawIndirectCountExtension        : 1;
    bool                         m_bDedicatedAllocationExtension      : 1;
    bool                         m_bExternalMemoryExtension           : 1;
    bool                         m_bDebugMarkerSupport                : 1;
    // 是否是共享的instance，如果一个instance对多个device就是false
    bool m_bOwnInstance : 1;
    union {
        struct
        {
            u8 m_uGraphicsQueueFamilyIndex;
            u8 m_uTransferQueueFamilyIndex;
            u8 m_uComputeQueueFamilyIndex;
        };
        u8 m_QueueFamilyIndices[3];
    };
};

// extern void _vkInitRendererContext(const RendererContextDesc* pDesc, RendererContext** ppContext);
// extern void _vkExitRendererContext(RendererContext* pContext);
// extern void _vkInitRenderer(const RendererDesc* pDesc, Renderer** ppRenderer);
// extern void _vkExitRenderer(Renderer* pRenderer);

}