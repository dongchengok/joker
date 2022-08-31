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

#ifndef JRHI_VK_CHECK
#define JRHI_VK_CHECK(exp) JASSERT(0==(n32)(exp))
#endif