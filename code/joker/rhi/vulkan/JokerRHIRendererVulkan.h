#pragma once

#include "JokerRHIRenderer.h"
#include <vulkan/vulkan_core.h>

namespace joker
{

class JRHI_ALIGN RHIRendererVulkan final : public RHIRenderer
{
  public:
    RHIRendererVulkan(const RHIRendererDesc& desc);
    virtual ~RHIRendererVulkan();

  private:
    void        _CreateInstance();
    void        _CreateDevice();
    void        _CreateVmaAllocator();

    void        _SelectBestCPU();

    static bool _CheckVersion(u32 uNeedVersion);
    static bool _CheckAndAddLayer(const char* szName, u32 uCount, VkLayerProperties* pSupports, vector<const char*>& vUsed);
    static bool _CheckLayer(const char* szName, u32 uCount, VkLayerProperties* pSupprots);
    static bool _CheckAndAddExtension(const char* szName, u32 uCount, VkExtensionProperties* pSupports, vector<const char*>& vUsed);
    static bool _CheckExtension(const char* szName, u32 uCount, VkExtensionProperties* pSupports);
    static bool _DeviceBetterFunc(u32 uTestIndex, u32 uRefIndex, const VkPhysicalDeviceProperties2* pGPUProperties, const VkPhysicalDeviceMemoryProperties* pGPUMemoryProperties);
    static bool _QueryGPUProperties(VkPhysicalDevice gpu, VkPhysicalDeviceProperties2* pProperties, VkPhysicalDeviceMemoryProperties* pMemProperties,
                                    VkPhysicalDeviceFeatures2* pFeatures, VkQueueFamilyProperties** ppQueueFamilyProperties, u32* pQueueFamilyPropertyCount);

  public:
    u32                         m_uInstanceSupportLayersCount     = 0;
    u32                         m_uInstanceSupportExtensionsCount = 0;
    VkLayerProperties*          m_pInstanceSupportLayers          = nullptr;
    VkExtensionProperties*      m_pInstanceSupportExtensions      = nullptr;

    u32                         m_uDeviceSupportLayersCount       = 0;
    u32                         m_uDeviceSupportExtensionsCount   = 0;
    u32                         m_uDeviceSupportFeaturesCount     = 0;

    u32                         m_uDeviceCount                    = 0;

    struct VmaAllocator_T*      m_pVmaAllocator                   = nullptr;
    VkAllocationCallbacks*      m_pVkAllocationCallbacks          = nullptr;
    VkDebugUtilsMessengerEXT    m_hVkDebugMessenger               = nullptr;
    VkPhysicalDevice            m_hVkActiveDevice                 = nullptr;
    VkPhysicalDeviceProperties2 m_VkActiveDeviceProperties;

    vector<const char*>         m_vInstanceUsedLayers;
    vector<const char*>         m_vInstanceUsedExtensions;

    bool                        m_bRaytracingSupported                    : 1;
    bool                        m_bDedicatedAllocationExtension           : 1;
    bool                        m_bGetMemoryRequirement2Extension         : 1;
    bool                        m_bExternalMemoryExtension                : 1;
    bool                        m_bExternalMemoryWin32Extension           : 1;
    bool                        m_bDrawIndirectCountExtension             : 1;
    bool                        m_bAMDDrawIndirectCountExtension          : 1;
    bool                        m_bAMDGCNShaderExtension                  : 1;
    bool                        m_bDescriptorIndexingExtension            : 1;
    bool                        m_bShaderFloatControlsExtension           : 1;
    bool                        m_bBufferDeviceAddressExtension           : 1;
    bool                        m_bDeferredHostOperationsExtension        : 1;
    bool                        m_bAccelerationStructureExtension         : 1;
    bool                        m_bSpirv14Extension                       : 1;
    bool                        m_bRayTracingPipelineExtension            : 1;
    bool                        m_bRayQueryExtension                      : 1;
    bool                        m_bSamplerYCbCrConversionExtension        : 1;
    bool                        m_bFragmentShaderInterlockExtension       : 1;
    bool                        m_bMultiviewExtension                     : 1;
    bool                        m_bNvDeviceDiagnosticCheckpointsExtension : 1;
    bool                        m_bNvDeviceDiagnosticsConfigExtension     : 1;
    bool                        m_bDebugMarkerSupport                     : 1;

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

extern RHIRenderer* RHIInitRendererVulkan(const RHIRendererDesc& desc);
extern void         RHIExitRendererVulkan(RHIRenderer* pRenderer);

}