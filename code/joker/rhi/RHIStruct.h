#pragma once

#include "RHIEnum.h"
#if (defined(JOPTION_RHI_MULTI) && JOPTION_RHI_NULL) || (JOPTION_RHI_API == JOPTION_RHI_NULL)
#include "null/RHIStructNull.h"
#define JDECL_RHI_PROP_NULL(type) type##Null Null;
#else
#define JDECL_RHI_PROP_NULL(type)
#endif

#if (defined(JOPTION_RHI_MULTI) && JOPTION_RHI_VULKAN) || (JOPTION_RHI_API == JOPTION_RHI_VULKAN)
#include "vulkan/RHIStructVulkan.h"
#define JDECL_RHI_PROP_VULKAN(type) type##Vulkan Vulkan;
#else
#define JDECL_RHI_PROP_VULKAN(type)
#endif

#if defined(JOPTION_RHI_MULTI)
#define JDECL_RHI_STRUCT_BEGIN(type)                                                                                                                 \
    struct type                                                                                                                                      \
    {                                                                                                                                                \
        JDECL_RHI_PROP_NULL(type)                                                                                                                    \
        JDECL_RHI_PROP_VULKAN(type)

#define JDECL_RHI_STRUCT_ALIGNED_BEGIN(type, a)                                                                                                      \
    struct alignas(a) type                                                                                                                           \
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
    struct type                                                                                                                                      \
    {                                                                                                                                                \
        JDECL_RHI_PROP(type)

#define JDECL_RHI_STRUCT_ALIGNED_BEGIN(type, a)                                                                                                      \
    struct alignas(a) type                                                                                                                           \
    {                                                                                                                                                \
        JDECL_RHI_PROP(type)
#define JDECL_RHI_STRUCT_END                                                                                                                         \
    }                                                                                                                                                \
    ;
#endif

struct RHIGPUSettings
{
};

struct RHIGPUInfo
{
};

JDECL_RHI_STRUCT_BEGIN(RHIRendererContextDesc)
ERHIRenderer m_eRenderer;
bool         m_bEnableGPUBasedValidation;
JDECL_RHI_STRUCT_END

JDECL_RHI_STRUCT_ALIGNED_BEGIN(RHIRendererContext, 64)
RHIGPUInfo* m_pGpus;
u32         m_uGpuCount;
JDECL_RHI_STRUCT_END

JDECL_RHI_STRUCT_BEGIN(RHIRendererDesc)
//这是个基于GPU的验证，很慢，会打补丁到shader里
bool m_bEnableGPUBasedValidation;
JDECL_RHI_STRUCT_END

JDECL_RHI_STRUCT_ALIGNED_BEGIN(RHIRenderer, 64)
JDECL_RHI_STRUCT_END

JDECL_RHI_STRUCT_BEGIN(RHICommandSignature)
JDECL_RHI_STRUCT_END