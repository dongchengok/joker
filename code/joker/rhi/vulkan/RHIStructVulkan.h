#pragma once

#include "RHIType.h"
struct RHIRendererContextDescVulkan
{
	const char** m_ppInstanceLayers;
	const char** m_ppInstanceExtensions;
	u32			 m_uInstanceLayerCount;
	u32			 m_uInstanceExtensionCount;
};

struct RHIRendererContextVulkan
{

};