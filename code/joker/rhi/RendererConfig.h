#pragma once

#if (defined(JOPTION_RHI_MULTI) && JOPTION_RHI_NULL) || (JOPTION_RHI_API == JOPTION_RHI_NULL)
#include "null/RendererNull.h"
#define JDECL_RHI_PROP_NULL(type) type##Null Null;
#else
#define JDECL_RHI_PROP_NULL(type)
#endif

#if (defined(JOPTION_RHI_MULTI) && JOPTION_RHI_VULKAN) || (JOPTION_RHI_API == JOPTION_RHI_VULKAN)
#include "vulkan/RendererVulkan.h"
#define JDECL_RHI_PROP_VULKAN(type) joker::rhi::vulkan::vk##type Vulkan;
#else
#define JDECL_RHI_PROP_VULKAN(type)
#endif

#if defined(JOPTION_RHI_MULTI)
#define JDECL_RHI_STRUCT_BEGIN(type)                                                                                                                 \
    struct alignas(64) type                                                                                                                          \
    {                                                                                                                                                \
        JDECL_RHI_PROP_NULL(type)                                                                                                                    \
        JDECL_RHI_PROP_VULKAN(type)
#define JDECL_RHI_STRUCT_END                                                                                                                         \
    }                                                                                                                                                \
    ;
#else
#if (JOPTION_RHI_API == JOPTION_RHI_NULL)
#define JDECL_RHI_PROP JDECL_RHI_PROP_NULL
#elif (JOPTION_RHI_API == JOPTION_RHI_VULKAN)
#define JDECL_RHI_PROP JDECL_RHI_PROP_VULKAN
#endif

#define JDECL_RHI_STRUCT_BEGIN(type)                                                                                                                 \
    struct alignas(64) type                                                                                                                          \
    {                                                                                                                                                \
        JDECL_RHI_PROP(type)
#define JDECL_RHI_STRUCT_END                                                                                                                         \
    }                                                                                                                                                \
    ;
#endif

#ifndef JCHECK_RHI_RESULT
#define JCHECK_RHI_RESULT(exp) JASSERT(0==(n32)(exp))
#endif