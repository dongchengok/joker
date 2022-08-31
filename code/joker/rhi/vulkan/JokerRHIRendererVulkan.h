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

  public:
    u32                      m_uInstanceSupportLayersCount     = 0;
    u32                      m_uInstanceSupportExtensionsCount = 0;
    VkLayerProperties*       m_pInstanceSupportLayers          = nullptr;
    VkExtensionProperties*   m_pInstanceSupportExtensions      = nullptr;

    u32                      m_uDeviceSupportLayersCount       = 0;
    u32                      m_uDeviceSupportExtensionsCount   = 0;
    u32                      m_uDeviceSupportFeaturesCount     = 0;

    u32                      m_uDeviceCount                    = 0;

    VkAllocationCallbacks*   m_pVkAllocationCallbacks          = nullptr;
    VkDebugUtilsMessengerEXT m_hVkDebugMessenger               = nullptr;

    vector<const char*>      m_vInstanceUsedLayers;
    vector<const char*>      m_vInstanceUsedExtensions;
};

extern RHIRenderer* RHIInitRendererVulkan(const RHIRendererDesc& desc);
extern void         RHIExitRendererVulkan(RHIRenderer* pRenderer);

}