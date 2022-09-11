#pragma once

#include "JokerVulkan.h"
#include "JokerDevice.h"

namespace joker::rhi::vulkan
{

// TODO 回头把平时用不到的数据挪出去，减少对象大小
class JRHI_ALIGN DeviceVulkan final : public Device
{
    struct GPUInfo
    {
        VkPhysicalDevice     hVkPhysicalDevice;
        string               szVendor;
        string               szModel;
        string               szRevision;
        string               szGPUName;
        string               szGPUDriverVersion;
        string               szGPUDrriverDate;
        EGPUType             eGPUType;
        EWaveOpsSupportFlags eWaveOpsSupportFlags;
        EShadingRate         eShadingRates;
        EShadingRateCaps     eShadingRateCaps;
        VkDeviceSize         uMemoryHeapSize;
        u32                  uApiVersion;
        u32                  uUniformBufferAlignment;
        u32                  uUploadBufferTextureAlignment;
        u32                  uUploadBufferTextureRowAlignment;
        u32                  uMaxVertexInputBindings;
        u32                  uMaxRootSignatureDWORDS;
        u32                  uWaveLaneCount;
        u32                  uShadingRateTexelWidth;
        u32                  uShadingRateTexelHeight;
        u32                  bMultiDrawIndirect       : 1;
        u32                  bROVsSupported           : 1;
        u32                  bTessellationSupported   : 1;
        u32                  bGeometryShaderSupported : 1;
        u32                  bGpuBreadcrumbs          : 1;
        u32                  bHDRSupported            : 1;
        bool                 bValid                   : 1;
    };
    struct DeviceInfo
    {
        u32                    uUsingGPUIndex;
        vector<GPUInfo>        vGPUs;
        VkAllocationCallbacks* pAllocationCallbacks = nullptr;
    };

  public:
    DeviceVulkan(const DeviceDesc& desc);
    virtual ~DeviceVulkan();

    void                  Init();
    void                  Exit();

    virtual n32           GetGPUCount() const override;
    virtual n32           GetGPUUsingIndex() const override;
    virtual const string& GetGPUName(n32 idx) const override;
    virtual const string& GetGPUVendor(n32 idx) const override;
    virtual const string& GetGPUModel(n32 idx) const override;

  private:
    void              _CreateInstance();
    void              _CreateDevice();
    void              _CreateVmaAllocator();

    void              _QueryGPUInfos();
    void              _SelectBestCPU();

    static EGPUVendor _GetGPUVendor(u32 uVendorId);
    static bool       _CheckVersion(u32 uNeedVersion);
    static bool       _CheckAndAddLayer(const char* szName, u32 uCount, VkLayerProperties* pSupports, vector<const char*>& vUsed);
    static bool       _CheckLayer(const char* szName, u32 uCount, VkLayerProperties* pSupprots);
    static bool       _CheckAndAddExtension(const char* szName, u32 uCount, VkExtensionProperties* pSupports, vector<const char*>& vUsed);
    static bool       _CheckExtension(const char* szName, u32 uCount, VkExtensionProperties* pSupports);
    static bool       _DeviceBetterFunc(const GPUInfo& testInfo, const GPUInfo& refInfo);

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
    vector<const char*>         m_vDeviceUsedExtensions;

    // 先不考虑多GPU模式了，手机不可能有。。。
    // bool m_bGroupCreationExtension                 : 1;
    bool m_bRaytracingSupported                    : 1;
    bool m_bDedicatedAllocationExtension           : 1;
    bool m_bGetMemoryRequirement2Extension         : 1;
    bool m_bExternalMemoryExtension                : 1;
    bool m_bExternalMemoryWin32Extension           : 1;
    bool m_bDrawIndirectCountExtension             : 1;
    bool m_bAMDDrawIndirectCountExtension          : 1;
    bool m_bAMDGCNShaderExtension                  : 1;
    bool m_bDescriptorIndexingExtension            : 1;
    bool m_bShaderFloatControlsExtension           : 1;
    bool m_bBufferDeviceAddressExtension           : 1;
    bool m_bDeferredHostOperationsExtension        : 1;
    bool m_bAccelerationStructureExtension         : 1;
    bool m_bSpirv14Extension                       : 1;
    bool m_bRayTracingPipelineExtension            : 1;
    bool m_bRayQueryExtension                      : 1;
    bool m_bSamplerYCbCrConversionExtension        : 1;
    bool m_bFragmentShaderInterlockExtension       : 1;
    bool m_bMultiviewExtension                     : 1;
    bool m_bNvDeviceDiagnosticCheckpointsExtension : 1;
    bool m_bNvDeviceDiagnosticsConfigExtension     : 1;
    bool m_bDebugMarkerSupport                     : 1;

    union {
        struct
        {
            u8 m_uPresentQueueFamilyIndex;
            u8 m_uGraphicsQueueFamilyIndex;
            u8 m_uTransferQueueFamilyIndex;
            u8 m_uComputeQueueFamilyIndex;
        };
        u8 m_QueueFamilyIndices[4];
    };

  public:
    DeviceInfo* m_pInfo;
};

extern Device* InitDeviceVulkan(const DeviceDesc& desc);
extern void    ExitDeviceVulkan(Device* pRenderer);

}