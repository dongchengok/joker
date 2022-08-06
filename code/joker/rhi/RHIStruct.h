#pragma once

#include "RHIStruct.h"

#if defined(JOPTION_RHI_NULL)
#include "null/RHIStructNull.h"
#define JDECL_RHI_PROP_NULL(type) \
	type##Null Null;
#else
#define JDECL_RHI_PROP_NULL(type)
#endif

#if defined(JOPTION_RHI_VULKAN)
#include "vulkan/RHIStructVulkan.h"
#define JDECL_RHI_PROP_VULKAN(type) \
	type##Vulkan Vulkan;
#else
#define JDECL_RHI_PROP_VULKAN(type)
#endif

#if defined(JOPTION_RHI_MULTI)
#define JDECL_RHI_STRUCT_BEGIN(type) \
	struct type                      \
	{                                \
		JDECL_RHI_PROP_NULL(type)    \
		JDECL_RHI_PROP_VULKAN(type)

#define JDECL_RHI_STRUCT_ALIGNED_BEGIN(type, a) \
	struct alignas(a) type                      \
	{                                           \
		JDECL_RHI_PROP_NULL(type)               \
		JDECL_RHI_PROP_VULKAN(type)
#define JDECL_RHI_STRUCT_END \
	}                        \
	;
#else
#endif

struct RHIGPUSettings
{
};

struct RHIGPUInfo
{
};

// JDECL_RHI_STRUCT_BEGIN(RHIRendererContextDesc)
// JDECL_RHI_STRUCT_END

// JDECL_RHI_STRUCT_ALIGNED_BEGIN(RHIRendererContext, 64)
// RHIGPUInfo* m_pGpus;
// u32			m_uGpuCount;
// JDECL_RHI_STRUCT_END
