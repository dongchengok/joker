#include "JokerRHIPCH.h"
#include "RendererTypes.h"
#include "RendererPrivateVulkan.h"
#include "RendererInit.h"

namespace joker::rhi::vulkan
{

constexpr u32                kQueueMaxFamilies = 16;
constexpr u32                kQueueMaxCount    = 64;
constexpr u32                kMaxQueueFlag     = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT | VK_QUEUE_PROTECTED_BIT;

constexpr static const char* kInstanceLayers[] = {
    "",
};

//必须支持的扩展
constexpr static const char* kInstanceExtensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(_WIN32)
    "VK_KHR_win32_surface",
#endif
    VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME, //干啥的不知道，TODO
    VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,       //合法使用HDR格式
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
};

constexpr static const char* kDeviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_MAINTENANCE1_EXTENSION_NAME,
    VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
    VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME,
    VK_EXT_SHADER_SUBGROUP_VOTE_EXTENSION_NAME,
    VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
    VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
    VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
    VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
    VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
    VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME,
#endif
#if VK_KHR_draw_indirect_count
    VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
#endif
#if VK_EXT_fragment_shader_interlock
    VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME,
#endif
    VK_AMD_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
    VK_AMD_SHADER_BALLOT_EXTENSION_NAME,
    VK_AMD_GCN_SHADER_EXTENSION_NAME,
#if VK_KHR_device_group
    VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
#endif
    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
#if VK_KHR_maintenance3 // descriptor indexing depends on this
    VK_KHR_MAINTENANCE3_EXTENSION_NAME,
#endif
#ifdef VK_RAYTRACING_AVAILABLE
    VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,

    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
    VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,

    VK_KHR_RAY_QUERY_EXTENSION_NAME,
#endif
#if VK_KHR_bind_memory2
    // Requirement for VK_KHR_sampler_ycbcr_conversion
    VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
#endif
#if VK_KHR_sampler_ycbcr_conversion
    VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME,
#if VK_KHR_bind_memory2 // ycbcr conversion depends on this
    VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
#endif
#endif
};

// TODO
constexpr static VkAllocationCallbacks* kAllocator = nullptr;

// 检查版本号
static bool _vkCheckVersion(u32 uNeedVersion)
{
    u32 uVersion;
    vkEnumerateInstanceVersion(&uVersion);
    if (uVersion >= uNeedVersion)
    {
        return true;
    }
    JLOG_ERROR("need version:{}.{}.{}.{} but only:{}.{}.{}.{}", VK_API_VERSION_VARIANT(uNeedVersion), VK_API_VERSION_MAJOR(uNeedVersion), VK_API_VERSION_MINOR(uNeedVersion),
               VK_API_VERSION_PATCH(uNeedVersion), VK_API_VERSION_VARIANT(uVersion), VK_API_VERSION_MAJOR(uVersion), VK_API_VERSION_MINOR(uVersion),
               VK_API_VERSION_PATCH(uVersion));
    return false;
}

// 检查验证层
static bool _vkCheckNAddValidationLayer(const char* szName, u32 uCount, const VkLayerProperties* pLayers, u32* pUsedCount, const char** pszUsedValidationLayers)
{
    for (u32 i = 0; i < *pUsedCount; ++i)
    {
        if (strcmp(szName, pszUsedValidationLayers[i]) == 0)
        {
            return true;
        }
    }
    for (u32 i = 0; i < uCount; ++i)
    {
        if (strcmp(szName, pLayers[i].layerName) == 0)
        {
            pszUsedValidationLayers[(*pUsedCount)++] = pLayers[i].layerName;
            return true;
        }
    }
    return false;
}

static bool _vkCheckNAddValidationLayer(const char* szName, u32 uCount, const VkLayerProperties* pLayers, u32* pUsedCount, const char** pszUsedValidationLayers, bool log)
{
    bool bRet = _vkCheckNAddValidationLayer(szName, uCount, pLayers, pUsedCount, pszUsedValidationLayers);
    if (!bRet)
    {
        JLOG_WARN("can not find layer {}", szName);
    }
    return bRet;
}

static bool _vkCheckValidationLayer(const char* szName, u32 uCount, const VkLayerProperties* pLayers)
{
    for (u32 i = 0; i < uCount; ++i)
    {
        if (strcmp(szName, pLayers[i].layerName) == 0)
        {
            return true;
        }
    }
    return false;
}

static bool _vkCheckValidationLayer(_vkRendererContext* pContext, const char* szName)
{
    return _vkCheckValidationLayer(szName, pContext->m_uSupportLayersCount, pContext->m_pSupportLayers);
}

// 检查扩展
static bool _vkCheckNAddExtension(const char* szName, u32 uCount, const VkExtensionProperties* pExts, u32* pUsedCount, const char** pszUsedExtensions)
{
    for (u32 i = 0; i < *pUsedCount; ++i)
    {
        if (strcmp(szName, pszUsedExtensions[i]) == 0)
        {
            return true;
        }
    }
    for (u32 i = 0; i < uCount; ++i)
    {
        if (strcmp(szName, pExts[i].extensionName) == 0)
        {
            pszUsedExtensions[(*pUsedCount)++] = pExts[i].extensionName;
            return true;
        }
    }

    return false;
}

inline static bool _vkCheckNAddExtension(const char* szName, u32 uCount, const VkExtensionProperties* pExts, u32* pUsedCount, const char** pszUsedExtensions, bool log)
{
    bool ret = _vkCheckNAddExtension(szName, uCount, pExts, pUsedCount, pszUsedExtensions);
    if (!ret)
    {
        JLOG_WARN("can not find extension {}", szName);
    }
    return ret;
}

inline static bool _vkCheckNAddExtension(const char* szName, u32 uCount, const VkExtensionProperties* pExts, Renderer* pRenderer)
{
    return _vkCheckNAddExtension(szName, uCount, pExts, &pRenderer->Vulkan.m_uDeviceUsedExtensionsCount, pRenderer->Vulkan.m_pszDeviceUsedExtensions);
}

inline static bool _vkCheckNAddExtension(const char* szName, u32 uCount, const VkExtensionProperties* pExts, Renderer* pRenderer, bool log)
{
    return _vkCheckNAddExtension(szName, uCount, pExts, &pRenderer->Vulkan.m_uDeviceUsedExtensionsCount, pRenderer->Vulkan.m_pszDeviceUsedExtensions, log);
}

static bool _vkCheckExtension(const char* szName, u32 uCount, const VkExtensionProperties* pExtensions)
{
    for (u32 i = 0; i < uCount; ++i)
    {
        if (strcmp(szName, pExtensions[i].extensionName) == 0)
        {
            return true;
        }
    }
    return false;
}

static bool _vkCheckExtension(_vkRendererContext* pContext, const char* szName)
{
    return _vkCheckExtension(szName, pContext->m_uSupportExtensionsCount, pContext->m_pSupportExtensions);
}

static inline u32 _kVkVendorIDNvidia = 0x10DE;
static inline u32 _kVkVendorIDAmd    = 0x1002;
static inline u32 _kVkVendorIDAmd1   = 0x1022;
static inline u32 _kVkVendorIDIntel  = 0x163C;
static inline u32 _kVkVendorIDIntel1 = 0x8086;
static inline u32 _kVkVendorIDIntel2 = 0x8087;

static EGPUVendor _vkGetGPUVendor(u32 uVendorId)
{
    if (uVendorId == _kVkVendorIDNvidia)
    {
        return EGPUVendor::Nvidia;
    }
    else if (uVendorId == _kVkVendorIDAmd || uVendorId == _kVkVendorIDAmd1)
    {
        return EGPUVendor::Amd;
    }
    else if (uVendorId == _kVkVendorIDIntel || uVendorId == _kVkVendorIDIntel1 || uVendorId == _kVkVendorIDIntel2)
    {
        return EGPUVendor::Intel;
    }
    else
    {
        //还没处理手机的
        return EGPUVendor::Unknown;
    }
}

static VkBool32 VKAPI_PTR _vkDebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    const char* pLayerPrefix = pCallbackData->pMessageIdName;
    const char* pMessage     = pCallbackData->pMessage;
    n32         messageCode  = pCallbackData->messageIdNumber;
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
    {
        JLOG_INFO("[{}] : {} ({})", pLayerPrefix, pMessage, messageCode);
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        JLOG_WARN("[{}] : {} ({})", pLayerPrefix, pMessage, messageCode);
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        JLOG_ERROR("[{}] : {} ({})", pLayerPrefix, pMessage, messageCode);
        JASSERT(false);
    }

    return VK_FALSE;
}

static bool _vkDeviceBetterFunc(u32 uTestIndex, u32 uRefIndex, const GPUSettings* pGPUSettings, const VkPhysicalDeviceProperties2* pGPUProperties,
                                const VkPhysicalDeviceMemoryProperties* pGPUMemoryProperties)
{
    const GPUSettings& testSettings = pGPUSettings[uTestIndex];
    const GPUSettings& refSettings  = pGPUSettings[uRefIndex];

    // 如果等级都不一样，直接选等级就行了
    if (testSettings.m_GPUVendorPreset.m_ePresetLevel != refSettings.m_GPUVendorPreset.m_ePresetLevel)
    {
        return testSettings.m_GPUVendorPreset.m_ePresetLevel > refSettings.m_GPUVendorPreset.m_ePresetLevel;
    }

    // 处理一个是独显，一个不是独显的情况
    const VkPhysicalDeviceProperties& testProps = pGPUProperties[uTestIndex].properties;
    const VkPhysicalDeviceProperties& refProps  = pGPUProperties[uRefIndex].properties;
    if (testProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && refProps.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        return true;
    }
    if (testProps.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && refProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        return false;
    }

    // 都一样的情况下按显存挑,只算显卡的独立显存
    if (testProps.vendorID == refProps.vendorID && testProps.deviceID == refProps.deviceID)
    {
        const VkPhysicalDeviceMemoryProperties& testMemoryProps = pGPUMemoryProperties[uTestIndex];
        const VkPhysicalDeviceMemoryProperties& refMemoryProps  = pGPUMemoryProperties[uRefIndex];
        VkDeviceSize                            totalTestVram   = 0;
        VkDeviceSize                            totalRefVram    = 0;
        for (uint32_t i = 0; i < testMemoryProps.memoryHeapCount; ++i)
        {
            if (VK_MEMORY_HEAP_DEVICE_LOCAL_BIT & testMemoryProps.memoryHeaps[i].flags)
                totalTestVram += testMemoryProps.memoryHeaps[i].size;
        }
        for (uint32_t i = 0; i < refMemoryProps.memoryHeapCount; ++i)
        {
            if (VK_MEMORY_HEAP_DEVICE_LOCAL_BIT & refMemoryProps.memoryHeaps[i].flags)
                totalRefVram += refMemoryProps.memoryHeaps[i].size;
        }
        return totalTestVram >= totalRefVram;
    }
    return false;
};

static void _vkQueryGPUProperties(VkPhysicalDevice gpu, VkPhysicalDeviceProperties2* pGPUProperties, VkPhysicalDeviceMemoryProperties* pGPUMemoryProperties,
                                  VkPhysicalDeviceFeatures2* pGPUFeatures, VkQueueFamilyProperties** ppQueueFamilyProperties, u32* pQueueFamilyPropertyCount,
                                  GPUSettings* pGPUSettings)
{
    vkGetPhysicalDeviceMemoryProperties(gpu, pGPUMemoryProperties);

    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT featureFragmentInterlock;
    featureFragmentInterlock.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT;
    featureFragmentInterlock.pNext = nullptr;
    pGPUFeatures->sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pGPUFeatures->pNext            = &featureFragmentInterlock;
    vkGetPhysicalDeviceFeatures2(gpu, pGPUFeatures);

    VkPhysicalDeviceSubgroupProperties propsSubgroup;
    propsSubgroup.sType   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
    propsSubgroup.pNext   = nullptr;
    pGPUProperties->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    pGPUProperties->pNext = &propsSubgroup;
    vkGetPhysicalDeviceProperties2(gpu, pGPUProperties);

    vkGetPhysicalDeviceQueueFamilyProperties(gpu, pQueueFamilyPropertyCount, nullptr);
    *ppQueueFamilyProperties = (VkQueueFamilyProperties*)JCALLOC(*pQueueFamilyPropertyCount, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, pQueueFamilyPropertyCount, *ppQueueFamilyProperties);

    JCLEAR(pGPUSettings, sizeof(GPUSettings));
    pGPUSettings->m_uUniformBufferAlignment          = (u32)pGPUProperties->properties.limits.minUniformBufferOffsetAlignment;
    pGPUSettings->m_uUploadBufferTextureAlignment    = (u32)pGPUProperties->properties.limits.optimalBufferCopyOffsetAlignment;
    pGPUSettings->m_uUploadBufferTextureRowAlignment = (u32)pGPUProperties->properties.limits.optimalBufferCopyRowPitchAlignment;
    pGPUSettings->m_uMaxVertexInputBindings          = pGPUProperties->properties.limits.maxVertexInputBindings;
    pGPUSettings->m_bMultiDrawIndirect               = pGPUFeatures->features.multiDrawIndirect;
    pGPUSettings->m_uWaveLaneCount                   = propsSubgroup.subgroupSize;
    pGPUSettings->m_bROVsSupported                   = featureFragmentInterlock.fragmentShaderPixelInterlock;
    pGPUSettings->m_bTessellationSupported           = pGPUFeatures->features.tessellationShader;
    pGPUSettings->m_bGeometryShaderSupported         = pGPUFeatures->features.geometryShader;

    pGPUSettings->m_eWaveOpsSupportFlags             = EWaveOpsSupportFlags::None;
    if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_BASIC_BIT)
        pGPUSettings->m_eWaveOpsSupportFlags |= EWaveOpsSupportFlags::BasicBit;
    if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_VOTE_BIT)
        pGPUSettings->m_eWaveOpsSupportFlags |= EWaveOpsSupportFlags::VoteBit;
    if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT)
        pGPUSettings->m_eWaveOpsSupportFlags |= EWaveOpsSupportFlags::ArithmeticBit;
    if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_BALLOT_BIT)
        pGPUSettings->m_eWaveOpsSupportFlags |= EWaveOpsSupportFlags::BallotBit;
    if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_BIT)
        pGPUSettings->m_eWaveOpsSupportFlags |= EWaveOpsSupportFlags::ShuffleBit;
    if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT)
        pGPUSettings->m_eWaveOpsSupportFlags |= EWaveOpsSupportFlags::ShuffleRelativeBit;
    if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_CLUSTERED_BIT)
        pGPUSettings->m_eWaveOpsSupportFlags |= EWaveOpsSupportFlags::ClusteredBit;
    if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_QUAD_BIT)
        pGPUSettings->m_eWaveOpsSupportFlags |= EWaveOpsSupportFlags::QuadBit;
    if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV)
        pGPUSettings->m_eWaveOpsSupportFlags |= EWaveOpsSupportFlags::PartitionedBitNV;

    sprintf_s(pGPUSettings->m_GPUVendorPreset.m_szModelId, JMAX_NAME_LENGTH, "%#x", pGPUProperties->properties.deviceID);
    sprintf_s(pGPUSettings->m_GPUVendorPreset.m_szVendorId, JMAX_NAME_LENGTH, "%#x", pGPUProperties->properties.vendorID);
    strncpy_s(pGPUSettings->m_GPUVendorPreset.m_szGPUName, pGPUProperties->properties.deviceName, JMAX_NAME_LENGTH);
    strncpy_s(pGPUSettings->m_GPUVendorPreset.m_szRevisionId, "0x00", JMAX_NAME_LENGTH);
    pGPUSettings->m_GPUVendorPreset.m_ePresetLevel = EGPUPresetLevel::Low;
    // getGPUPresetLevel(pGPUSettings->m_GPUVendorPreset.m_szVendorId, pGPUSettings->m_GPUVendorPreset.m_szModelId,
    // pGPUSettings->m_GPUVendorPreset.m_szRevisionId);

    // fill in driver info
    uint32_t major, minor, secondaryBranch, tertiaryBranch;
    switch (_vkGetGPUVendor(pGPUProperties->properties.vendorID))
    {
    case EGPUVendor::Nvidia:
        major           = (pGPUProperties->properties.driverVersion >> 22) & 0x3ff;
        minor           = (pGPUProperties->properties.driverVersion >> 14) & 0x0ff;
        secondaryBranch = (pGPUProperties->properties.driverVersion >> 6) & 0x0ff;
        tertiaryBranch  = (pGPUProperties->properties.driverVersion) & 0x003f;

        sprintf_s(pGPUSettings->m_GPUVendorPreset.m_szGPUDriverVersion, JMAX_NAME_LENGTH, "%u.%u.%u.%u", major, minor, secondaryBranch, tertiaryBranch);
        break;
    default:
        sprintf_s(pGPUSettings->m_GPUVendorPreset.m_szGPUDriverVersion, JMAX_NAME_LENGTH, "%u.%u.%u", VK_VERSION_MAJOR(pGPUProperties->properties.driverVersion),
                  VK_VERSION_MINOR(pGPUProperties->properties.driverVersion), VK_VERSION_PATCH(pGPUProperties->properties.driverVersion));
        break;
    }

    pGPUFeatures->pNext   = nullptr;
    pGPUProperties->pNext = nullptr;
}

static bool _vkSelectBestGPU(Renderer* pRenderer)
{
    u32 uGPUCount = 0;
    JCHECK_RHI_RESULT(vkEnumeratePhysicalDevices(pRenderer->m_pContext->Vulkan.m_hVkInstance, &uGPUCount, nullptr));
    JASSERT(uGPUCount > 0);

    VkInstance                        hVkInstance               = pRenderer->m_pContext->Vulkan.m_hVkInstance;
    VkPhysicalDevice*                 pGPUs                     = (VkPhysicalDevice*)alloca(uGPUCount * sizeof(VkPhysicalDevice));
    VkPhysicalDeviceProperties2*      pGPUProperties            = (VkPhysicalDeviceProperties2*)alloca(uGPUCount * sizeof(VkPhysicalDeviceProperties2));
    VkPhysicalDeviceMemoryProperties* pGPUMemoryProperties      = (VkPhysicalDeviceMemoryProperties*)alloca(uGPUCount * sizeof(VkPhysicalDeviceMemoryProperties));
    VkPhysicalDeviceFeatures2KHR*     pGPUFeatures              = (VkPhysicalDeviceFeatures2KHR*)alloca(uGPUCount * sizeof(VkPhysicalDeviceFeatures2KHR));
    VkQueueFamilyProperties**         ppQueueFamilyProperties   = (VkQueueFamilyProperties**)alloca(uGPUCount * sizeof(VkQueueFamilyProperties*));
    u32*                              pQueueFamilyPropertyCount = (u32*)alloca(uGPUCount * sizeof(u32));

    JCHECK_RHI_RESULT(vkEnumeratePhysicalDevices(hVkInstance, &uGPUCount, pGPUs));

    u32          uGPUIndex    = UINT32_MAX;
    GPUSettings* pGPUSettings = (GPUSettings*)alloca(uGPUCount * sizeof(GPUSettings));
    for (uint32_t i = 0; i < uGPUCount; ++i)
    {
        _vkQueryGPUProperties(pGPUs[i], &pGPUProperties[i], &pGPUMemoryProperties[i], &pGPUFeatures[i], &ppQueueFamilyProperties[i], &pQueueFamilyPropertyCount[i],
                              &pGPUSettings[i]);

        JLOG_INFO("GPU[{}] detected. Vendor ID:{}, Model ID: {}, Preset: {}, GPU Name: {}, driver version: {}", i, pGPUSettings[i].m_GPUVendorPreset.m_szVendorId,
                  pGPUSettings[i].m_GPUVendorPreset.m_szModelId, EGPUPresetLevelToString(pGPUSettings[i].m_GPUVendorPreset.m_ePresetLevel),
                  pGPUSettings[i].m_GPUVendorPreset.m_szGPUName, pGPUSettings[i].m_GPUVendorPreset.m_szGPUDriverVersion);

        if (uGPUIndex == UINT32_MAX || _vkDeviceBetterFunc(i, uGPUIndex, pGPUSettings, pGPUProperties, pGPUMemoryProperties))
        {
            u32                      count      = pQueueFamilyPropertyCount[i];
            VkQueueFamilyProperties* properties = ppQueueFamilyProperties[i];

            for (uint32_t j = 0; j < count; j++)
            {
                if (properties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    uGPUIndex = i;
                    break;
                }
            }
        }
    }

    if (VK_PHYSICAL_DEVICE_TYPE_CPU == pGPUProperties[uGPUIndex].properties.deviceType)
    {
        JLOG_CRITICAL("The only available GPU is of type VK_PHYSICAL_DEVICE_TYPE_CPU. Early exiting");
        JASSERT(false);
        return false;
    }

    JASSERT(uGPUIndex != UINT32_MAX);
    pRenderer->Vulkan.m_hVkActiveGPU                  = pGPUs[uGPUIndex];
    pRenderer->Vulkan.m_pVkActiveGPUProperties        = (VkPhysicalDeviceProperties2*)JMALLOC(sizeof(VkPhysicalDeviceProperties2));
    pRenderer->m_pActiveGPUSettings                   = (GPUSettings*)JMALLOC(sizeof(GPUSettings));
    *pRenderer->Vulkan.m_pVkActiveGPUProperties       = pGPUProperties[uGPUIndex];
    pRenderer->Vulkan.m_pVkActiveGPUProperties->pNext = nullptr;
    *pRenderer->m_pActiveGPUSettings                  = pGPUSettings[uGPUIndex];
    JASSERT(VK_NULL_HANDLE != pRenderer->Vulkan.m_hVkActiveGPU);

    JLOG_INFO("GPU[{}] is selected as default GPU", uGPUIndex);
    JLOG_INFO("Name of selected gpu: {}", pRenderer->m_pActiveGPUSettings->m_GPUVendorPreset.m_szGPUName);
    JLOG_INFO("Vendor id of selected gpu: {}", pRenderer->m_pActiveGPUSettings->m_GPUVendorPreset.m_szVendorId);
    JLOG_INFO("Model id of selected gpu: {}", pRenderer->m_pActiveGPUSettings->m_GPUVendorPreset.m_szModelId);
    JLOG_INFO("Preset of selected gpu: {}", EGPUPresetLevelToString(pRenderer->m_pActiveGPUSettings->m_GPUVendorPreset.m_ePresetLevel));

    for (uint32_t i = 0; i < uGPUCount; ++i)
    {
        JFREE(ppQueueFamilyProperties[i]);
    }
    return true;
}

static void _vkCreateInstance(const RendererContextDesc* pDesc, RendererContext* pContext)
{
    JCHECK_RHI_RESULT(volkInitialize());

    // 初始化支持的layer
    vkEnumerateInstanceLayerProperties(&pContext->Vulkan.m_uSupportLayersCount, nullptr);
    pContext->Vulkan.m_pSupportLayers   = (VkLayerProperties*)JMALLOC(sizeof(VkLayerProperties) * pContext->Vulkan.m_uSupportLayersCount);
    pContext->Vulkan.m_uUsedLayersCount = 0;
    pContext->Vulkan.m_pszUsedLayers    = (const char**)JMALLOC(sizeof(char*) * pContext->Vulkan.m_uSupportLayersCount);
    vkEnumerateInstanceLayerProperties(&pContext->Vulkan.m_uSupportLayersCount, pContext->Vulkan.m_pSupportLayers);
    for (u32 i = 0; i < pContext->Vulkan.m_uSupportLayersCount; ++i)
    {
        JLOG_INFO("VkLayerProperties {}: {}", i, pContext->Vulkan.m_pSupportLayers[i].layerName);
    }
    for (u32 i = 0; i < pDesc->Vulkan.m_uNeedLayersCount; ++i)
    {
        _vkCheckNAddValidationLayer(pDesc->Vulkan.m_ppNeedLayers[i], pContext->Vulkan.m_uSupportLayersCount, pContext->Vulkan.m_pSupportLayers,
                                    &pContext->Vulkan.m_uUsedLayersCount, pContext->Vulkan.m_pszUsedLayers, true);
    }
    if (pDesc->m_bDebug)
    {
        _vkCheckNAddValidationLayer("VK_LAYER_KHRONOS_validation", pContext->Vulkan.m_uSupportLayersCount, pContext->Vulkan.m_pSupportLayers, &pContext->Vulkan.m_uUsedLayersCount,
                                    pContext->Vulkan.m_pszUsedLayers, true);
    }

    // 初始化支持的extensions
    vkEnumerateInstanceExtensionProperties(nullptr, &pContext->Vulkan.m_uSupportExtensionsCount, nullptr);
    pContext->Vulkan.m_pSupportExtensions   = (VkExtensionProperties*)JMALLOC(sizeof(VkExtensionProperties) * pContext->Vulkan.m_uSupportExtensionsCount);
    pContext->Vulkan.m_uUsedExtensionsCount = 0;
    pContext->Vulkan.m_pszUsedExtensions    = (const char**)JMALLOC(sizeof(char*) * pContext->Vulkan.m_uSupportExtensionsCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &pContext->Vulkan.m_uSupportExtensionsCount, pContext->Vulkan.m_pSupportExtensions);
    for (u32 i = 0; i < pContext->Vulkan.m_uSupportExtensionsCount; ++i)
    {
        JLOG_INFO("VkExtensionProperties {}: {}", i, pContext->Vulkan.m_pSupportExtensions[i].extensionName);
    }
    for (u32 i = 0; i < pDesc->Vulkan.m_uNeedExtensionsCount; ++i)
    {
        _vkCheckNAddExtension(pDesc->Vulkan.m_ppNeedExtensions[i], pContext->Vulkan.m_uSupportExtensionsCount, pContext->Vulkan.m_pSupportExtensions,
                              &pContext->Vulkan.m_uUsedExtensionsCount, pContext->Vulkan.m_pszUsedExtensions, true);
    }
    if (pDesc->m_bDebug)
    {
        _vkCheckNAddExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, pContext->Vulkan.m_uSupportExtensionsCount, pContext->Vulkan.m_pSupportExtensions,
                              &pContext->Vulkan.m_uUsedExtensionsCount, pContext->Vulkan.m_pszUsedExtensions);
    }

    _vkCheckVersion(VK_VERSION_1_1);

    // feature开关，是否允许校验层检查gpu错误
    VkValidationFeaturesEXT validationFeaturesExt{};
    validationFeaturesExt.sType                              = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validationFeaturesExt.pNext                              = nullptr;
    validationFeaturesExt.enabledValidationFeatureCount      = 0;
    validationFeaturesExt.pEnabledValidationFeatures         = nullptr;
    validationFeaturesExt.disabledValidationFeatureCount     = 0;
    validationFeaturesExt.pDisabledValidationFeatures        = nullptr;
    VkValidationFeatureEnableEXT enabledValidationFeatures[] = {
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
    };
    if (pDesc->m_bGPUDebug)
    {
        validationFeaturesExt.enabledValidationFeatureCount = 1;
        validationFeaturesExt.pEnabledValidationFeatures    = enabledValidationFeatures;
    }

    VkApplicationInfo appInfo{};
    appInfo.sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext            = nullptr;
    appInfo.pApplicationName = pDesc->m_szAppName;
    appInfo.pEngineName      = "joker";
    appInfo.engineVersion    = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion       = VK_API_VERSION_1_1;

    VkInstance           hVkInstance;
    VkInstanceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext                   = &validationFeaturesExt;
    createInfo.flags                   = 0;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledLayerCount       = pContext->Vulkan.m_uUsedLayersCount;
    createInfo.ppEnabledLayerNames     = pContext->Vulkan.m_pszUsedLayers;
    createInfo.enabledExtensionCount   = pContext->Vulkan.m_uUsedExtensionsCount;
    createInfo.ppEnabledExtensionNames = pContext->Vulkan.m_pszUsedExtensions;
    JCHECK_RHI_RESULT(vkCreateInstance(&createInfo, pContext->Vulkan.m_pVkAllocationCallbacks, &hVkInstance));

    // 必须加载instance的接口
    volkLoadInstanceOnly(hVkInstance);

    if (pDesc->m_bDebug && _vkCheckExtension(&pContext->Vulkan, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
    {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
        debugInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.pNext           = nullptr;
        debugInfo.pUserData       = nullptr;
        debugInfo.pfnUserCallback = _vkDebugUtilsMessengerCallback;
        debugInfo.flags           = 0;
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        JCHECK_RHI_RESULT(vkCreateDebugUtilsMessengerEXT(hVkInstance, &debugInfo, pContext->Vulkan.m_pVkAllocationCallbacks, &pContext->Vulkan.m_hVkDebugMessenger));
    }

    pContext->Vulkan.m_hVkInstance = hVkInstance;
}

static void _vkInitGPUInfos(RendererContext* pContext)
{
    u32 uGPUCount = 0;
    JCHECK_RHI_RESULT(vkEnumeratePhysicalDevices(pContext->Vulkan.m_hVkInstance, &uGPUCount, nullptr));
    //都在栈上分配就行了，不需要回收
    VkPhysicalDevice*                 pGPUs                = (VkPhysicalDevice*)JALLOCA(uGPUCount * sizeof(VkPhysicalDevice));
    VkPhysicalDeviceProperties2*      pGPUProperties       = (VkPhysicalDeviceProperties2*)JALLOCA(uGPUCount * sizeof(VkPhysicalDeviceProperties2));
    VkPhysicalDeviceMemoryProperties* pGPUMemoryProperties = (VkPhysicalDeviceMemoryProperties*)JALLOCA(uGPUCount * sizeof(VkPhysicalDeviceMemoryProperties));
    VkPhysicalDeviceFeatures2KHR*     pGPUFeatures         = (VkPhysicalDeviceFeatures2KHR*)JALLOCA(uGPUCount * sizeof(VkPhysicalDeviceFeatures2KHR));
    JCHECK_RHI_RESULT(vkEnumeratePhysicalDevices(pContext->Vulkan.m_hVkInstance, &uGPUCount, pGPUs));
    GPUSettings* pGPUSettings  = (GPUSettings*)JALLOCA(uGPUCount * sizeof(GPUSettings));
    bool*        pGPUValid     = (bool*)JALLOCA(uGPUCount * sizeof(bool));

    u32          uRealGPUCount = 0;
    for (u32 i = 0; i < uGPUCount; i++)
    {
        u32                      uQueueFamilyPropertyCount = 0;
        VkQueueFamilyProperties* queueFamilyProperties     = NULL;
        _vkQueryGPUProperties(pGPUs[i], &pGPUProperties[i], &pGPUMemoryProperties[i], &pGPUFeatures[i], &queueFamilyProperties, &uQueueFamilyPropertyCount, &pGPUSettings[i]);

        // Filter GPUs that don't meet requirements
        bool supportGraphics = false;
        for (uint32_t j = 0; j < uQueueFamilyPropertyCount; ++j)
        {
            if (queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                supportGraphics = true;
                break;
            }
        }
        pGPUValid[i] = supportGraphics && (VK_PHYSICAL_DEVICE_TYPE_CPU != pGPUProperties[i].properties.deviceType);
        if (pGPUValid[i])
        {
            ++uRealGPUCount;
        }

        JFREE(queueFamilyProperties);
    }

    pContext->m_uGPUCount = uRealGPUCount;
    pContext->m_pGPUs     = (GPUInfo*)JCALLOC(uRealGPUCount, sizeof(GPUInfo));

    for (uint32_t i = 0, realGpu = 0; i < uRealGPUCount; ++i)
    {
        if (!pGPUValid[i])
        {
            continue;
        }

        pContext->m_pGPUs[realGpu].m_Settings                   = pGPUSettings[i];
        pContext->m_pGPUs[realGpu].Vulkan.m_hVkGPU              = pGPUs[i];
        pContext->m_pGPUs[realGpu].Vulkan.m_GPUProperties       = pGPUProperties[i];
        pContext->m_pGPUs[realGpu].Vulkan.m_GPUProperties.pNext = NULL;

        JLOG_INFO("GPU[{}] detected. Vendor ID:{}, Model ID: {}, Preset: {}, GPU Name: {}, driver version: {}", realGpu, pGPUSettings[i].m_GPUVendorPreset.m_szVendorId,
                  pGPUSettings[i].m_GPUVendorPreset.m_szModelId, EGPUPresetLevelToString(pGPUSettings[i].m_GPUVendorPreset.m_ePresetLevel),
                  pGPUSettings[i].m_GPUVendorPreset.m_szGPUName, pGPUSettings[i].m_GPUVendorPreset.m_szGPUDriverVersion);
        ++realGpu;
    }
}

void _vkCreateDevice(const RendererDesc* pDesc, Renderer* pRenderer)
{
    JASSERT(pRenderer->m_pContext->Vulkan.m_hVkInstance);

    VkDeviceGroupDeviceCreateInfoKHR deviceGroupInfo;
    if (pRenderer->m_eGPUMode == EGPUMode::Linked && _vkCheckExtension(&pRenderer->m_pContext->Vulkan, VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME))
    {
        deviceGroupInfo.sType         = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO_KHR;
        deviceGroupInfo.pNext         = nullptr;
        pRenderer->m_uLinkedNodeCount = 1;
        u32 uDeviceGroupCount         = 0;
        vkEnumeratePhysicalDeviceGroupsKHR(pRenderer->m_pContext->Vulkan.m_hVkInstance, &uDeviceGroupCount, NULL);
        VkPhysicalDeviceGroupPropertiesKHR* props = (VkPhysicalDeviceGroupPropertiesKHR*)JALLOCA(uDeviceGroupCount * sizeof(VkPhysicalDeviceGroupPropertiesKHR));
        for (uint32_t i = 0; i < uDeviceGroupCount; ++i)
        {
            props[i].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES_KHR;
            props[i].pNext = nullptr;
        }
        JCHECK_RHI_RESULT(vkEnumeratePhysicalDeviceGroupsKHR(pRenderer->m_pContext->Vulkan.m_hVkInstance, &uDeviceGroupCount, props));
        for (uint32_t i = 0; i < uDeviceGroupCount; ++i)
        {
            if (props[i].physicalDeviceCount > 1)
            {
                deviceGroupInfo.physicalDeviceCount = props[i].physicalDeviceCount;
                deviceGroupInfo.pPhysicalDevices    = props[i].physicalDevices;
                pRenderer->m_uLinkedNodeCount       = deviceGroupInfo.physicalDeviceCount;
                break;
            }
        }
    }

    if (pRenderer->m_uLinkedNodeCount < 2 && pRenderer->m_eGPUMode == EGPUMode::Unlinked)
    {
        pRenderer->m_eGPUMode = EGPUMode::Single;
    }

    if (!pDesc->m_pRenderContext)
    {
        _vkSelectBestGPU(pRenderer);
    }
    else
    {
        JASSERT(pDesc->m_uGPUIndex < pDesc->m_pRenderContext->m_uGPUCount);

        pRenderer->Vulkan.m_hVkActiveGPU                  = pDesc->m_pRenderContext->m_pGPUs[pDesc->m_uGPUIndex].Vulkan.m_hVkGPU;
        pRenderer->Vulkan.m_pVkActiveGPUProperties        = (VkPhysicalDeviceProperties2*)JMALLOC(sizeof(VkPhysicalDeviceProperties2));
        pRenderer->m_pActiveGPUSettings                   = (GPUSettings*)JMALLOC(sizeof(GPUSettings));
        *pRenderer->Vulkan.m_pVkActiveGPUProperties       = pDesc->m_pRenderContext->m_pGPUs[pDesc->m_uGPUIndex].Vulkan.m_GPUProperties;
        pRenderer->Vulkan.m_pVkActiveGPUProperties->pNext = NULL;
        *pRenderer->m_pActiveGPUSettings                  = pDesc->m_pRenderContext->m_pGPUs[pDesc->m_uGPUIndex].m_Settings;
    }

    u32 uLayerCount = 0;
    u32 uExtCount   = 0;
    vkEnumerateDeviceLayerProperties(pRenderer->Vulkan.m_hVkActiveGPU, &uLayerCount, nullptr);
    vkEnumerateDeviceExtensionProperties(pRenderer->Vulkan.m_hVkActiveGPU, nullptr, &uExtCount, nullptr);
    VkLayerProperties*     pLayers = (VkLayerProperties*)alloca(sizeof(VkLayerProperties) * uLayerCount);
    VkExtensionProperties* pExts   = (VkExtensionProperties*)alloca(sizeof(VkExtensionProperties) * uExtCount);
    vkEnumerateDeviceLayerProperties(pRenderer->Vulkan.m_hVkActiveGPU, &uLayerCount, pLayers);
    vkEnumerateDeviceExtensionProperties(pRenderer->Vulkan.m_hVkActiveGPU, nullptr, &uExtCount, pExts);
    //给used分配一个最大支持数，反正都是指针数组，没多大
    pRenderer->Vulkan.m_uDeviceUsedExtensionsCount = 0;
    pRenderer->Vulkan.m_pszDeviceUsedExtensions    = (const char**)JMALLOC(sizeof(const char*) * uExtCount);
    for (u32 i = 0; i < uLayerCount; ++i)
    {
        JLOG_INFO("Device VkLayerProperties {}: {}", i, pLayers[i].layerName);
    }
    for (u32 i = 0; i < uExtCount; ++i)
    {
        JLOG_INFO("Device VkExtensionProperties {}: {}", i, pExts[i].extensionName);
    }

    bool bDedicatedAllocationExtension     = false;
    bool bMemoryReq2Extension              = false;
    bool bFragmentShaderInterlockExtension = false;
    bool bExternalMemoryExtension          = false;
    bool bExternalMemoryWin32Extension     = false;

    {
        // const char*  szLayerName                = nullptr;
        // u32          uInitialCount              = JARRAY_SIZE(kDeviceExtensions);
        // u32          uUserRequestedCount        = (u32)pDesc->Vulkan.m_uDeviceExtensionCount;
        // u32          uNeedDeviceExtensionsCount = 0;
        // const char** pszNeedDeviceExtensions    = (const char**)JALLOCA(sizeof(const char*) * uExtCount);
        // u32          uCount                     = 0;
        // vkEnumerateDeviceExtensionProperties(pRenderer->Vulkan.m_hVkActiveGPU, szLayerName, &uCount, nullptr);
        for (u32 i = 0; i < pDesc->Vulkan.m_uDeviceNeedExtensionCount; ++i)
        {
            _vkCheckNAddExtension(pDesc->Vulkan.m_pszDeviceNeedExtensions[i], uExtCount, pExts, pRenderer);
        }
        pRenderer->Vulkan.m_bDedicatedAllocationExtension           = _vkCheckNAddExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bGetMemoryRequirement2Extension         = _vkCheckNAddExtension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bExternalMemoryExtension                = _vkCheckNAddExtension(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bExternalMemoryWin32Extension           = _vkCheckNAddExtension(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bDrawIndirectCountExtension             = _vkCheckNAddExtension(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bAMDDrawIndirectCountExtension          = _vkCheckNAddExtension(VK_AMD_DRAW_INDIRECT_COUNT_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bAMDGCNShaderExtension                  = _vkCheckNAddExtension(VK_AMD_GCN_SHADER_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bDescriptorIndexingExtension            = _vkCheckNAddExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bShaderFloatControlsExtension           = _vkCheckNAddExtension(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bBufferDeviceAddressExtension           = _vkCheckNAddExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bDeferredHostOperationsExtension        = _vkCheckNAddExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bAccelerationStructureExtension         = _vkCheckNAddExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bSpirv14Extension                       = _vkCheckNAddExtension(VK_KHR_SPIRV_1_4_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bRayTracingPipelineExtension            = _vkCheckNAddExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bRayQueryExtension                      = _vkCheckNAddExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bSamplerYCbCrConversionExtension        = _vkCheckNAddExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bFragmentShaderInterlockExtension       = _vkCheckNAddExtension(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bMultiviewExtension                     = _vkCheckNAddExtension(VK_EXT_MULTI_DRAW_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bNvDeviceDiagnosticCheckpointsExtension = _vkCheckNAddExtension(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME, uExtCount, pExts, pRenderer);
        pRenderer->Vulkan.m_bNvDeviceDiagnosticsConfigExtension     = _vkCheckNAddExtension(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME, uExtCount, pExts, pRenderer);

        pRenderer->Vulkan.m_bRaytracingSupported                    = pRenderer->Vulkan.m_bShaderFloatControlsExtension && pRenderer->Vulkan.m_bBufferDeviceAddressExtension &&
                                                   pRenderer->Vulkan.m_bBufferDeviceAddressExtension && pRenderer->Vulkan.m_bDeferredHostOperationsExtension &&
                                                   pRenderer->Vulkan.m_bAccelerationStructureExtension && pRenderer->Vulkan.m_bSpirv14Extension &&
                                                   pRenderer->Vulkan.m_bRayTracingPipelineExtension;
    }

    VkPhysicalDeviceFeatures2 features2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    VkBaseOutStructure*       pBase = (VkBaseOutStructure*)&features2;

#define _JRHI_VK_ADD_FEATURE(cond, next)                                                                                                                                           \
    if ((cond))                                                                                                                                                                    \
    {                                                                                                                                                                              \
        pBase->pNext = (VkBaseOutStructure*)&next;                                                                                                                                 \
        pBase        = (VkBaseOutStructure*)pBase->pNext;                                                                                                                          \
    }
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT featureFragmentShaderInterlock{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT};
    _JRHI_VK_ADD_FEATURE(pRenderer->Vulkan.m_bFragmentShaderInterlockExtension, featureFragmentShaderInterlock);

    VkPhysicalDeviceDescriptorIndexingFeatures featureDescriptorIndexing{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
    _JRHI_VK_ADD_FEATURE(pRenderer->Vulkan.m_bDescriptorIndexingExtension, featureDescriptorIndexing);

    VkPhysicalDeviceSamplerYcbcrConversionFeatures featureSamplerYcbcr{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES};
    _JRHI_VK_ADD_FEATURE(pRenderer->Vulkan.m_bSamplerYCbCrConversionExtension, featureSamplerYcbcr);

    VkPhysicalDeviceMultiviewFeatures featureMultiview{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES};
    _JRHI_VK_ADD_FEATURE(pRenderer->Vulkan.m_bMultiviewExtension, featureMultiview);

    VkPhysicalDeviceBufferDeviceAddressFeatures featureBufferDeviceAddress{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES};
    featureBufferDeviceAddress.bufferDeviceAddress = VK_TRUE;
    _JRHI_VK_ADD_FEATURE(pRenderer->Vulkan.m_bBufferDeviceAddressExtension, featureBufferDeviceAddress);

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR featureRayTracingPipeline{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
    featureRayTracingPipeline.rayTracingPipeline = VK_TRUE;
    _JRHI_VK_ADD_FEATURE(pRenderer->Vulkan.m_bRayTracingPipelineExtension, featureRayTracingPipeline);

    VkPhysicalDeviceAccelerationStructureFeaturesKHR featureAccelerationStructure{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
    featureAccelerationStructure.accelerationStructure = VK_TRUE;
    _JRHI_VK_ADD_FEATURE(pRenderer->Vulkan.m_bAccelerationStructureExtension, featureAccelerationStructure);

    VkPhysicalDeviceRayQueryFeaturesKHR featureRayQuery{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR};
    featureRayQuery.rayQuery = VK_TRUE;
    _JRHI_VK_ADD_FEATURE(pRenderer->Vulkan.m_bRayQueryExtension, featureRayQuery);

    _JRHI_VK_ADD_FEATURE(pRenderer->m_eGPUMode == EGPUMode::Linked, deviceGroupInfo);

#undef _JRHI_VK_ADD_FEATURE

    vkGetPhysicalDeviceFeatures2(pRenderer->Vulkan.m_hVkActiveGPU, &features2);

    u32 uQueueFamiliesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pRenderer->Vulkan.m_hVkActiveGPU, &uQueueFamiliesCount, NULL);
    VkQueueFamilyProperties* pQueueFamiliesProperties = (VkQueueFamilyProperties*)JALLOCA(uQueueFamiliesCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(pRenderer->Vulkan.m_hVkActiveGPU, &uQueueFamiliesCount, pQueueFamiliesProperties);

    float                    ppQueueFamilyPriorities[kQueueMaxFamilies][kQueueMaxCount] = {};
    u32                      uQueueCreateInfosCount                                     = 0;
    VkDeviceQueueCreateInfo* pQeueuCreateInfos                                          = (VkDeviceQueueCreateInfo*)JALLOCA(uQueueFamiliesCount * sizeof(VkDeviceQueueCreateInfo));

    pRenderer->Vulkan.m_pAvailableQueueCount                                            = (u32**)JMALLOC(pRenderer->m_uLinkedNodeCount * sizeof(u32*));
    pRenderer->Vulkan.m_pUsedQueueCount                                                 = (u32**)JMALLOC(pRenderer->m_uLinkedNodeCount * sizeof(u32*));
    for (u32 i = 0; i < pRenderer->m_uLinkedNodeCount; ++i)
    {
        // TODO
        pRenderer->Vulkan.m_pAvailableQueueCount[i] = (u32*)JCALLOC(kMaxQueueFlag, sizeof(u32));
        pRenderer->Vulkan.m_pUsedQueueCount[i]      = (u32*)JCALLOC(kMaxQueueFlag, sizeof(u32));
    }

    for (u32 i = 0; i < uQueueFamiliesCount; i++)
    {
        u32 uQueueCount = pQueueFamiliesProperties[i].queueCount;
        if (uQueueCount > 0)
        {
            // Request only one queue of each type if mRequestAllAvailableQueues is not set to true
            if (uQueueCount > 1 && !pDesc->Vulkan.m_bRequestAllAvailableQueues)
            {
                uQueueCount = 1;
            }

            JASSERT(uQueueCount <= kQueueMaxCount);
            uQueueCount                                                = JMIN(uQueueCount, kQueueMaxCount);
            pQeueuCreateInfos[uQueueCreateInfosCount]                  = {};
            pQeueuCreateInfos[uQueueCreateInfosCount].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            pQeueuCreateInfos[uQueueCreateInfosCount].pNext            = NULL;
            pQeueuCreateInfos[uQueueCreateInfosCount].flags            = 0;
            pQeueuCreateInfos[uQueueCreateInfosCount].queueFamilyIndex = i;
            pQeueuCreateInfos[uQueueCreateInfosCount].queueCount       = uQueueCount;
            pQeueuCreateInfos[uQueueCreateInfosCount].pQueuePriorities = ppQueueFamilyPriorities[i];
            uQueueCreateInfosCount++;

            for (uint32_t n = 0; n < pRenderer->m_uLinkedNodeCount; ++n)
            {
                pRenderer->Vulkan.m_pAvailableQueueCount[n][pQueueFamiliesProperties[i].queueFlags] = uQueueCount;
            }
        }
    }

    VkDeviceCreateInfo createInfo      = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext                   = &features2;
    createInfo.flags                   = 0;
    createInfo.queueCreateInfoCount    = uQueueCreateInfosCount;
    createInfo.pQueueCreateInfos       = pQeueuCreateInfos;
    createInfo.enabledLayerCount       = 0;
    createInfo.ppEnabledLayerNames     = nullptr;
    createInfo.enabledExtensionCount   = pRenderer->Vulkan.m_uDeviceUsedExtensionsCount;
    createInfo.ppEnabledExtensionNames = pRenderer->Vulkan.m_pszDeviceUsedExtensions;
    createInfo.pEnabledFeatures        = nullptr;

    JCHECK_RHI_RESULT(vkCreateDevice(pRenderer->Vulkan.m_hVkActiveGPU, &createInfo, kAllocator, &pRenderer->Vulkan.m_hVkDevice));

    if(pRenderer->m_eGPUMode != EGPUMode::Unlinked)
    {
        volkLoadDevice(pRenderer->Vulkan.m_hVkDevice);
    }
}

void _vkInitRendererContext(const RendererContextDesc* pDesc, RendererContext** ppContext)
{
    RendererContext* pRendererContext = (RendererContext*)JCALLOC_ALIGNED(1, alignof(RendererContext), sizeof(RendererContext));
    pRendererContext->m_eRenderer     = ERenderer::Vulkan;

    _vkCreateInstance(pDesc, pRendererContext);
    JASSERT(pRendererContext->Vulkan.m_hVkInstance);

    _vkInitGPUInfos(pRendererContext);

    *ppContext = pRendererContext;
}

void _vkExitRendererContext(RendererContext* pContext)
{
    if (pContext->Vulkan.m_hVkInstance)
    {
        if (pContext->Vulkan.m_hVkDebugMessenger)
        {
            vkDestroyDebugUtilsMessengerEXT(pContext->Vulkan.m_hVkInstance, pContext->Vulkan.m_hVkDebugMessenger, pContext->Vulkan.m_pVkAllocationCallbacks);
        }
        vkDestroyInstance(pContext->Vulkan.m_hVkInstance, pContext->Vulkan.m_pVkAllocationCallbacks);
    }
    JFREE(pContext->Vulkan.m_pSupportLayers)
    JFREE(pContext->Vulkan.m_pszUsedLayers);
    JFREE(pContext->Vulkan.m_pSupportExtensions)
    JFREE(pContext->Vulkan.m_pszUsedExtensions);
    JFREE(pContext->m_pGPUs);
    JFREE(pContext);
}

void _vkInitRenderer(const RendererDesc* pDesc, Renderer** ppRenderer)
{
    u8* pMem = (u8*)JCALLOC_ALIGNED(1, alignof(Renderer*), sizeof(Renderer) + sizeof(NullDescriptors));
    JASSERT(pMem);

    Renderer* pRenderer           = (Renderer*)pMem;
    pRenderer->m_eRenderer        = ERenderer::Vulkan;
    pRenderer->m_pContext         = pDesc->m_pRenderContext;
    pRenderer->m_eGPUMode         = pDesc->m_eGPUMode;
    pRenderer->m_eShaderMode      = pDesc->m_eShaderMode;
    pRenderer->m_pNullDescriptors = (NullDescriptors*)(pMem + sizeof(Renderer));

    size_t uNameLen               = strlen(pDesc->m_szName);
    pRenderer->m_szName           = (char*)JCALLOC(uNameLen + 1, sizeof(char));
    strcpy_s(pRenderer->m_szName, uNameLen + 1, pDesc->m_szName);

    //如果是unlinked模式，就必须有RendererContext
    JASSERT(pDesc->m_eGPUMode != EGPUMode::Unlinked || pDesc);
    if (pRenderer->m_pContext)
    {
        pRenderer->Vulkan.m_bOwnInstance   = false;
        pRenderer->Vulkan.m_uRendererIndex = pRenderer->m_pContext->m_uRendererCount;
    }
    else
    {
        RendererContextDesc desc;
        desc.m_eRenderer                   = pDesc->m_eRenderer;
        desc.m_bDebug                      = pDesc->m_bDebug;
        desc.m_bGPUDebug                   = pDesc->m_bGPUDebug;
        desc.m_szAppName                   = pDesc->m_szName;
        desc.Vulkan.m_uNeedLayersCount     = pDesc->Vulkan.m_uInstanceNeedLayerCount;
        desc.Vulkan.m_ppNeedLayers         = pDesc->Vulkan.m_pszInstanceNeedLayers;
        desc.Vulkan.m_uNeedExtensionsCount = pDesc->Vulkan.m_uInstanceNeedExtensionCount;
        desc.Vulkan.m_ppNeedExtensions     = pDesc->Vulkan.m_pszInstanceNeedExtensions;

        pRenderer->m_pContext              = (RendererContext*)JCALLOC_ALIGNED(1, alignof(RendererContext), sizeof(RendererContext));
        pRenderer->m_pContext->m_eRenderer = ERenderer::Vulkan;
        pRenderer->Vulkan.m_bOwnInstance   = true;
        _vkCreateInstance(&desc, pRenderer->m_pContext);
    }

    _vkCreateDevice(pDesc, pRenderer);

    // 内存分配
    // VmaAllocatorCreateInfo createInfo = { 0 };
	// 	createInfo.device = pRenderer->mVulkan.pVkDevice;
	// 	createInfo.physicalDevice = pRenderer->mVulkan.pVkActiveGPU;
	// 	createInfo.instance = pRenderer->mVulkan.pVkInstance;

    ++pRenderer->m_pContext->m_uRendererCount;
    *ppRenderer = pRenderer;
}

void _vkExitRenderer(Renderer* pRenderer)
{
}
}