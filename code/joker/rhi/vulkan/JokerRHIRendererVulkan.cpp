#include "JokerRHIPCH.h"
#include "JokerRHIRenderer.h"
#include "JokerRHIRendererVulkan.h"
#include "vulkan/JokerRHIVulkan.h"
#include <vulkan/vulkan_core.h>

namespace joker
{

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
    return new RHIRendererVulkan(desc);
}

void RHIExitRendererVulkan(RHIRenderer* pRenderer)
{
    delete pRenderer;
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
        JRHI_VK_CHECK(vkCreateDebugUtilsMessengerEXT(*(VkInstance*)(m_pHWContext), &debugInfo, m_pVkAllocationCallbacks, &m_hVkDebugMessenger));
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
}

void RHIRendererVulkan::_SelectBestCPU()
{
    VkInstance                        hVkInstance               = pRenderer->m_pContext->Vulkan.m_hVkInstance;

    JRHI_VK_CHECK(vkEnumeratePhysicalDevices(, &m_uDeviceCount, nullptr));

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

}