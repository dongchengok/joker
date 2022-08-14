#include "JokerRHIPCH.h"
#include "RendererVulkan.h"
#include "RendererInit.h"

namespace joker::rhi::vulkan
{

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

};

static bool _vkCheckContainsLayer(const char* szName, u32 uCount, const VkLayerProperties* pLayers)
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

static bool _vkCheckContainsExt(const char* szName, u32 uCount, const VkExtensionProperties* pExts)
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

static VkInstance _vkInitInstance(const char* szAppName, bool bEnableValidation)
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
    vkEnumerateInstanceLayerProperties(&uLayerCount, pLayers);
    for (u32 i = 0; i < uLayerCount; ++i)
    {
        JLOG_INFO("VkLayerProperties {}: {}", i, pLayers[i].layerName);
    }

    // 检查包含哪些扩展
    u32 uExtCount{0};
    vkEnumerateInstanceExtensionProperties(nullptr, &uExtCount, nullptr);
    VkExtensionProperties* pExts = (VkExtensionProperties*)JALLOC(sizeof(VkExtensionProperties) * uExtCount);
    vkEnumerateInstanceExtensionProperties(NULL, &uExtCount, pExts);
    for (u32 i = 0; i < uExtCount; ++i)
    {
        JLOG_INFO("VkLayerProperties {}: {}", i, pExts[i].extensionName);
    }

    if (bEnableValidation && !_vkCheckContainsLayer("VK_LAYER_KHRONOS_validation", uLayerCount, pLayers))
    {
        JLOG_WARN("can not find layer VK_LAYER_KHRONOS_validation!");
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

    return nullptr;
}

RendererContext* vkInitRendererContext(const RendererContextDesc* pDesc)
{
    VkInstance hVkInstance = _vkInitInstance(pDesc->m_szAppName, pDesc->m_bbEnableValidation);
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