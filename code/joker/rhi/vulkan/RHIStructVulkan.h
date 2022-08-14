#pragma once

#include <vulkan/vulkan_core.h>
struct RHIRendererContextDescVulkan
{
    const char** m_ppInstanceLayers;
    const char** m_ppInstanceExtensions;
    u32          m_uInstanceLayerCount;
    u32          m_uInstanceExtensionCount;
};

struct alignas(64) RHIRendererContextVulkan
{
    VkInstance               m_hVkInstance;
    VkDebugUtilsMessengerEXT m_hVkDebugUtilsMessager;
    bool                     m_bExtDeviceGroupCreation : 1;
    bool                     m_bExtUtilsExtension      : 1;
};

struct RHIRendererDescVulkan
{
    const char** m_ppInstanceLayers;
    const char** m_ppInstanceExtensions;
    const char** m_ppDeviceExtensions;
    u32          m_uInstanceLayerCount;
    u32          m_uInstanceExtensionCount;
    u32          m_uDeviceExtensionCount;
    bool         m_bRequestAllAvailableQueue : 1; //每种类型的queue请求一个，还是请求全部，影响内存占用约200mb
};

struct RHIRendererVulkan
{
    VkInstance                  m_hVkInstance;
    VkPhysicalDevice            m_vkActiveGPU;
    VkPhysicalDeviceProperties2 m_vkActiveGPUProperties;
    VkDebugUtilsMessengerEXT    m_hVkDebugUtilsMessenger;
    u32**                       m_ppAvailableQueueCount;
    u32**                       m_ppUsedQueueCount;
    VkDescriptorPool            m_vkEmptyDescriptorPool;
    VkDescriptorSetLayout       m_vkEmptyDescriptorSetLayout;
    VkDescriptorSet             m_vkEmptyDescriptorSet;
    struct VmaAllocator_T*      m_pVmaAllocator;
    bool                        m_bRaytracingSupported               : 1;
    bool                        m_bYCbCrExtension                    : 1;
    bool                        m_bKHRSpirv14Extension               : 1;
    bool                        m_bKHRAccelerationStructureExtension : 1;
    bool                        m_bKHRRayQueryExtension              : 1;
    bool                        m_bAMDGCNShaderExtension             : 1;
    bool                        m_bAMDDrawIndirectCountExtensions    : 1;
    bool                        m_bDescriptorIndexingExtension       : 1;
    bool                        m_bBufferDeviceAddressExtension      : 1;
    bool                        m_bDeferredHostOperationsExtension   : 1;
    bool                        m_bDrawIndirectCountExtension        : 1;
    bool                        m_bDedicatedAllocationExtension      : 1;
    bool                        m_bExternalMemoryExtension           : 1;
    bool                        m_bDebugMarkerSupport                : 1;
    bool                        m_bOwnInstance                       : 1; // unlink模式会共享instance，这里是false
    union {
        struct
        {
            u8 m_uGraphicsQueueFamilyIndex;
            u8 m_uTransferQueueFamilyIndex;
            u8 m_uComputeQueueFamilyIndex;
        };
        u8 m_uQueueFamilyIndices[3];
    };
};

struct RHIGPUInfoVulkan
{
    VkPhysicalDevice            m_hVkGPU;
    VkPhysicalDeviceProperties2 m_hVkGPUProperties;
};

struct RHICommandSignatureVulkan
{
};

namespace joker::rhi
{
    struct GPUInfo
    {

    };

    
}