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
    RHIRendererVulkan* pVulkan = new RHIRendererVulkan(desc);
    g_pRendererVulkan          = pVulkan;
    pVulkan->Init();
    return g_pRendererVulkan;
}

void RHIExitRendererVulkan(RHIRenderer* pRenderer)
{
    JASSERT(g_pRendererVulkan == pRenderer);
    RHIRendererVulkan* pVulkan = (RHIRendererVulkan*)g_pRendererVulkan;
    pVulkan->Exit();
    delete pRenderer;
    g_pRendererVulkan = nullptr;
}

RHIRendererVulkan::RHIRendererVulkan(const RHIRendererDesc& desc) : RHIRenderer(desc)
{
    m_pInfo = new RendererInfo();
}

RHIRendererVulkan::~RHIRendererVulkan()
{
    delete m_pInfo;
    delete[] m_pInstanceSupportLayers;
    delete[] m_pInstanceSupportExtensions;
}

void RHIRendererVulkan::Init()
{
    _CreateInstance();
    _QueryGPUInfos();
    _SelectBestCPU();
    _CreateDevice();
}

void RHIRendererVulkan::Exit()
{
    vkDestroyDevice(JRHI_VK_DEVICE, m_pInfo->pAllocationCallbacks);
    if (m_hVkDebugMessenger)
    {
        vkDestroyDebugUtilsMessengerEXT(JRHI_VK_INSTANCE, m_hVkDebugMessenger, m_pInfo->pAllocationCallbacks);
    }
    vkDestroyInstance(JRHI_VK_INSTANCE, m_pInfo->pAllocationCallbacks);
}

n32 RHIRendererVulkan::GetGPUCount() const
{
    return (n32)m_pInfo->vGPUs.size();
}

n32 RHIRendererVulkan::GetGPUUsingIndex() const
{
    return (n32)m_pInfo->uUsingGPUIndex;
}

const string& RHIRendererVulkan::GetGPUName(n32 idx) const
{
    return m_pInfo->vGPUs[idx].szGPUName;
}

const string& RHIRendererVulkan::GetGPUVendor(n32 idx) const
{
    return m_pInfo->vGPUs[idx].szVendor;
}

const string& RHIRendererVulkan::GetGPUModel(n32 idx) const
{
    return m_pInfo->vGPUs[idx].szModel;
}

void RHIRendererVulkan::_CreateInstance()
{
    volkInitialize();

    vkEnumerateInstanceLayerProperties(&m_uInstanceSupportLayersCount, nullptr);
    vkEnumerateInstanceExtensionProperties(nullptr, &m_uInstanceSupportExtensionsCount, nullptr);

    m_pInstanceSupportLayers     = new VkLayerProperties[m_uInstanceSupportLayersCount];
    m_pInstanceSupportExtensions = new VkExtensionProperties[m_uInstanceSupportExtensionsCount];

    vkEnumerateInstanceLayerProperties(&m_uInstanceSupportLayersCount, m_pInstanceSupportLayers);
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
    JRHI_VK_CHECK(vkCreateInstance(&createInfo, nullptr, (VkInstance*)&m_pHWContext));

    // 必须加载instance的接口
    volkLoadInstanceOnly(*(VkInstance*)(&m_pHWContext));

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
    u32 uLayerCount = 0;
    u32 uExtCount   = 0;
    vkEnumerateDeviceLayerProperties(m_hVkActiveDevice, &uLayerCount, nullptr);
    vkEnumerateDeviceExtensionProperties(m_hVkActiveDevice, nullptr, &uExtCount, nullptr);
    VkLayerProperties*     pLayers = (VkLayerProperties*)alloca(sizeof(VkLayerProperties) * uLayerCount);
    VkExtensionProperties* pExts   = (VkExtensionProperties*)alloca(sizeof(VkExtensionProperties) * uExtCount);
    vkEnumerateDeviceLayerProperties(m_hVkActiveDevice, &uLayerCount, pLayers);
    vkEnumerateDeviceExtensionProperties(m_hVkActiveDevice, nullptr, &uExtCount, pExts);
    for (u32 i = 0; i < uLayerCount; ++i)
    {
        JLOG_INFO("Device VkLayerProperties {}: {}", i, pLayers[i].layerName);
    }
    for (u32 i = 0; i < uExtCount; ++i)
    {
        JLOG_INFO("Device VkExtensionProperties {}: {}", i, pExts[i].extensionName);
    }

    // vkEnumerateDeviceExtensionProperties(pRenderer->Vulkan.m_hVkActiveGPU, szLayerName, &uCount, nullptr);
    m_bDedicatedAllocationExtension           = _CheckAndAddExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bGetMemoryRequirement2Extension         = _CheckAndAddExtension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bExternalMemoryExtension                = _CheckAndAddExtension(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bExternalMemoryWin32Extension           = _CheckAndAddExtension(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bDrawIndirectCountExtension             = _CheckAndAddExtension(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bAMDDrawIndirectCountExtension          = _CheckAndAddExtension(VK_AMD_DRAW_INDIRECT_COUNT_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bAMDGCNShaderExtension                  = _CheckAndAddExtension(VK_AMD_GCN_SHADER_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bDescriptorIndexingExtension            = _CheckAndAddExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bShaderFloatControlsExtension           = _CheckAndAddExtension(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bBufferDeviceAddressExtension           = _CheckAndAddExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bDeferredHostOperationsExtension        = _CheckAndAddExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bAccelerationStructureExtension         = _CheckAndAddExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bSpirv14Extension                       = _CheckAndAddExtension(VK_KHR_SPIRV_1_4_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bRayTracingPipelineExtension            = _CheckAndAddExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bRayQueryExtension                      = _CheckAndAddExtension(VK_KHR_RAY_QUERY_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bSamplerYCbCrConversionExtension        = _CheckAndAddExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bFragmentShaderInterlockExtension       = _CheckAndAddExtension(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bMultiviewExtension                     = _CheckAndAddExtension(VK_EXT_MULTI_DRAW_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bNvDeviceDiagnosticCheckpointsExtension = _CheckAndAddExtension(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);
    m_bNvDeviceDiagnosticsConfigExtension     = _CheckAndAddExtension(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME, uExtCount, pExts, m_vDeviceUsedExtensions);

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
    _JRHI_VK_ADD_FEATURE(m_bFragmentShaderInterlockExtension, featureFragmentShaderInterlock);

    VkPhysicalDeviceDescriptorIndexingFeatures featureDescriptorIndexing{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES};
    _JRHI_VK_ADD_FEATURE(m_bDescriptorIndexingExtension, featureDescriptorIndexing);

    VkPhysicalDeviceSamplerYcbcrConversionFeatures featureSamplerYcbcr{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES};
    _JRHI_VK_ADD_FEATURE(m_bSamplerYCbCrConversionExtension, featureSamplerYcbcr);

    VkPhysicalDeviceMultiviewFeatures featureMultiview{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES};
    _JRHI_VK_ADD_FEATURE(m_bMultiviewExtension, featureMultiview);

    VkPhysicalDeviceBufferDeviceAddressFeatures featureBufferDeviceAddress{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES};
    featureBufferDeviceAddress.bufferDeviceAddress = VK_TRUE;
    _JRHI_VK_ADD_FEATURE(m_bBufferDeviceAddressExtension, featureBufferDeviceAddress);

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR featureRayTracingPipeline{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
    featureRayTracingPipeline.rayTracingPipeline = VK_TRUE;
    _JRHI_VK_ADD_FEATURE(m_bRayTracingPipelineExtension, featureRayTracingPipeline);

    VkPhysicalDeviceAccelerationStructureFeaturesKHR featureAccelerationStructure{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
    featureAccelerationStructure.accelerationStructure = VK_TRUE;
    _JRHI_VK_ADD_FEATURE(m_bAccelerationStructureExtension, featureAccelerationStructure);

    VkPhysicalDeviceRayQueryFeaturesKHR featureRayQuery{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR};
    featureRayQuery.rayQuery = VK_TRUE;
    _JRHI_VK_ADD_FEATURE(m_bRayQueryExtension, featureRayQuery);

#undef _JRHI_VK_ADD_FEATURE

    vkGetPhysicalDeviceFeatures2(m_hVkActiveDevice, &features2);

    u32 uQueueFamiliesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_hVkActiveDevice, &uQueueFamiliesCount, NULL);
    VkQueueFamilyProperties* pQueueFamiliesProperties = (VkQueueFamilyProperties*)JALLOCA(uQueueFamiliesCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(m_hVkActiveDevice, &uQueueFamiliesCount, pQueueFamiliesProperties);

    u32                      uQueueCreateInfosCount = 0;
    VkDeviceQueueCreateInfo* pQeueuCreateInfos      = (VkDeviceQueueCreateInfo*)JALLOCA(uQueueFamiliesCount * sizeof(VkDeviceQueueCreateInfo));

    // 队列优先级，vk好像可以把其它的队列饿死
    float* pPriorities = (float*)JALLOCA(sizeof(float) * m_Desc.uMaxQueueCount);
    memset(pPriorities, 0, sizeof(float) * m_Desc.uMaxQueueCount);
    for (u32 i = 0; i < uQueueFamiliesCount; i++)
    {
        u32 uQueueCount = pQueueFamiliesProperties[i].queueCount;
        if (uQueueCount > 0)
        {
            if (uQueueCount > 1 && !m_Desc.bUseAllQueue)
            {
                uQueueCount = 1;
            }
            pQeueuCreateInfos[uQueueCreateInfosCount]                  = {};
            pQeueuCreateInfos[uQueueCreateInfosCount].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            pQeueuCreateInfos[uQueueCreateInfosCount].pNext            = NULL;
            pQeueuCreateInfos[uQueueCreateInfosCount].flags            = 0;
            pQeueuCreateInfos[uQueueCreateInfosCount].queueFamilyIndex = i;
            pQeueuCreateInfos[uQueueCreateInfosCount].queueCount       = JMIN(uQueueCount, m_Desc.uMaxQueueCount);
            pQeueuCreateInfos[uQueueCreateInfosCount].pQueuePriorities = pPriorities;
            uQueueCreateInfosCount++;
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
    createInfo.enabledExtensionCount   = (u32)m_vDeviceUsedExtensions.size();
    createInfo.ppEnabledExtensionNames = m_vDeviceUsedExtensions.data();
    createInfo.pEnabledFeatures        = nullptr;

    JRHI_VK_CHECK(vkCreateDevice(m_hVkActiveDevice, &createInfo, m_pInfo->pAllocationCallbacks, (VkDevice*)&m_pHWDevice));

    volkLoadDevice(JRHI_VK_DEVICE);
}

void RHIRendererVulkan::_CreateVmaAllocator()
{
    VmaAllocatorCreateInfo createInfo = {0};
    createInfo.device                 = JRHI_VK_DEVICE;
    createInfo.physicalDevice         = m_hVkActiveDevice;
    createInfo.instance               = JRHI_VK_INSTANCE;

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
    createInfo.pAllocationCallbacks = m_pInfo->pAllocationCallbacks;
    vmaCreateAllocator(&createInfo, &m_pVmaAllocator);
}

void RHIRendererVulkan::_SelectBestCPU()
{
    m_pInfo->uUsingGPUIndex = UINT32_MAX;
    for (u32 i = 0; i < m_uDeviceCount; ++i)
    {
        if (m_pInfo->uUsingGPUIndex == UINT32_MAX || _DeviceBetterFunc(m_pInfo->vGPUs[i], m_pInfo->vGPUs[m_pInfo->uUsingGPUIndex]))
        {
            m_pInfo->uUsingGPUIndex = i;
        }
    }
    JASSERT(m_pInfo->uUsingGPUIndex != UINT32_MAX);
    m_hVkActiveDevice = m_pInfo->vGPUs[m_pInfo->uUsingGPUIndex].hVkPhysicalDevice;
    JASSERT(VK_NULL_HANDLE != m_hVkActiveDevice);

    const GPUInfo& gpu = m_pInfo->vGPUs[m_pInfo->uUsingGPUIndex];
    JLOG_INFO("GPU[{}] is selected as default GPU", m_pInfo->uUsingGPUIndex);
    JLOG_INFO("Name of selected gpu: {}", gpu.szGPUName.c_str());
    JLOG_INFO("Vendor id of selected gpu: {}", gpu.szVendor.c_str());
    JLOG_INFO("Model id of selected gpu: {}", gpu.szModel.c_str());
    JLOG_INFO("Heap memory of selected gpu: {}", gpu.uMemoryHeapSize);
    JLOG_INFO("Vulkan api version of selected gpu: {}.{}.{}", VK_VERSION_MAJOR(gpu.uApiVersion), VK_VERSION_MINOR(gpu.uApiVersion), VK_VERSION_PATCH(gpu.uApiVersion));
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

EGPUVendor RHIRendererVulkan::_GetGPUVendor(u32 uVendorId)
{
    constexpr u32 _kVkVendorIDNvidia = 0x10DE;
    constexpr u32 _kVkVendorIDAmd    = 0x1002;
    constexpr u32 _kVkVendorIDAmd1   = 0x1022;
    constexpr u32 _kVkVendorIDIntel  = 0x163C;
    constexpr u32 _kVkVendorIDIntel1 = 0x8086;
    constexpr u32 _kVkVendorIDIntel2 = 0x8087;
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

void RHIRendererVulkan::_QueryGPUInfos()
{
    JRHI_VK_CHECK(vkEnumeratePhysicalDevices(JRHI_VK_INSTANCE, &m_uDeviceCount, nullptr));
    VkPhysicalDevice* pGPUs = (VkPhysicalDevice*)alloca(m_uDeviceCount * sizeof(VkPhysicalDevice));
    JRHI_VK_CHECK(vkEnumeratePhysicalDevices(JRHI_VK_INSTANCE, &m_uDeviceCount, pGPUs));

    m_pInfo->vGPUs.resize(m_uDeviceCount);
    for (u32 i = 0; i < m_uDeviceCount; ++i)
    {
        GPUInfo&                                           info = m_pInfo->vGPUs[i];
        VkPhysicalDevice                                   gpu  = pGPUs[i];
        VkPhysicalDeviceMemoryProperties                   memory;
        VkPhysicalDeviceProperties2                        property;
        VkPhysicalDeviceFeatures2KHR                       feature;
        VkQueueFamilyProperties*                           pQueues;
        u32                                                uQueueCount;

        VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT featureFragmentInterlock;
        featureFragmentInterlock.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT;
        featureFragmentInterlock.pNext = nullptr;
        feature.sType                  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        feature.pNext                  = &featureFragmentInterlock;
        vkGetPhysicalDeviceFeatures2(gpu, &feature);

        VkPhysicalDeviceSubgroupProperties propsSubgroup;
        propsSubgroup.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
        propsSubgroup.pNext = nullptr;
        property.sType      = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        property.pNext      = &propsSubgroup;
        vkGetPhysicalDeviceProperties2(gpu, &property);

        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &uQueueCount, nullptr);
        pQueues = (VkQueueFamilyProperties*)JALLOCA(uQueueCount * sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &uQueueCount, pQueues);

        // 必须有graphic queue才可用
        info.bValid = false;
        for (u32 j = 0; j < uQueueCount; ++j)
        {
            info.bValid |= pQueues[j].queueFlags & VK_QUEUE_GRAPHICS_BIT;
        }

        info.hVkPhysicalDevice                = gpu;
        info.uUniformBufferAlignment          = (u32)property.properties.limits.minUniformBufferOffsetAlignment;
        info.uUploadBufferTextureAlignment    = (u32)property.properties.limits.optimalBufferCopyOffsetAlignment;
        info.uUploadBufferTextureRowAlignment = (u32)property.properties.limits.optimalBufferCopyRowPitchAlignment;
        info.uMaxVertexInputBindings          = property.properties.limits.maxVertexInputBindings;
        info.bMultiDrawIndirect               = feature.features.multiDrawIndirect;
        info.uWaveLaneCount                   = propsSubgroup.subgroupSize;
        info.bROVsSupported                   = featureFragmentInterlock.fragmentShaderPixelInterlock;
        info.bTessellationSupported           = feature.features.tessellationShader;
        info.bGeometryShaderSupported         = feature.features.geometryShader;

        switch (property.properties.deviceType)
        {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            info.eGPUType = EGPUType::Other;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            info.eGPUType = EGPUType::IntergratedGPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            info.eGPUType = EGPUType::VirtualGPU;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            info.eGPUType = EGPUType::CPU;
            break;
        default:
            info.eGPUType = EGPUType::Unknow;
        }

        info.eWaveOpsSupportFlags = EWaveOpsSupportFlags::None;
        if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_BASIC_BIT)
            info.eWaveOpsSupportFlags |= EWaveOpsSupportFlags::BasicBit;
        if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_VOTE_BIT)
            info.eWaveOpsSupportFlags |= EWaveOpsSupportFlags::VoteBit;
        if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT)
            info.eWaveOpsSupportFlags |= EWaveOpsSupportFlags::ArithmeticBit;
        if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_BALLOT_BIT)
            info.eWaveOpsSupportFlags |= EWaveOpsSupportFlags::BallotBit;
        if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_BIT)
            info.eWaveOpsSupportFlags |= EWaveOpsSupportFlags::ShuffleBit;
        if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT)
            info.eWaveOpsSupportFlags |= EWaveOpsSupportFlags::ShuffleRelativeBit;
        if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_CLUSTERED_BIT)
            info.eWaveOpsSupportFlags |= EWaveOpsSupportFlags::ClusteredBit;
        if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_QUAD_BIT)
            info.eWaveOpsSupportFlags |= EWaveOpsSupportFlags::QuadBit;
        if (propsSubgroup.supportedOperations & VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV)
            info.eWaveOpsSupportFlags |= EWaveOpsSupportFlags::PartitionedBitNV;

        // 计算显存
        vkGetPhysicalDeviceMemoryProperties(gpu, &memory);
        info.uMemoryHeapSize = 0;
        for (u32 i = 0; i < memory.memoryHeapCount; ++i)
        {
            if (VK_MEMORY_HEAP_DEVICE_LOCAL_BIT & memory.memoryHeaps[i].flags)
                info.uMemoryHeapSize += memory.memoryHeaps[i].size;
        }

        info.szModel.sprintf("%#x", property.properties.deviceID);
        info.szVendor.sprintf("%#x", property.properties.vendorID);
        info.szGPUName   = property.properties.deviceName;
        info.szRevision  = "0x00";
        info.uApiVersion = property.properties.apiVersion;

        // fill in driver info
        uint32_t major, minor, secondaryBranch, tertiaryBranch;
        switch (_GetGPUVendor(property.properties.vendorID))
        {
        case EGPUVendor::Nvidia:
            major           = (property.properties.driverVersion >> 22) & 0x3ff;
            minor           = (property.properties.driverVersion >> 14) & 0x0ff;
            secondaryBranch = (property.properties.driverVersion >> 6) & 0x0ff;
            tertiaryBranch  = (property.properties.driverVersion) & 0x003f;

            info.szGPUDriverVersion.sprintf("%u.%u.%u.%u", major, minor, secondaryBranch, tertiaryBranch);
            break;
        default:
            info.szGPUDriverVersion.sprintf("%u.%u.%u", VK_VERSION_MAJOR(property.properties.driverVersion), VK_VERSION_MINOR(property.properties.driverVersion),
                                            VK_VERSION_PATCH(property.properties.driverVersion));
            break;
        }

        JLOG_INFO("GPU[{}] detected. Vendor ID:{}, Model ID: {}, GPU Name: {}, driver version: {}", i, info.szVendor.c_str(), info.szModel.c_str(), info.szGPUName.c_str(),
                  info.szGPUDriverVersion.c_str());
    }
}

bool RHIRendererVulkan::_DeviceBetterFunc(const GPUInfo& testGPU, const GPUInfo& refGPU)
{
    if (!testGPU.bValid)
    {
        return false;
    }
    if (testGPU.eGPUType == EGPUType::DiscreteGPU && refGPU.eGPUType != EGPUType::DiscreteGPU)
    {
        return true;
    }
    if (testGPU.eGPUType != EGPUType::DiscreteGPU && refGPU.eGPUType == EGPUType::DiscreteGPU)
    {
        return false;
    }
    return testGPU.uMemoryHeapSize > refGPU.uMemoryHeapSize;
}

}