#include "JokerRHIPCH.h"
#include "CoreType.h"
#include "RHIEnum.h"
#include "RHIFunction.h"
#include "RHIStruct.h"
#include "EASTL/vector.h"
#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#if defined(JOPTION_RHI_MULTI)
#define JIMPL_RHI_FUNC_VK(ret, name, ...) ret name##VK(__VA_ARGS__)
#define JREG_RHI_FUNC_VK(name)            name = name##VK;
#else
#define JIMPL_RHI_FUNC_VK(ret, name, ...) ret name(__VA_ARGS__)
#define JREG_RHI_FUNC_VK(name)
#endif

#define JCHECK_VKRESULT(exp)                                                                                                                         \
    {                                                                                                                                                \
        VkResult vkres = (exp);                                                                                                                      \
        if (VK_SUCCESS != vkres)                                                                                                                     \
        {                                                                                                                                            \
            JLOG_CRITICAL("{}: FAILED with VkResult: {}", #exp, vkres);                                                                              \
            JASSERT(false);                                                                                                                          \
        }                                                                                                                                            \
    }

constexpr const char* kVkWantedInstanceExtensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_GGP)
    VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_VI_NN)
    VK_NN_VI_SURFACE_EXTENSION_NAME,
#endif
// Debug utils not supported on all devices yet
#ifdef ENABLE_DEBUG_UTILS_EXTENSION
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#else
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
    VK_NV_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
    // To legally use HDR formats
    VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,
    /************************************************************************/
    // VR Extensions
    /************************************************************************/
    VK_KHR_DISPLAY_EXTENSION_NAME,
    VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME,
/************************************************************************/
// Multi GPU Extensions
/************************************************************************/
#if VK_KHR_device_group_creation
    VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME,
#endif
#ifndef NX64
    /************************************************************************/
    // Property querying extensions
    /************************************************************************/
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
/************************************************************************/
/************************************************************************/
#endif
};

constexpr const char* kVkWantedDeviceExtensions[] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_MAINTENANCE1_EXTENSION_NAME,
	VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
	VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME,
	VK_EXT_SHADER_SUBGROUP_VOTE_EXTENSION_NAME,
	VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
	VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
#ifdef USE_EXTERNAL_MEMORY_EXTENSIONS
	VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
	VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
	VK_KHR_EXTERNAL_FENCE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
	VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
	VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME,
#endif
#endif
	// Debug marker extension in case debug utils is not supported
#ifndef ENABLE_DEBUG_UTILS_EXTENSION
	VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
#endif
#if defined(VK_USE_PLATFORM_GGP)
	VK_GGP_FRAME_TOKEN_EXTENSION_NAME,
#endif

#if VK_KHR_draw_indirect_count
	VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
#endif
	// Fragment shader interlock extension to be used for ROV type functionality in Vulkan
#if VK_EXT_fragment_shader_interlock
	VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME,
#endif
	/************************************************************************/
	// NVIDIA Specific Extensions
	/************************************************************************/
#ifdef USE_NV_EXTENSIONS
	VK_NVX_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME,
#endif
	/************************************************************************/
	// AMD Specific Extensions
	/************************************************************************/
	VK_AMD_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
	VK_AMD_SHADER_BALLOT_EXTENSION_NAME,
	VK_AMD_GCN_SHADER_EXTENSION_NAME,
	/************************************************************************/
	// Multi GPU Extensions
	/************************************************************************/
#if VK_KHR_device_group
	VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
#endif
	/************************************************************************/
	// Bindless & None Uniform access Extensions
	/************************************************************************/
	VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
#if VK_KHR_maintenance3 // descriptor indexing depends on this
        VK_KHR_MAINTENANCE3_EXTENSION_NAME,
#endif
	/************************************************************************/
	// Raytracing
	/************************************************************************/
#ifdef VK_RAYTRACING_AVAILABLE
	VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
	VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
	VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, 

	VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,

	VK_KHR_RAY_QUERY_EXTENSION_NAME,
#endif
	/************************************************************************/
	// YCbCr format support
	/************************************************************************/
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
	/************************************************************************/
	// Multiview support
	/************************************************************************/
#ifdef QUEST_VR
	VK_KHR_MULTIVIEW_EXTENSION_NAME,
#endif
    /************************************************************************/
	// Nsight Aftermath
	/************************************************************************/
#ifdef ENABLE_NSIGHT_AFTERMATH
	VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME,
	VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME,
#endif
	/************************************************************************/
	/************************************************************************/
};

constexpr u32 kVendorIDNvidia = 0x10DE;
constexpr u32 kVendorIDAmd    = 0x1002;
constexpr u32 kVendorIDAmd1   = 0x1022;
constexpr u32 kVendorIDIntel  = 0x163C;
constexpr u32 kVendorIDIntel1 = 0x8086;
constexpr u32 kVendorIDIntel2 = 0x8087;

// #define VK_FORMAT_VERSION( version, outVersionString )                     \
// 	JASSERT( VK_MAX_DESCRIPTION_SIZE == ARRAYSIZE( outVersionString ) ); \
// 	sprintf( outVersionString, "%u.%u.%u", VK_VERSION_MAJOR( version ), VK_VERSION_MINOR( version ), VK_VERSION_PATCH( version ) );

static ERHIGPUVendor _RHIGetGPUVendor(u32 uVendorID)
{
    if (uVendorID == kVendorIDNvidia)
    {
        return ERHIGPUVendor::Nvidia;
    }
    else if (uVendorID == kVendorIDAmd || uVendorID == kVendorIDAmd1)
    {
        return ERHIGPUVendor::Amd;
    }
    else if (uVendorID == kVendorIDIntel || uVendorID == kVendorIDIntel1 || uVendorID == kVendorIDIntel2)
    {
        return ERHIGPUVendor::Intel;
    }
    else
    {
        return ERHIGPUVendor::Unknown;
    }
}

// TODO
static void _RHIQueryGPUSettings(VkPhysicalDevice gpu, VkPhysicalDeviceProperties2* gpuProperties,
                                 VkPhysicalDeviceMemoryProperties* gpuMemoryProperties, VkPhysicalDeviceFeatures2KHR* gpuFeatures,
                                 VkQueueFamilyProperties** queueFamilyProperties, u32* queueFamilyPropertyCount, RHIGPUSettings* gpuSettings)
{
    *gpuProperties            = {};
    *gpuMemoryProperties      = {};
    *gpuFeatures              = {};
    *queueFamilyProperties    = {};
    *queueFamilyPropertyCount = {};

    //获取内存属性
    vkGetPhysicalDeviceMemoryProperties(gpu, gpuMemoryProperties);

    //获取features
    gpuFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
#if VK_EXT_fragment_shader_interlock
    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT fragmentShaderInterlockFeatures = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT};
    gpuFeatures->pNext = &fragmentShaderInterlockFeatures;
#endif
    vkGetPhysicalDeviceFeatures2KHR(gpu, gpuFeatures);

    //获取设备属性
    VkPhysicalDeviceSubgroupProperties subgroupProperties{};
    subgroupProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
    subgroupProperties.pNext = nullptr;
    gpuProperties->sType     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
    subgroupProperties.pNext = gpuProperties->pNext;
    gpuProperties->pNext     = &subgroupProperties;
    vkGetPhysicalDeviceProperties2KHR(gpu, gpuProperties);

    //获取queue的属性
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, queueFamilyPropertyCount, nullptr);
    *queueFamilyProperties = (VkQueueFamilyProperties*)JCALLOC(*queueFamilyPropertyCount, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, queueFamilyPropertyCount, *queueFamilyProperties);

    *gpuSettings                                    = {};
    gpuSettings->m_uUniformBufferAlignment          = (u32)gpuProperties->properties.limits.minUniformBufferOffsetAlignment;
    gpuSettings->m_uUploadBufferTextureAlignment    = (u32)gpuProperties->properties.limits.optimalBufferCopyOffsetAlignment;
    gpuSettings->m_uUploadBufferTextureRowAlignment = (u32)gpuProperties->properties.limits.optimalBufferCopyRowPitchAlignment;
    gpuSettings->m_uMaxVertexInputBindings          = gpuProperties->properties.limits.maxVertexInputBindings;
    gpuSettings->m_uMultiDrawIndirect               = gpuFeatures->features.multiDrawIndirect;

    gpuSettings->m_uWaveLaneCount                   = subgroupProperties.subgroupSize;
    gpuSettings->m_eWaveOpsSupportFlags             = ERHIWaveOpsSupportFlags::None;
    if (subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_BASIC_BIT)
    {
        gpuSettings->m_eWaveOpsSupportFlags |= ERHIWaveOpsSupportFlags::BasicBit;
    }
    if (subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_VOTE_BIT)
    {
        gpuSettings->m_eWaveOpsSupportFlags |= ERHIWaveOpsSupportFlags::VoteBit;
    }
    if (subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT)
    {
        gpuSettings->m_eWaveOpsSupportFlags |= ERHIWaveOpsSupportFlags::ArithmeticBit;
    }
    if (subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_BALLOT_BIT)
    {
        gpuSettings->m_eWaveOpsSupportFlags |= ERHIWaveOpsSupportFlags::BallotBit;
    }
    if (subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_BIT)
    {
        gpuSettings->m_eWaveOpsSupportFlags |= ERHIWaveOpsSupportFlags::ShuffleBit;
    }
    if (subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT)
    {
        gpuSettings->m_eWaveOpsSupportFlags |= ERHIWaveOpsSupportFlags::ShuffleRelativeBit;
    }
    if (subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_CLUSTERED_BIT)
    {
        gpuSettings->m_eWaveOpsSupportFlags |= ERHIWaveOpsSupportFlags::ClusteredBit;
    }
    if (subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_QUAD_BIT)
    {
        gpuSettings->m_eWaveOpsSupportFlags |= ERHIWaveOpsSupportFlags::QuadBit;
    }
    if (subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV)
    {
        gpuSettings->m_eWaveOpsSupportFlags |= ERHIWaveOpsSupportFlags::PartitionedBitNV;
    }

#if VK_EXT_fragment_shader_interlock
    gpuSettings->m_uROVsSupported = (bool)fragmentShaderInterlockFeatures.fragmentShaderPixelInterlock;
#endif
    gpuSettings->m_uTessellationSupported   = gpuFeatures->features.tessellationShader;
    gpuSettings->m_uGeometryShaderSupported = gpuFeatures->features.geometryShader;

    sprintf(gpuSettings->m_GPUVendorPreset.m_szModelID, "%#x", gpuProperties->properties.deviceID);
    sprintf(gpuSettings->m_GPUVendorPreset.m_szVenderID, "%#x", gpuProperties->properties.vendorID);
    strcpy_s(gpuSettings->m_GPUVendorPreset.m_szGPUName, VK_MAX_DESCRIPTION_SIZE, gpuProperties->properties.deviceName);
    strcpy_s(gpuSettings->m_GPUVendorPreset.m_szRevisionID, VK_MAX_DESCRIPTION_SIZE, "0x00");
    // TODO
    gpuSettings->m_GPUVendorPreset.m_ePresetLevel = ERHIGPUPresetLevel::None;

    u32 uMajor, uMinor, uSecondaryBranch, uTertiaryBranch;
    switch (_RHIGetGPUVendor(gpuProperties->properties.vendorID))
    {
    case ERHIGPUVendor::Nvidia:
        uMajor           = (gpuProperties->properties.driverVersion >> 22) & 0x3ff;
        uMinor           = (gpuProperties->properties.driverVersion >> 14) & 0x0ff;
        uSecondaryBranch = (gpuProperties->properties.driverVersion >> 6) & 0x0ff;
        uTertiaryBranch  = (gpuProperties->properties.driverVersion) & 0x003f;

        sprintf(gpuSettings->m_GPUVendorPreset.m_szGPUDriverVersion, "%u.%u.%u.%u", uMajor, uMinor, uSecondaryBranch, uTertiaryBranch);
        break;
    default:
        JASSERT(VK_MAX_DESCRIPTION_SIZE == JARRAY_SIZE(gpuSettings->m_GPUVendorPreset.m_szGPUDriverVersion));
        sprintf(gpuSettings->m_GPUVendorPreset.m_szGPUDriverVersion, "%u.%u.%u", VK_VERSION_MAJOR(gpuProperties->properties.driverVersion),
                VK_VERSION_MINOR(gpuProperties->properties.driverVersion), VK_VERSION_PATCH(gpuProperties->properties.driverVersion));
    }

    gpuFeatures->pNext   = nullptr;
    gpuProperties->pNext = nullptr;
}

JIMPL_RHI_FUNC_VK(void, RHIAddFence, RHIRenderer* pRenderer, RHIFence** ppFence)
{
}

JIMPL_RHI_FUNC_VK(void, RHIRemoveFence, RHIRenderer* pRenderer, RHIFence* pFence)
{
}

static void _RHICreateInstance(const char* szName, const RHIRendererDesc* pDesc, uint32_t uInstanceLayerCount, const char** ppInstanceLayers,
                               RHIRenderer* pRenderer)
{
    const char* ppInstanceExtensionCache[64] = {};

    u32         uLayerCount                  = 0;
    u32         uExtCount                    = 0;
    vkEnumerateInstanceLayerProperties(&uLayerCount, nullptr);
    vkEnumerateInstanceExtensionProperties(nullptr, &uExtCount, nullptr);

    VkLayerProperties* pLayers = (VkLayerProperties*)JMALLOC(sizeof(VkLayerProperties) * uLayerCount);
    vkEnumerateInstanceLayerProperties(&uLayerCount, pLayers);

    VkExtensionProperties* pExts = (VkExtensionProperties*)JMALLOC(sizeof(VkExtensionProperties*) * uExtCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &uExtCount, pExts);

    for (u32 i = 0; i < uLayerCount; ++i)
    {
        JLOG_INFO("{} ( {} )", pLayers[i].layerName, "vkInstance-layer");
    }

    for (u32 i = 0; i < uExtCount; ++i)
    {
        JLOG_INFO("{} ( {} )", pExts[i].extensionName, "vkInstance-ext");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType                        = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext                        = nullptr;
    appInfo.pApplicationName             = szName;
    appInfo.applicationVersion           = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName                  = "joker";
    appInfo.engineVersion                = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion                   = VK_API_VERSION_1_1;

    eastl::vector<const char*> layerTemp = eastl::vector<const char*>(uInstanceLayerCount);
    memcpy(layerTemp.data(), ppInstanceLayers, layerTemp.size() * sizeof(char*));

    {
        for (u32 i = 0; i < (u32)layerTemp.size(); ++i)
        {
            bool bLayerFound = false;
            for (u32 j = 0; j < uLayerCount; ++j)
            {
                if (strcmp(ppInstanceLayers[i], pLayers[j].layerName) == 0)
                {
                    bLayerFound = true;
                    break;
                }
            }
            if (!bLayerFound)
            {
                JLOG_WARN("{} ( {} )", ppInstanceLayers[i], "vkInstance-layer-missing");
                i = (u32)(layerTemp.erase(layerTemp.begin() + i) - layerTemp.begin());
            }
        }

        u32                        uExtensionCount     = 0;
        const u32                  uInitialCount       = sizeof(kVkWantedInstanceExtensions) / sizeof(kVkWantedInstanceExtensions[0]);
        const u32                  uUserRequestedCount = (u32)pDesc->Vulkan.m_uInstanceExtensionCount;
        eastl::vector<const char*> wantedInstanceExtensions(uInitialCount + uUserRequestedCount);
        for (u32 i = 0; i < uInitialCount; ++i)
        {
            wantedInstanceExtensions[i] = kVkWantedInstanceExtensions[i];
        }
        for (u32 i = 0; i < uUserRequestedCount; ++i)
        {
            wantedInstanceExtensions[uInitialCount + i] = pDesc->Vulkan.m_ppInstanceExtensions[i];
        }
        const u32 uWantedExtensionCount         = (u32)wantedInstanceExtensions.size();
        bool      bDeviceGroupCreationExtension = false;
        bool      bDebugUtilsExtension          = false;
        for (size_t i = 0; i < layerTemp.size(); ++i)
        {
            const char* szLayerName = layerTemp[i];
            u32         uCount      = 0;
            vkEnumerateInstanceExtensionProperties(szLayerName, &uCount, nullptr);
            JASSERT(uCount != 0);
            VkExtensionProperties* pProperties = (VkExtensionProperties*)JCALLOC(uCount, sizeof(VkExtensionProperties));
            vkEnumerateInstanceExtensionProperties(szLayerName, &uCount, pProperties);
            for (u32 j = 0; j < uCount; ++j)
            {
                for (u32 k = 0; k < uWantedExtensionCount; ++k)
                {
                    if (strcmp(wantedInstanceExtensions[k], pProperties[j].extensionName) == 0)
                    {
                        if (strcmp(wantedInstanceExtensions[k], VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME) == 0)
                        {
                            bDeviceGroupCreationExtension = true;
                        }
                        if (strcmp(wantedInstanceExtensions[i], VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
                        {
                            bDebugUtilsExtension = true;
                        }
                        ppInstanceExtensionCache[uExtensionCount++] = wantedInstanceExtensions[k];
                        // 清理掉之后放置加载多次
                        wantedInstanceExtensions[k] = "";
                        break;
                    }
                }
            }
            JFREE(pProperties);
        }

        // 独立的扩展
        {
            const char* szLayerName = nullptr;
            u32         uCount      = 0;
            vkEnumerateInstanceExtensionProperties(szLayerName, &uCount, nullptr);
            if (uCount > 0)
            {
                VkExtensionProperties* pProperties = (VkExtensionProperties*)JCALLOC(uCount, sizeof(VkExtensionProperties));
                JASSERT(pProperties != nullptr);
                vkEnumerateInstanceExtensionProperties(szLayerName, &uCount, pProperties);
                for (u32 j = 0; j < uCount; ++j)
                {
                    for (u32 k = 0; k < uWantedExtensionCount; ++k)
                    {
                        if (strcmp(wantedInstanceExtensions[k], pProperties[j].extensionName) == 0)
                        {
                            if (strcmp(wantedInstanceExtensions[k], VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME) == 0)
                            {
                                bDeviceGroupCreationExtension = true;
                            }
                            if (strcmp(wantedInstanceExtensions[k], VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
                            {
                                bDebugUtilsExtension = true;
                            }
                            ppInstanceExtensionCache[uExtensionCount++] = wantedInstanceExtensions[k];
                            // 清理掉之后放置加载多次
                            wantedInstanceExtensions[k] = "";
                            break;
                        }
                    }
                }
            }
        }
        VkInstanceCreateInfo createInfo{};
        createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.flags                   = 0;
        createInfo.pApplicationInfo        = &appInfo;
        createInfo.ppEnabledLayerNames     = layerTemp.data();
        createInfo.enabledExtensionCount   = uExtensionCount;
        createInfo.ppEnabledExtensionNames = ppInstanceExtensionCache;
#if VK_HEADER_VERSION >= 108
        VkValidationFeaturesEXT      validationFreaturesExt     = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT};
        VkValidationFeatureEnableEXT enableValidationFeatures[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
        if (pDesc->m_bEnableGPUBasedValidation)
        {
            validationFreaturesExt.enabledValidationFeatureCount = 1;
            validationFreaturesExt.pEnabledValidationFeatures    = enableValidationFeatures;
        }
#endif
        JCHECK_VKRESULT(vkCreateInstance(&createInfo, nullptr, &(pRenderer->Vulkan.m_hVkInstance)));
    }
}

static void _RHIRemoveInstanceVK(RHIRenderer* pRenderer)
{
    JASSERT(VK_NULL_HANDLE != pRenderer->Vulkan.m_hVkInstance);
    if (pRenderer->Vulkan.m_hVkDebugUtilsMessenger)
    {
        // TODO allocator
        vkDestroyDebugUtilsMessengerEXT(pRenderer->Vulkan.m_hVkInstance, pRenderer->Vulkan.m_hVkDebugUtilsMessenger, nullptr);
        pRenderer->Vulkan.m_hVkDebugUtilsMessenger = nullptr;
    }
    // TODO allocator
    vkDestroyInstance(pRenderer->Vulkan.m_hVkInstance, nullptr);
}

static bool _RHIAddDeviceVK(const RHIRendererDesc* pDesc, RHIRenderer* pRenderer)
{
    JASSERT(VK_NULL_HANDLE != pRenderer->Vulkan.m_hVkInstance);

    const char* deviceExtensionCache[RHIConst::kMaxDeviceExtensions] = {};

#if VK_KHR_device_group_creation
    // 如果支持一组硬件使用同一逻辑接口,感觉应该用检查，不应该用宏判断
    // TODO 需要判断是否支持group创建
    
    if(pRenderer->m_eGPUMode==ERHIGPUMode::Linked&&pDesc->m_pRenderContext!=nullptr&&pDesc->m_pRenderContext->Vulkan.m_bExtDeviceGroupCreation)
    {
        VkDeviceGroupDeviceCreateInfo   deviceGroupInfo{VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO};
        VkPhysicalDeviceGroupProperties props[RHIConst::kMaxLinkedGPUs]{};

        pRenderer->m_uLinkedNodeCount = 1;
    }
#endif
    return false;
}

static bool _RHIInitCommonVK(const char* szAppName, const RHIRendererDesc* pDesc, RHIRenderer* pRenderer)
{
    const char** ppInstanceLayers    = (const char**)JMALLOC(2 + pDesc->Vulkan.m_uInstanceLayerCount * sizeof(char*));
    u32          uInstanceLayerCount = 0;
    // 添加默认的调试层
    ppInstanceLayers[uInstanceLayerCount++] = "VK_LAYER_KHRONOS_validation";
    // 添加自定义的调试层
    for (u32 i = 0; i < (u32)pDesc->Vulkan.m_uInstanceLayerCount; i++)
    {
        ppInstanceLayers[uInstanceLayerCount++] = pDesc->Vulkan.m_ppInstanceLayers[i];
    }
    // 初始化volk
    VkResult ret = volkInitialize();
    if (ret != VK_SUCCESS)
    {
        JLOG_CRITICAL("Failed to initialize Vulkan");
        return false;
    }

    _RHICreateInstance(szAppName, pDesc, uInstanceLayerCount, ppInstanceLayers, pRenderer);

    pRenderer->m_uUnlinkedRenderIndex = 0;
    pRenderer->Vulkan.m_bOwnInstance  = true;
    return true;
}

static void _RHIExitCommonVK(RHIRenderer* pRenderer)
{
    _RHIRemoveInstanceVK(pRenderer);
}

void RHIInitRendererContextVK(const char* szAppName, const RHIRendererContextDesc* pSettings, RHIRendererContext** ppContext)
{
    JASSERT(szAppName);
    JASSERT(pSettings);
    JASSERT(ppContext);
    // JASSERT(gRendererCount);

    RHIRendererDesc fakeDesc{};
    fakeDesc.Vulkan.m_uInstanceLayerCount     = pSettings->Vulkan.m_uInstanceLayerCount;
    fakeDesc.Vulkan.m_uInstanceExtensionCount = pSettings->Vulkan.m_uInstanceExtensionCount;
    fakeDesc.Vulkan.m_ppInstanceExtensions    = pSettings->Vulkan.m_ppInstanceExtensions;
    fakeDesc.Vulkan.m_ppInstanceLayers        = pSettings->Vulkan.m_ppInstanceLayers;
    fakeDesc.m_bEnableGPUBasedValidation      = pSettings->m_bEnableGPUBasedValidation;

    RHIRenderer fakeRenderer{};
    if (!_RHIInitCommonVK(szAppName, &fakeDesc, &fakeRenderer))
    {
        return;
    }
    RHIRendererContext* pContext             = (RHIRendererContext*)JMALLOC_ALIGNED(1, alignof(RHIRendererContext), sizeof(RHIRendererContext));
    pContext->Vulkan.m_hVkInstance           = fakeRenderer.Vulkan.m_hVkInstance;
    pContext->Vulkan.m_hVkDebugUtilsMessager = fakeRenderer.Vulkan.m_hVkDebugUtilsMessenger;

    u32 uGPUCount                            = 0;
    JCHECK_VKRESULT(vkEnumeratePhysicalDevices(pContext->Vulkan.m_hVkInstance, &uGPUCount, NULL));

    VkPhysicalDevice*                 pGPUs          = (VkPhysicalDevice*)JMALLOC(uGPUCount * sizeof(VkPhysicalDevice));
    VkPhysicalDeviceFeatures2*        pGPUFeatures   = (VkPhysicalDeviceFeatures2*)JMALLOC(uGPUCount * sizeof(VkPhysicalDeviceFeatures2));
    VkPhysicalDeviceProperties2*      pGPUProperties = (VkPhysicalDeviceProperties2*)JMALLOC(uGPUCount * sizeof(VkPhysicalDeviceProperties2));
    VkPhysicalDeviceMemoryProperties* pGPUMemoryProperties =
        (VkPhysicalDeviceMemoryProperties*)JMALLOC(uGPUCount * sizeof(VkPhysicalDeviceMemoryProperties));

    JCHECK_VKRESULT(vkEnumeratePhysicalDevices(pContext->Vulkan.m_hVkInstance, &uGPUCount, pGPUs));

    // 由于个数不确定，又要在栈上分配，所以用alloca分配速度最快
    RHIGPUSettings* pGPUSettings  = (RHIGPUSettings*)JALLOCA(uGPUCount * sizeof(RHIGPUSettings));
    bool*           pGPUValid     = (bool*)JALLOCA(uGPUCount * sizeof(bool));
    u32             uRealGPUCount = 0;
    for (u32 i = 0; i < uGPUCount; ++i)
    {
        u32                      uQueueFamilyPropertyCount = 0;
        VkQueueFamilyProperties* pQueueFamilyProperties    = nullptr;
        _RHIQueryGPUSettings(pGPUs[i], &pGPUProperties[i], &pGPUMemoryProperties[i], &pGPUFeatures[i], &pQueueFamilyProperties,
                             &uQueueFamilyPropertyCount, &pGPUSettings[i]);
        // 过滤一下GPU看那些不满足需求
        bool bSupportGraphics = false;
        for (u32 j = 0; j < uQueueFamilyPropertyCount; ++j)
        {
            // 必须有graphics的queue
            if (pQueueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                bSupportGraphics = true;
                break;
            }
        }
        // 不能是CPU假装GPU，连假装都不假装
        pGPUValid[i] = bSupportGraphics && (VK_PHYSICAL_DEVICE_TYPE_CPU != pGPUProperties[i].properties.deviceType);
        if (pGPUValid[i])
        {
            ++uRealGPUCount;
        }

        JFREE(pQueueFamilyProperties);
    }

    pContext->m_uGPUCount = uRealGPUCount;
    pContext->m_pGPUs     = (RHIGPUInfo*)JCALLOC(uRealGPUCount, sizeof(RHIGPUInfo));

    for (u32 i = 0, count = 0; i < uGPUCount; ++i)
    {
        if (!pGPUValid[i])
        {
            continue;
        }

        pContext->m_pGPUs[count].m_Settings                      = pGPUSettings[i];
        pContext->m_pGPUs[count].Vulkan.m_hVkGPU                 = pGPUs[i];
        pContext->m_pGPUs[count].Vulkan.m_hVkGPUProperties       = pGPUProperties[i];
        pContext->m_pGPUs[count].Vulkan.m_hVkGPUProperties.pNext = nullptr;

        // LOGF(LogLevel::eINFO, "GPU[%i] detected. Vendor ID: %s, Model ID: %s, Preset: %s, GPU Name: %s", realGpu,
        // 	gpuSettings[i].mGpuVendorPreset.mVendorId,
        // 	gpuSettings[i].mGpuVendorPreset.mModelId,
        // 	presetLevelToString(gpuSettings[i].mGpuVendorPreset.mPresetLevel),
        // 	gpuSettings[i].mGpuVendorPreset.mGpuName);

        ++count;
    }
    *ppContext = pContext;
}

void RHIExitRendererContextVK(RHIRendererContext* pContext)
{
    // TODO 这个变量因该记录到renderercontext里
    // JASSERT(gRendererCount==0);
    RHIRenderer fakeRenderer                     = {};
    fakeRenderer.Vulkan.m_hVkInstance            = pContext->Vulkan.m_hVkInstance;
    fakeRenderer.Vulkan.m_hVkDebugUtilsMessenger = pContext->Vulkan.m_hVkDebugUtilsMessager;
    _RHIExitCommonVK(&fakeRenderer);
    JFREE(pContext->m_pGPUs);
    JFREE(pContext);
}

void _RHIInitRendererFuncsVK()
{
    JREG_RHI_FUNC_VK(RHIAddFence)
    JREG_RHI_FUNC_VK(RHIRemoveFence)
}

void RHIInitRendererVK(const char* szAppName, const RHIRendererDesc* pDesc, RHIRenderer** ppRenderer)
{
    JASSERT(szAppName);
    JASSERT(pDesc);
    JASSERT(ppRenderer);

    _RHIInitRendererFuncsVK();

    // NullDescriptor是指针的原始是解除头文件依赖
    u8* pMem = (u8*)JCALLOC_ALIGNED(1, alignof(RHIRenderer), sizeof(RHIRenderer) + sizeof(RHINullDescriptors));
    JASSERT(pMem);

    RHIRenderer* pRenderer                = (RHIRenderer*)pMem;
    pRenderer->m_eGPUMode                 = pDesc->m_eGPUMode;
    pRenderer->m_eShaderMode              = pDesc->m_eShaderMode;
    pRenderer->m_bEnableGpuBaseValidation = pDesc->m_bEnableGPUBasedValidation;
    pRenderer->m_pNullDescriptors         = (RHINullDescriptors*)(pMem + sizeof(RHIRenderer));
    // TODO app name 定长就行了吧，还有结尾是否为0需要调一下
    pRenderer->m_szName = (char*)JMALLOC(strlen(szAppName) + 1);
    strcpy_s(pRenderer->m_szName, strlen(szAppName) + 1, szAppName);

    {
        JASSERT(pDesc->m_eGPUMode != ERHIGPUMode::Unlinked || pDesc->m_pRenderContext);
        // 如果是unlink的模式
        if (pDesc->m_pRenderContext)
        {
            JASSERT(pDesc->m_uGPUIndex < pDesc->m_pRenderContext->m_uGPUCount);
            pRenderer->Vulkan.m_hVkInstance            = pDesc->m_pRenderContext->Vulkan.m_hVkInstance;
            pRenderer->Vulkan.m_bOwnInstance           = false;
            pRenderer->Vulkan.m_hVkDebugUtilsMessenger = pDesc->m_pRenderContext->Vulkan.m_hVkDebugUtilsMessager;
            pRenderer->m_uUnlinkedRenderIndex          = pDesc->m_pRenderContext->m_uRendererCount;
        }
        else if (!_RHIInitCommonVK(szAppName, pDesc, pRenderer))
        {
            JFREE(pRenderer->m_szName);
            JFREE(pRenderer);
            return;
        }

        if (_RHIAddDeviceVK(pDesc, pRenderer))
        {
        }
    }
    ++pDesc->m_pRenderContext->m_uRendererCount;
}

void RHIExitRendererVK(RHIRenderer* pRenderer)
{
}