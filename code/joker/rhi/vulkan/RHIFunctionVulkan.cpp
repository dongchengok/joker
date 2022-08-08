#include "JokerRHIPCH.h"
#include "CoreType.h"
#include "RHIFunction.h"
#include "RHIStruct.h"
#include "EASTL/vector.h"
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

const char* gVkWantedInstanceExtensions[] = {
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

// TODO
void util_query_gpu_settings(VkPhysicalDevice gpu, VkPhysicalDeviceProperties2* gpuProperties, VkPhysicalDeviceMemoryProperties* gpuMemoryProperties,
                             VkPhysicalDeviceFeatures2KHR* gpuFeatures, VkQueueFamilyProperties** queueFamilyProperties,
                             uint32_t* queueFamilyPropertyCount, RHIGPUSettings* gpuSettings)
{
}

JIMPL_RHI_FUNC_VK(void, RHIAddFence, RHIRenderer* pRenderer, RHIFence** ppFence)
{
}

JIMPL_RHI_FUNC_VK(void, RHIRemoveFence, RHIRenderer* pRenderer, RHIFence* pFence)
{
}

void RHIInitVulkanAPI()
{
    JREG_RHI_FUNC_VK(RHIAddFence)
    JREG_RHI_FUNC_VK(RHIRemoveFence)
}

static void* VKAPI_PTR gVkAllocation(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return tf_memalign(alignment, size);
}

static void* VKAPI_PTR
			 gVkReallocation(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return tf_realloc(pOriginal, size);
}

static void VKAPI_PTR gVkFree(void* pUserData, void* pMemory) { tf_free(pMemory); }

static void VKAPI_PTR
			gVkInternalAllocation(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{
}

static void VKAPI_PTR
			gVkInternalFree(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{
}

VkAllocationCallbacks gVkAllocationCallbacks =
{
	// pUserData
	NULL,
	// pfnAllocation
	gVkAllocation,
	// pfnReallocation
	gVkReallocation,
	// pfnFree
	gVkFree,
	// pfnInternalAllocation
	gVkInternalAllocation,
	// pfnInternalFree
	gVkInternalFree
};

void RHICreateInstance(const char* szName, const RHIRendererDesc* pDesc, uint32_t uInstanceLayerCount, const char** ppInstanceLayers,
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
        const u32                  uInitialCount       = sizeof(gVkWantedInstanceExtensions) / sizeof(gVkWantedInstanceExtensions[0]);
        const u32                  uUserRequestedCount = (u32)pDesc->Vulkan.m_uInstanceExtensionCount;
        eastl::vector<const char*> wantedInstanceExtensions(uInitialCount + uUserRequestedCount);
        for (u32 i = 0; i < uInitialCount; ++i)
        {
            wantedInstanceExtensions[i] = gVkWantedInstanceExtensions[i];
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
                vkEnumerateInstanceExtensionProperties(szLayerName, &uCount, pProperties);
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
        }
        // #if VK_HEADER_VERSION >= 108
        //         VkValidationFeaturesEXT      validationFreaturesExt     = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT};
        //         VkValidationFeatureEnableEXT enableValidationFeatures[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
        //         if (pDesc->m_bEnableGPUBasedValidation)
        //         {
        //             validationFreaturesExt.enabledValidationFeatureCount = 1;
        //             validationFreaturesExt.pEnabledValidationFeatures    = enableValidationFeatures;
        //         }
        // #endif
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.flags = 0;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.ppEnabledLayerNames = layerTemp.data();
        createInfo.enabledExtensionCount = uExtensionCount;
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
        JCHECK_VKRESULT(vkCreateInstance(&createInfo, &poolCreateInfo, &(pRenderer->Vulkan.m_vkInstance)));
    }
}

bool RHIInitCommonVK(const char* szAppName, const RHIRendererDesc* pDesc, RHIRenderer* pRenderer)
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

    RHICreateInstance(szAppName, pDesc, uInstanceLayerCount, ppInstanceLayers, pRenderer);

    pRenderer->m_uUnlinkedRenderIndex = 0;
    pRenderer->Vulkan.m_uOwnInstance  = true;
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
    if (!RHIInitCommonVK(szAppName, &fakeDesc, &fakeRenderer))
    {
        return;
    }
    RHIRendererContext* pContext            = (RHIRendererContext*)JMALLOC_ALIGNED(1, alignof(RHIRendererContext), sizeof(RHIRenderContext));
    pContext->Vulkan.m_vkInstance           = fakeRenderer.Vulkan.m_vkInstance;
    pContext->Vulkan.m_vkDebugUtilsMessager = fakeRenderer.Vulkan.m_vkDebugUtilsMessenger;

    u32 uGpuCount                           = 0;
    JCHECK_VKRESULT(vkEnumeratePhysicalDevices(pContext->Vulkan.m_vkInstance, &uGpuCount, NULL));

    VkPhysicalDevice*                 pGpus          = (VkPhysicalDevice*)JMALLOC(uGpuCount * sizeof(VkPhysicalDevice));
    VkPhysicalDeviceFeatures2*        pGuiFeatures   = (VkPhysicalDeviceFeatures2*)JMALLOC(uGpuCount * sizeof(VkPhysicalDeviceFeatures2));
    VkPhysicalDeviceProperties2*      pGpuProperties = (VkPhysicalDeviceProperties2*)JMALLOC(uGpuCount * sizeof(VkPhysicalDeviceProperties2));
    VkPhysicalDeviceMemoryProperties* pGuiProperties =
        (VkPhysicalDeviceMemoryProperties*)JMALLOC(uGpuCount * sizeof(VkPhysicalDeviceMemoryProperties));

    JCHECK_VKRESULT(vkEnumeratePhysicalDevices(pContext->Vulkan.m_vkInstance, &uGpuCount, pGpus));

    bool* bGpuValid     = (bool*)JMALLOC(uGpuCount * sizeof(bool));

    u32   uRealGpuCount = 0;
    for (u32 i = 0; i < uGpuCount; ++i)
    {
        u32                      uQueueFamilyPropertyCount = 0;
        VkQueueFamilyProperties* pQueueFamilyProperties    = nullptr;
        // util_query_gpu_settings(gpus[i], &gpuProperties[i], &gpuMemoryProperties[i], &gpuFeatures[i], &queueFamilyProperties,
        //                         &queueFamilyPropertyCount, &gpuSettings[i]);
    }
}