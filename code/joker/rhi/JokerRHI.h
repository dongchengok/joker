#pragma once

#include "JokerCore.h"

#include "JokerRendererEnum.h"
#include "JokerRendererType.h"

#include "JokerRenderer.h"

namespace joker::rhi
{

extern Renderer* InitRenderer(const RendererDesc& desc);
extern void      ExitRenderer(Renderer* pRenderer);

}