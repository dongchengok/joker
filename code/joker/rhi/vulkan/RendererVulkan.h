#pragma once

#include "RendererTypes.h"

namespace joker::rhi::vulkan
{

struct alignas(64) vkRendererContext
{

};

struct alignas(64) vkRenderer
{

};

extern RendererContext* vkInitRendererContext(const RendererContextDesc* pDesc);
extern void vkExitRendererContext(RendererContext* pContext);
extern Renderer* vkInitRenderer(const RendererDesc* pDesc);
extern void vkExitRenderer(Renderer* pRenderer);

}