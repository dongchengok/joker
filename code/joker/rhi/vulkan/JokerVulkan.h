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

namespace joker::rhi
{
    class Renderer;
}

namespace joker::rhi::vulkan
{

extern Renderer* g_pRendererVulkan;

}

#define JRHI_VK_INSTANCE (*(VkInstance*)(&joker::rhi::vulkan::g_pRendererVulkan->m_pHWContext))
#define JRHI_VK_DEVICE (*(VkDevice*)(&joker::rhi::vulkan::g_pRendererVulkan->m_pHWDevice))