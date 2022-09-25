#pragma once
#if defined(_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__ANDROID__)
#ifndef VK_USE_PLATFORM_ANDROID_KHR
#define VK_USE_PLATFORM_ANDROID_KHR
#endif
#endif

#include <volk.h>

#if defined(VK_KHR_ray_tracing_pipeline) && defined(VK_KHR_acceleration_structure)
#define VK_RAYTRACING_AVAILABLE
#endif

//应vma要求VK_USE_PLATFORM_WIN32_KHR VK_NO_PROTOTYPES 需要定义在引入他前面,所以先引用vulkan
#include <vk_mem_alloc.h>

#ifndef JRHI_VK_CHECK
#define JRHI_VK_CHECK(exp) JASSERT(0 == (n32)(exp))
#endif

#ifndef JRHI_SDL_CHECK
#define JRHI_SDL_CHECK(exp)                                                                                                                                                        \
    if (exp == 0)                                                                                                                                                                  \
    {                                                                                                                                                                              \
        JLOG_CRITICAL(SDL_GetError());                                                                                                                                             \
        JASSERT(0);                                                                                                                                                                \
    }
#endif

#if defined(JOPTION_RHI_MULTI)
#else
#define JRHI_IMPL_FUNC_VK(ret, name, ...) JRHI_IMPL_FUNC_API(ret, VK, name, __VA_ARGS__)
#endif

namespace joker::rhi::vulkan
{

extern class DeviceVulkan*             g_pRendererVulkan;

constexpr inline VkSampleCountFlagBits ToSampleVK(joker::rhi::ESampleCount n)
{
    return (VkSampleCountFlagBits)n;
}

constexpr inline VkFormat ToFormatVK(EPixelFormat eFormat)
{
    return (VkFormat)eFormat;
}

constexpr inline VkAttachmentLoadOp ToLoadOpVK(ELoadAction eLoad)
{
    return (VkAttachmentLoadOp)eLoad;
}

constexpr inline VkAttachmentStoreOp ToStoreOpVK(EStoreAction eStore)
{
    return (VkAttachmentStoreOp)eStore;
}

constexpr inline VkImageLayout ToDepthStencilImageLayoutVK(bool bDepthWriteable, bool bStencilWriteable)
{
    return (bDepthWriteable || bStencilWriteable) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
}

}

#define JRHI_VK_INSTANCE ((VkInstance)joker::rhi::vulkan::g_pRendererVulkan->m_HandleContext)
#define JRHI_VK_DEVICE   ((VkDevice)joker::rhi::vulkan::g_pRendererVulkan->m_HandleDevice)
#define JRHI_VK_GPU      (joker::rhi::vulkan::g_pRendererVulkan->m_hVkActiveDevice)
#define JRHI_VK_ALLOC    (g_pRendererVulkan->m_pVkAllocationCallbacks)

// constexpr inline VkFormat JRHI_VK_FORMAT()