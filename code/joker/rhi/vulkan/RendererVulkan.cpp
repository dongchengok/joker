#include "JokerRHIPCH.h"
#include "RendererVulkan.h"
#include "RendererPrivateVulkan.h"
#include "RendererInit.h"

namespace joker::rhi::vulkan
{

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
    "",
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
    JLOG_ERROR("need version:{}.{}.{}.{} but only:{}.{}.{}.{}", VK_API_VERSION_VARIANT(uNeedVersion), VK_API_VERSION_MAJOR(uNeedVersion),
               VK_API_VERSION_MINOR(uNeedVersion), VK_API_VERSION_PATCH(uNeedVersion), VK_API_VERSION_VARIANT(uVersion),
               VK_API_VERSION_MAJOR(uVersion), VK_API_VERSION_MINOR(uVersion), VK_API_VERSION_PATCH(uVersion));
    return false;
}

// 检查验证层
static bool _vkCheckNAddValidationLayer(const char* szName, u32 uCount, const VkLayerProperties* pLayers, u32* pUsedCount,
                                        const char** ppUsedValidationLayers)
{
    for (u32 i = 0; i < uCount; ++i)
    {
        if (strcmp(szName, pLayers[i].layerName) == 0)
        {
            ppUsedValidationLayers[(*pUsedCount)++] = pLayers[i].layerName;
            return true;
        }
    }
    JLOG_WARN("can not find layer {}", szName);
    return false;
}

// 检查扩展
static bool _vkCheckNAddExtension(const char* szName, u32 uCount, const VkExtensionProperties* pExts, u32* pUsedCount, const char** ppUsedExtensions)
{
    for (u32 i = 0; i < uCount; ++i)
    {
        if (strcmp(szName, pExts[i].extensionName) == 0)
        {
            ppUsedExtensions[(*pUsedCount)++] = pExts[i].extensionName;
            return true;
        }
    }
    JLOG_WARN("can not find extension {}", szName);
    return false;
}

static VkBool32 VKAPI_PTR _vkDebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                         VkDebugUtilsMessageTypeFlagsEXT             messageType,
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

static VkInstance _vkCreateInstance(const char* szAppName, bool bEnableValidation, bool bEnableGPUBasedValidation, bool bEnableDebugUtilsMessager)
{
    JCHECK_RHI_RESULT(volkInitialize());

    // 检查包含哪些layer
    u32 uLayerCount{0};
    vkEnumerateInstanceLayerProperties(&uLayerCount, nullptr);
    VkLayerProperties* pLayers          = (VkLayerProperties*)JALLOC(sizeof(VkLayerProperties) * uLayerCount);
    const char**       ppUsedLayerNames = (const char**)JALLOC(sizeof(char*) * uLayerCount);
    u32                uUsedLayerCount  = 0;
    vkEnumerateInstanceLayerProperties(&uLayerCount, pLayers);
    for (u32 i = 0; i < uLayerCount; ++i)
    {
        JLOG_INFO("VkLayerProperties {}: {}", i, pLayers[i].layerName);
    }

    if (bEnableValidation)
    {
        _vkCheckNAddValidationLayer("VK_LAYER_KHRONOS_validation", uLayerCount, pLayers, &uUsedLayerCount, ppUsedLayerNames);
        _vkCheckNAddValidationLayer("VK_LAYER_LUNARG_monitor", uLayerCount, pLayers, &uUsedLayerCount, ppUsedLayerNames);
    }

    // 检查包含哪些扩展
    u32 uExtCount{0};
    vkEnumerateInstanceExtensionProperties(nullptr, &uExtCount, nullptr);
    VkExtensionProperties* pExts                = (VkExtensionProperties*)JALLOC(sizeof(VkExtensionProperties) * uExtCount);
    const char**           ppUsedExtensionNames = (const char**)JALLOC(sizeof(char*) * uExtCount);
    u32                    uUsedExtensionCount  = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &uExtCount, pExts);
    for (u32 i = 0; i < uExtCount; ++i)
    {
        JLOG_INFO("VkLayerProperties {}: {}", i, pExts[i].extensionName);
    }

    if (bEnableDebugUtilsMessager)
    {
        _vkCheckNAddExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, uExtCount, pExts, &uUsedExtensionCount, ppUsedExtensionNames);
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
    if (bEnableGPUBasedValidation)
    {
        validationFeaturesExt.enabledValidationFeatureCount = 1;
        validationFeaturesExt.pEnabledValidationFeatures    = enabledValidationFeatures;
    }

    VkApplicationInfo appInfo{};
    appInfo.sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext            = nullptr;
    appInfo.pApplicationName = szAppName;
    appInfo.pEngineName      = "joker";
    appInfo.engineVersion    = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion       = VK_API_VERSION_1_1;

    VkInstance           instance;
    VkInstanceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext                   = &validationFeaturesExt;
    createInfo.flags                   = 0;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledLayerCount       = uUsedLayerCount;
    createInfo.ppEnabledLayerNames     = uUsedLayerCount ? ppUsedLayerNames : nullptr;
    createInfo.enabledExtensionCount   = uUsedExtensionCount;
    createInfo.ppEnabledExtensionNames = uUsedExtensionCount ? ppUsedExtensionNames : nullptr;
    JCHECK_RHI_RESULT(vkCreateInstance(&createInfo, nullptr, &instance));

    // 必须加载instance的接口
    volkLoadInstanceOnly(instance);

    if (bEnableDebugUtilsMessager)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
        debugInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.pNext           = nullptr;
        debugInfo.pUserData       = nullptr;
        debugInfo.pfnUserCallback = _vkDebugUtilsMessengerCallback;
        debugInfo.flags           = 0;
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        VkDebugUtilsMessengerEXT messager;
        JCHECK_RHI_RESULT(vkCreateDebugUtilsMessengerEXT(instance, &debugInfo, kAllocator, &messager));
        // TODO messager传出去
    }

    return instance;
}



RendererContext* vkInitRendererContext(const RendererContextDesc* pDesc)
{
    VkInstance hVkInstance =
        _vkCreateInstance(pDesc->m_szAppName, pDesc->m_bEnableValidation, pDesc->m_bEnableGPUBasedValidation, pDesc->m_bEnableDebugUtilsMessager);
    JASSERT(hVkInstance);
    return nullptr;
}

void vkExitRendererContext(RendererContext* pContext)
{
    if (pContext->Vulkan.m_hVkDebugMessenger)
    {
        vkDestroyDebugUtilsMessengerEXT(pContext->Vulkan.m_hVkInstance, pContext->Vulkan.m_hVkDebugMessenger,
                                        pContext->Vulkan.m_pVkAllocationCallbacks);
    }
    vkDestroyInstance(pContext->Vulkan.m_hVkInstance, pContext->Vulkan.m_pVkAllocationCallbacks);
    JFREE(pContext);
}

Renderer* vkInitRenderer(const RendererDesc* pDesc)
{
    u8*           pMem        = (u8*)JCALLOC_ALIGNED(1, alignof(Renderer*), sizeof(Renderer) + sizeof(NullDescriptors));
    JASSERT(pMem);

    Renderer* pRenderer                   = (Renderer*)pMem;
    pRenderer->m_eGPUMode                 = pDesc->m_eGPUMode;
    pRenderer->m_eShaderMode              = pDesc->m_eShaderMode;
    pRenderer->m_bEnableGpuBaseValidation = pDesc->m_bEnableGPUBaseValidation;
    pRenderer->m_pNullDescriptors         = (NullDescriptors*)(pMem + sizeof(Renderer));

    // pRenderer->m_szName = (char*)JCALLOC(strlen(pDesc.sz))

    //如果是unlinked模式，就必须有RendererContext
    JASSERT(pDesc->m_eGPUMode != EGPUMode::Unlinked || pDesc);
    if (pDesc->m_pRenderContext)
    {
        pRenderer->Vulkan.m_hVkInstance       = pDesc->m_pRenderContext->Vulkan.m_hVkInstance;
        pRenderer->Vulkan.m_bOwnInstance      = false;
        pRenderer->Vulkan.m_hVkDebugMessenger = pDesc->m_pRenderContext->Vulkan.m_hVkDebugMessenger;
        // TODO
        pRenderer->Vulkan.m_uDeviceIndex = 0;
    }
    else
    {
        pRenderer->Vulkan.m_hVkInstance =
            _vkCreateInstance(pDesc->m_szAppName, pDesc->m_bEnableValidation, pDesc->m_bEnableGPUBaseValidation, pDesc->m_bEnableDebugUtilsMessager);
        JASSERT(pRenderer->Vulkan.m_hVkInstance);
        pRenderer->Vulkan.m_bOwnInstance = true;
    }

    return nullptr;
}

void vkExitRenderer(Renderer* pRenderer)
{
}

}