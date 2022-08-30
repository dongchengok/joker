#include "JokerRHIPCH.h"
#include "JokerRHIRendererVulkan.h"

namespace joker
{

RHIRenderer* RHIInitRendererVulkan(const RHIRendererDesc& desc)
{
    return new RHIRendererVulkan();
}

void RHIExitRendererVulkan(RHIRenderer* pRenderer)
{
    delete pRenderer;
}

RHIRendererVulkan::RHIRendererVulkan()
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

    _CheckAndAddLayer("VK_LAYER_KHRONOS_validation", m_uInstanceSupportLayersCount, m_pInstanceSupportLayers, m_vInstanceUsedLayers);
    _CheckAndAddExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, m_uInstanceSupportExtensionsCount, m_pInstanceSupportExtensions, m_vInstanceUsedExtensions);
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
    // if (pDesc->m_bGPUDebug)
    // {
    validationFeaturesExt.enabledValidationFeatureCount = 1;
    validationFeaturesExt.pEnabledValidationFeatures    = enabledValidationFeatures;
    // }

    VkApplicationInfo appInfo{};
    appInfo.sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext            = nullptr;
    appInfo.pApplicationName = "hahahahaha";
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
    // JCHECK_RHI_RESULT(vkCreateInstance(&createInfo, nullptr, (VkInstance*)m_pHWContext));
    vkCreateInstance(&createInfo, nullptr, (VkInstance*)m_pHWContext);

//     // 必须加载instance的接口
//     volkLoadInstanceOnly((VkInstance)(*m_pHWContext));

//     if (pDesc->m_bDebug && _vkCheckExtension(&pContext->Vulkan, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
//     {
//         VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
//         debugInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
//         debugInfo.pNext           = nullptr;
//         debugInfo.pUserData       = nullptr;
//         debugInfo.pfnUserCallback = _vkDebugUtilsMessengerCallback;
//         debugInfo.flags           = 0;
//         debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
//         debugInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
//         JCHECK_RHI_RESULT(vkCreateDebugUtilsMessengerEXT(hVkInstance, &debugInfo, pContext->Vulkan.m_pVkAllocationCallbacks, &pContext->Vulkan.m_hVkDebugMessenger));
//     }
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