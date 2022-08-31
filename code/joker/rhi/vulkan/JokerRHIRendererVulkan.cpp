#include "JokerRHIPCH.h"
#include "JokerRHIRendererVulkan.h"
#include "JokerRHIVulkan.h"
#include "vulkan/JokerRHIVulkan.h"

namespace joker
{

RHIRenderer*              g_pRendererVulkan = nullptr;

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

RHIRenderer* RHIInitRendererVulkan(const RHIRendererDesc& desc)
{
    JASSERT(g_pRendererVulkan == nullptr);
    g_pRendererVulkan = new RHIRendererVulkan(desc);
    return g_pRendererVulkan;
}

void RHIExitRendererVulkan(RHIRenderer* pRenderer)
{
    JASSERT(g_pRendererVulkan == pRenderer);
    delete pRenderer;
    g_pRendererVulkan = nullptr;
}

RHIRendererVulkan::RHIRendererVulkan(const RHIRendererDesc& desc)
{
}

RHIRendererVulkan::~RHIRendererVulkan()
{
    delete[] m_pInstanceSupportLayers;
    delete[] m_pInstanceSupportExtensions;
}

void RHIRendererVulkan::_CreateInstance()
{
    volkInitialize();

    vkEnumerateInstanceLayerProperties(&m_uInstanceSupportLayersCount, nullptr);
    vkEnumerateInstanceExtensionProperties(nullptr, &m_uInstanceSupportExtensionsCount, nullptr);

    m_pInstanceSupportLayers     = new VkLayerProperties[m_uInstanceSupportLayersCount];
    m_pInstanceSupportExtensions = new VkExtensionProperties[m_uInstanceSupportExtensionsCount];

    vkEnumerateInstanceLayerProperties(&m_uInstanceSupportExtensionsCount, m_pInstanceSupportLayers);
    vkEnumerateInstanceExtensionProperties(nullptr, &m_uInstanceSupportExtensionsCount, m_pInstanceSupportExtensions);

    for (u32 i = 0; i < m_uInstanceSupportLayersCount; i++)
    {
        JLOG_INFO("VkLayerProperties {}: {}", i, m_pInstanceSupportLayers[i].layerName);
    }

    for (u32 i = 0; i < m_uInstanceSupportExtensionsCount; i++)
    {
        JLOG_INFO("VkExtensionProperties {}: {}", i, m_pInstanceSupportExtensions[i].extensionName);
    }

    if (m_Desc.bCPUDebug)
    {
        _CheckAndAddLayer("VK_LAYER_KHRONOS_validation", m_uInstanceSupportLayersCount, m_pInstanceSupportLayers, m_vInstanceUsedLayers);
        _CheckAndAddExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, m_uInstanceSupportExtensionsCount, m_pInstanceSupportExtensions, m_vInstanceUsedExtensions);
    }
    _CheckVersion(VK_VERSION_1_1);

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
    if (m_Desc.bGPUDebug)
    {
        validationFeaturesExt.enabledValidationFeatureCount = 1;
        validationFeaturesExt.pEnabledValidationFeatures    = enabledValidationFeatures;
    }

    VkApplicationInfo appInfo{};
    appInfo.sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext            = nullptr;
    appInfo.pApplicationName = m_Desc.szAppName.c_str();
    appInfo.pEngineName      = "joker";
    appInfo.engineVersion    = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion       = VK_API_VERSION_1_1;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext                   = &validationFeaturesExt;
    createInfo.flags                   = 0;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledLayerCount       = (u32)m_vInstanceUsedLayers.size();
    createInfo.ppEnabledLayerNames     = m_vInstanceUsedLayers.data();
    createInfo.enabledExtensionCount   = (u32)m_vInstanceUsedExtensions.size();
    createInfo.ppEnabledExtensionNames = m_vInstanceUsedExtensions.data();
    JRHI_VK_CHECK(vkCreateInstance(&createInfo, nullptr, (VkInstance*)m_pHWContext));
    vkCreateInstance(&createInfo, nullptr, (VkInstance*)m_pHWContext);

    // 必须加载instance的接口
    volkLoadInstanceOnly(*(VkInstance*)(m_pHWContext));

    if (m_Desc.bCPUDebug && _CheckExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, m_uInstanceSupportExtensionsCount, m_pInstanceSupportExtensions))
    {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
        debugInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.pNext           = nullptr;
        debugInfo.pUserData       = nullptr;
        debugInfo.pfnUserCallback = _vkDebugUtilsMessengerCallback;
        debugInfo.flags           = 0;
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        JRHI_VK_CHECK(vkCreateDebugUtilsMessengerEXT(JRHI_VK_INSTANCE, &debugInfo, m_pVkAllocationCallbacks, &m_hVkDebugMessenger));
    }
}

void RHIRendererVulkan::_CreateDevice()
{
    VkDeviceGroupDeviceCreateInfoKHR deviceGroupInfo;
    if (_CheckExtension(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME, m_uInstanceSupportExtensionsCount, m_pInstanceSupportExtensions))
    {
        deviceGroupInfo.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO_KHR;
        deviceGroupInfo.pNext = nullptr;
        // pRenderer->m_uLinkedNodeCount = 1;
        u32 uDeviceGroupCount = 0;
        vkEnumeratePhysicalDeviceGroupsKHR(*(VkInstance*)(m_pHWContext), &uDeviceGroupCount, NULL);
        VkPhysicalDeviceGroupPropertiesKHR* props = (VkPhysicalDeviceGroupPropertiesKHR*)JALLOCA(uDeviceGroupCount * sizeof(VkPhysicalDeviceGroupPropertiesKHR));
        for (uint32_t i = 0; i < uDeviceGroupCount; ++i)
        {
            props[i].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES_KHR;
            props[i].pNext = nullptr;
        }
        JRHI_VK_CHECK(vkEnumeratePhysicalDeviceGroupsKHR(*(VkInstance*)(m_pHWContext), &uDeviceGroupCount, props));
        for (uint32_t i = 0; i < uDeviceGroupCount; ++i)
        {
            if (props[i].physicalDeviceCount > 1)
            {
                deviceGroupInfo.physicalDeviceCount = props[i].physicalDeviceCount;
                deviceGroupInfo.pPhysicalDevices    = props[i].physicalDevices;
                // pRenderer->m_uLinkedNodeCount       = deviceGroupInfo.physicalDeviceCount;
                break;
            }
        }
    }

    _SelectBestCPU();

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

    // vkEnumerateDeviceExtensionProperties(pRenderer->Vulkan.m_hVkActiveGPU, szLayerName, &uCount, nullptr);
    m_bDedicatedAllocationExtension           = _vkCheckNAddExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bGetMemoryRequirement2Extension         = _vkCheckNAddExtension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bExternalMemoryExtension                = _vkCheckNAddExtension(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bExternalMemoryWin32Extension           = _vkCheckNAddExtension(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bDrawIndirectCountExtension             = _vkCheckNAddExtension(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bAMDDrawIndirectCountExtension          = _vkCheckNAddExtension(VK_AMD_DRAW_INDIRECT_COUNT_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bAMDGCNShaderExtension                  = _vkCheckNAddExtension(VK_AMD_GCN_SHADER_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bDescriptorIndexingExtension            = _vkCheckNAddExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bShaderFloatControlsExtension           = _vkCheckNAddExtension(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bBufferDeviceAddressExtension           = _vkCheckNAddExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bDeferredHostOperationsExtension        = _vkCheckNAddExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bAccelerationStructureExtension         = _vkCheckNAddExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bSpirv14Extension                       = _vkCheckNAddExtension(VK_KHR_SPIRV_1_4_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bRayTracingPipelineExtension            = _vkCheckNAddExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bRayQueryExtension                      = _vkCheckNAddExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bSamplerYCbCrConversionExtension        = _vkCheckNAddExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bFragmentShaderInterlockExtension       = _vkCheckNAddExtension(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bMultiviewExtension                     = _vkCheckNAddExtension(VK_EXT_MULTI_DRAW_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bNvDeviceDiagnosticCheckpointsExtension = _vkCheckNAddExtension(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME, uExtCount, pExts, pRenderer);
    m_bNvDeviceDiagnosticsConfigExtension     = _vkCheckNAddExtension(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME, uExtCount, pExts, pRenderer);

    m_bRaytracingSupported = m_bShaderFloatControlsExtension && m_bBufferDeviceAddressExtension && m_bBufferDeviceAddressExtension && m_bDeferredHostOperationsExtension &&
                             m_bAccelerationStructureExtension && m_bSpirv14Extension && m_bRayTracingPipelineExtension;
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

    JRHI_VK_CHECK(vkCreateDevice(m_hVkActiveDevice, &createInfo, kAllocator, &pRenderer->Vulkan.m_hVkDevice));

    volkLoadDevice(JRHI_VK_DEVICE);
}

void RHIRendererVulkan::_CreateVmaAllocator()
{
        VmaAllocatorCreateInfo createInfo = {0};
    createInfo.device                 = pRenderer->Vulkan.m_hVkDevice;
    createInfo.physicalDevice         = pRenderer->Vulkan.m_hVkActiveGPU;
    createInfo.instance               = pRenderer->m_pContext->Vulkan.m_hVkInstance;

    if (m_bDedicatedAllocationExtension)
    {
        createInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
    }

    if (m_bBufferDeviceAddressExtension)
    {
        createInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    }

    VmaVulkanFunctions vulkanFunctions                  = {};
    vulkanFunctions.vkGetInstanceProcAddr               = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr                 = vkGetDeviceProcAddr;
    vulkanFunctions.vkAllocateMemory                    = vkAllocateMemory;
    vulkanFunctions.vkBindBufferMemory                  = vkBindBufferMemory;
    vulkanFunctions.vkBindImageMemory                   = vkBindImageMemory;
    vulkanFunctions.vkCreateBuffer                      = vkCreateBuffer;
    vulkanFunctions.vkCreateImage                       = vkCreateImage;
    vulkanFunctions.vkDestroyBuffer                     = vkDestroyBuffer;
    vulkanFunctions.vkDestroyImage                      = vkDestroyImage;
    vulkanFunctions.vkFreeMemory                        = vkFreeMemory;
    vulkanFunctions.vkGetBufferMemoryRequirements       = vkGetBufferMemoryRequirements;
    vulkanFunctions.vkGetBufferMemoryRequirements2KHR   = vkGetBufferMemoryRequirements2KHR;
    vulkanFunctions.vkGetImageMemoryRequirements        = vkGetImageMemoryRequirements;
    vulkanFunctions.vkGetImageMemoryRequirements2KHR    = vkGetImageMemoryRequirements2KHR;
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vulkanFunctions.vkGetPhysicalDeviceProperties       = vkGetPhysicalDeviceProperties;
    vulkanFunctions.vkMapMemory                         = vkMapMemory;
    vulkanFunctions.vkUnmapMemory                       = vkUnmapMemory;
    vulkanFunctions.vkFlushMappedMemoryRanges           = vkFlushMappedMemoryRanges;
    vulkanFunctions.vkInvalidateMappedMemoryRanges      = vkInvalidateMappedMemoryRanges;
    vulkanFunctions.vkCmdCopyBuffer                     = vkCmdCopyBuffer;
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
    /// Fetch "vkBindBufferMemory2" on Vulkan >= 1.1, fetch "vkBindBufferMemory2KHR" when using VK_KHR_bind_memory2 extension.
    vulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
    /// Fetch "vkBindImageMemory2" on Vulkan >= 1.1, fetch "vkBindImageMemory2KHR" when using VK_KHR_bind_memory2 extension.
    vulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
#ifdef NX64
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
#else
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
#endif
#endif
#if VMA_VULKAN_VERSION >= 1003000
    /// Fetch from "vkGetDeviceBufferMemoryRequirements" on Vulkan >= 1.3, but you can also fetch it from "vkGetDeviceBufferMemoryRequirementsKHR" if you enabled extension
    /// VK_KHR_maintenance4.
    vulkanFunctions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
    /// Fetch from "vkGetDeviceImageMemoryRequirements" on Vulkan >= 1.3, but you can also fetch it from "vkGetDeviceImageMemoryRequirementsKHR" if you enabled extension
    /// VK_KHR_maintenance4.
    vulkanFunctions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;
#endif
    createInfo.pVulkanFunctions     = &vulkanFunctions;
    createInfo.pAllocationCallbacks = kAllocator;
    vmaCreateAllocator(&createInfo, m_pVmaAllocator);
}

void RHIRendererVulkan::_SelectBestCPU()
{
    JRHI_VK_CHECK(vkEnumeratePhysicalDevices(, &m_uDeviceCount, nullptr));

    VkPhysicalDevice*                 pGPUs                     = (VkPhysicalDevice*)alloca(m_uDeviceCount * sizeof(VkPhysicalDevice));
    VkPhysicalDeviceProperties2*      pGPUProperties            = (VkPhysicalDeviceProperties2*)alloca(m_uDeviceCount * sizeof(VkPhysicalDeviceProperties2));
    VkPhysicalDeviceMemoryProperties* pGPUMemoryProperties      = (VkPhysicalDeviceMemoryProperties*)alloca(m_uDeviceCount * sizeof(VkPhysicalDeviceMemoryProperties));
    VkPhysicalDeviceFeatures2KHR*     pGPUFeatures              = (VkPhysicalDeviceFeatures2KHR*)alloca(m_uDeviceCount * sizeof(VkPhysicalDeviceFeatures2KHR));
    VkQueueFamilyProperties**         ppQueueFamilyProperties   = (VkQueueFamilyProperties**)alloca(m_uDeviceCount * sizeof(VkQueueFamilyProperties*));
    u32*                              pQueueFamilyPropertyCount = (u32*)alloca(m_uDeviceCount * sizeof(u32));

    JRHI_VK_CHECK(vkEnumeratePhysicalDevices(JRHI_VK_INSTANCE, &m_uDeviceCount, pGPUs));

    u32 uGPUIndex = UINT32_MAX;
    // GPUSettings* pGPUSettings = (GPUSettings*)alloca(uGPUCount * sizeof(GPUSettings));
    for (uint32_t i = 0; i < m_uDeviceCount; ++i)
    {
        _QueryGPUProperties(pGPUs[i], &pGPUProperties[i], &pGPUMemoryProperties[i], &pGPUFeatures[i], &ppQueueFamilyProperties[i], &pQueueFamilyPropertyCount[i]);

        // JLOG_INFO("GPU[{}] detected. Vendor ID:{}, Model ID: {}, Preset: {}, GPU Name: {}, driver version: {}", i, pGPUSettings[i].m_GPUVendorPreset.m_szVendorId,
        //           pGPUSettings[i].m_GPUVendorPreset.m_szModelId, EGPUPresetLevelToString(pGPUSettings[i].m_GPUVendorPreset.m_ePresetLevel),
        //           pGPUSettings[i].m_GPUVendorPreset.m_szGPUName, pGPUSettings[i].m_GPUVendorPreset.m_szGPUDriverVersion);

        if (uGPUIndex == UINT32_MAX || _DeviceBetterFunc(i, uGPUIndex, pGPUProperties, pGPUMemoryProperties))
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
        return;
    }

    JASSERT(uGPUIndex != UINT32_MAX);
    m_hVkActiveDevice                = pGPUs[uGPUIndex];
    m_VkActiveDeviceProperties       = pGPUProperties[uGPUIndex];
    m_VkActiveDeviceProperties.pNext = nullptr;
    JASSERT(VK_NULL_HANDLE != m_hVkActiveDevice);

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

bool RHIRendererVulkan::_CheckVersion(u32 uNeedVersion)
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

bool RHIRendererVulkan::_CheckLayer(const char* szName, u32 uCount, VkLayerProperties* pSupports)
{
    for (u32 i = 0; i < uCount; ++i)
    {
        if (strcmp(szName, pSupports[i].layerName) == 0)
        {
            return true;
        }
    }
    return false;
}

bool RHIRendererVulkan::_CheckAndAddLayer(const char* szName, u32 uCount, VkLayerProperties* pSupports, vector<const char*>& vUsed)
{
    for (auto v : vUsed)
    {
        if (strcmp(szName, v) == 0)
        {
            return true;
        }
    }
    for (u32 i = 0; i < uCount; ++i)
    {
        if (strcmp(szName, pSupports[i].layerName) == 0)
        {
            vUsed.push_back(szName);
            return true;
        }
    }
    return false;
}

bool RHIRendererVulkan::_CheckExtension(const char* szName, u32 uCount, VkExtensionProperties* pSupports)
{
    for (u32 i = 0; i < uCount; ++i)
    {
        if (strcmp(szName, pSupports[i].extensionName) == 0)
        {
            return true;
        }
    }
    return false;
}

bool RHIRendererVulkan::_CheckAndAddExtension(const char* szName, u32 uCount, VkExtensionProperties* pSupports, vector<const char*>& vUsed)
{
    for (auto v : vUsed)
    {
        if (strcmp(szName, v) == 0)
        {
            return true;
        }
    }
    for (u32 i = 0; i < uCount; ++i)
    {
        if (strcmp(szName, pSupports[i].extensionName) == 0)
        {
            vUsed.push_back(szName);
            return true;
        }
    }
    return false;
}

bool RHIRendererVulkan::_DeviceBetterFunc(u32 uTestIndex, u32 uRefIndex, const VkPhysicalDeviceProperties2* pGPUProperties,
                                          const VkPhysicalDeviceMemoryProperties* pGPUMemoryProperties)
{
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
}

void RHIRendererVulkan::_QueryGPUProperties(VkPhysicalDevice gpu, VkPhysicalDeviceProperties2* pGPUProperties, VkPhysicalDeviceMemoryProperties* pGPUMemProperties,
                                            VkPhysicalDeviceFeatures2* pGPUFeatures, VkQueueFamilyProperties** ppQueueFamilyProperties, u32* pQueueFamilyPropertyCount)
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
};

}