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
    const char**             m_pszUsedLayers;
    u32                      m_uUsedExtensionsCount;
    const char**             m_pszUsedExtensions;
    VkDebugUtilsMessengerEXT m_hVkDebugMessenger;
};

struct _vkRendererDesc
{
    u32          m_uInstanceNeedLayerCount;
    u32          m_uInstanceNeedExtensionCount;
    u32          m_uDeviceNeedExtensionCount;
    const char** m_pszInstanceNeedLayers;
    const char** m_pszInstanceNeedExtensions;
    const char** m_pszDeviceNeedExtensions;
    bool         m_bRequestAllAvailableQueues; //使用所有的queue，可能会多出200mb内存
};

struct alignas(64) _vkRenderer
{
    VkPhysicalDevice             m_hVkActiveGPU;
    VkPhysicalDeviceProperties2* m_pVkActiveGPUProperties;
    VkDevice                     m_hVkDevice;
    u32                          m_uDeviceUsedExtensionsCount;
    const char**                 m_pszDeviceUsedExtensions;
    u8                           m_uRendererIndex;
    u32**                        m_pAvailableQueueCount;
    u32**                        m_pUsedQueueCount;
    VkDescriptorPool             m_hVkEmptyDescriptorPool;
    VkDescriptorSetLayout        m_hVkEmptyDescriptorSetLayout;
    VkDescriptorSet              m_hVkEmptyDescriptorSet;
    struct VmaAllocator_T*       m_pVmaAllocator;
    bool                         m_bRaytracingSupported                    : 1;
    bool                         m_bDedicatedAllocationExtension           : 1;
    bool                         m_bGetMemoryRequirement2Extension         : 1;
    bool                         m_bExternalMemoryExtension                : 1;
    bool                         m_bExternalMemoryWin32Extension           : 1;
    bool                         m_bDrawIndirectCountExtension             : 1;
    bool                         m_bAMDDrawIndirectCountExtension          : 1;
    bool                         m_bAMDGCNShaderExtension                  : 1;
    bool                         m_bDescriptorIndexingExtension            : 1;
    bool                         m_bShaderFloatControlsExtension           : 1;
    bool                         m_bBufferDeviceAddressExtension           : 1;
    bool                         m_bDeferredHostOperationsExtension        : 1;
    bool                         m_bAccelerationStructureExtension         : 1;
    bool                         m_bSpirv14Extension                       : 1;
    bool                         m_bRayTracingPipelineExtension            : 1;
    bool                         m_bRayQueryExtension                      : 1;
    bool                         m_bSamplerYCbCrConversionExtension        : 1;
    bool                         m_bFragmentShaderInterlockExtension       : 1;
    bool                         m_bMultiviewExtension                     : 1;
    bool                         m_bNvDeviceDiagnosticCheckpointsExtension : 1;
    bool                         m_bNvDeviceDiagnosticsConfigExtension     : 1;
    bool                         m_bDebugMarkerSupport                     : 1;
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