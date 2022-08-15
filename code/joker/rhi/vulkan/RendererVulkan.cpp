#include "JokerRHIPCH.h"
#include "RendererVulkan.h"
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

//TODO
constexpr static VkAllocationCallbacks* kAllocator = nullptr;

static bool _vkCheckValidationLayer(const char* szName, u32 uCount, const VkLayerProperties* pLayers)
{
    for (u32 i = 0; i < uCount; ++i)
    {
        if (strcmp(szName, pLayers[i].layerName))
        {
            return true;
        }
    }
    return false;
}

static bool _vkCheckValidationExtension(const char* szName, u32 uCount, const VkExtensionProperties* pExts)
{
    for (u32 i = 0; i < uCount; ++i)
    {
        if (strcmp(szName, pExts[i].extensionName))
        {
            return true;
        }
    }
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

static VkInstance _vkInitInstance(const char* szAppName, bool bEnableValidation, bool bEnableDebugUtilsMessager)
{
    JCHECK_RHI_RESULT(volkInitialize());

    // const char** ppInstanceLayers    = (const char**)JALLOC(2 + sizeof(char*));
    // u32          uInstanceLayerCount = 0;
    // if (bEnableValidation)
    // {
    //     ppInstanceLayers[uInstanceLayerCount++] = "VK_LAYER_KHRONOS_validation";
    // }
    // const char* pInstanceExtensionCache[256] = {};

    // 检查包含哪些layer
    u32 uLayerCount{0};
    vkEnumerateInstanceLayerProperties(&uLayerCount, nullptr);
    VkLayerProperties* pLayers = (VkLayerProperties*)JALLOC(sizeof(VkLayerProperties) * uLayerCount);
    const char** ppLayerNames = (const char**)JALLOC(sizeof(char*)*uLayerCount);
    u32 uInstanceLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&uLayerCount, pLayers);
    for (u32 i = 0; i < uLayerCount; ++i)
    {
        JLOG_INFO("VkLayerProperties {}: {}", i, pLayers[i].layerName);
    }

    // 检查包含哪些扩展
    u32 uExtCount{0};
    vkEnumerateInstanceExtensionProperties(nullptr, &uExtCount, nullptr);
    VkExtensionProperties* pExts = (VkExtensionProperties*)JALLOC(sizeof(VkExtensionProperties) * uExtCount);
    const char** ppExtensionNames = (const char**)JALLOC(sizeof(char*)*uExtCount);
    u32 uInstanceExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &uExtCount, pExts);
    for (u32 i = 0; i < uExtCount; ++i)
    {
        JLOG_INFO("VkLayerProperties {}: {}", i, pExts[i].extensionName);
    }

    if (bEnableValidation && !_vkCheckValidationLayer("VK_LAYER_KHRONOS_validation", uLayerCount, pLayers))
    {
        //TODO 这俩都应该抽成函数
        JLOG_WARN("can not find layer VK_LAYER_KHRONOS_validation!");
    }
    else
    {
        static auto aaa = "VK_LAYER_KHRONOS_validation";
        ppLayerNames[uInstanceLayerCount++] = aaa;
    }

    if (bEnableDebugUtilsMessager && !_vkCheckValidationExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, uExtCount, pExts))
    {
        JLOG_WARN("can not find layer {}", VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    else
    {
        static auto aaa = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        ppExtensionNames[uInstanceExtensionCount++] = aaa;
    }

    // 检查版本号
    u32 version;
    vkEnumerateInstanceVersion(&version);
    u32               uMajor = VK_API_VERSION_MAJOR(version);
    u32               uMinor = VK_API_VERSION_MINOR(version);
    u32               uPatch = VK_API_VERSION_PATCH(version);

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
    createInfo.pNext                   = nullptr;
    createInfo.flags                   = 0;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledLayerCount       = uInstanceLayerCount;
    createInfo.ppEnabledLayerNames     = uInstanceLayerCount?ppLayerNames:nullptr;
    createInfo.enabledExtensionCount   = uInstanceExtensionCount;
    createInfo.ppEnabledExtensionNames = uInstanceExtensionCount?ppExtensionNames:nullptr;
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
        int a = 0;
    }

    return instance;
}

RendererContext* vkInitRendererContext(const RendererContextDesc* pDesc)
{
    VkInstance hVkInstance = _vkInitInstance(pDesc->m_szAppName, pDesc->m_bEnableValidation, pDesc->m_bEnableDebugUtilsMessager);
    JASSERT(hVkInstance);
    return nullptr;
}

void vkExitRendererContext(RendererContext* pContext)
{
    VkResult a;
}

Renderer* vkInitRenderer(const RendererDesc* pDesc)
{
    return nullptr;
}

void vkExitRenderer(Renderer* pRenderer)
{
}

}