#pragma once
#include "RendererTypes.h"

namespace joker::rhi
{

extern void InitRendererContext(const RendererContextDesc* pDesc, RendererContext** ppContext);
extern void ExitRendererContext(RendererContext* pCotnext);
extern void InitRenderer(const RendererDesc* pDesc, Renderer** ppRenderer);
extern void ExitRenderer(Renderer* pRenderer);

}