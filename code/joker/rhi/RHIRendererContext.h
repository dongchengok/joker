#pragma once

#include "RHIRendererContext.h"
#include "RHIType.h"

#include "vulkan/RHIStructVulkan.h"

#if defined(JOPTION_RHI_MULTI)
#define JRHI_STRUCT(type)
#else
#define JRHI_STRUCT(type) \
	type##Vulkan Vulkan
#endif

// struct RHIRendererContexDesc
// {
// };

// JDECL_RHI_STRUCT_BEGIN(RHIRendererContexDesc)
// JDECL_RHI_STRUCT_END()

struct RHIRendererContextDesc
{
	JRHI_STRUCT(RHIRendererContextDesc);
	bool m_bEnableGPUBasedValidation;
};