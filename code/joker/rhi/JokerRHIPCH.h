#pragma once

#include <JokerCore.h>
#include "vulkan/RendererConfigVulkan.h"
//应vma要求VK_USE_PLATFORM_WIN32_KHR VK_NO_PROTOTYPES 需要定义在引入他前面,所以先引用vulkan
#include <vk_mem_alloc.h>
#include "RHIEnum.h"
#include "JokerRHIType.h"
#include "vulkan/JokerRHIVulkan.h"