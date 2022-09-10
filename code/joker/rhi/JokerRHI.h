#pragma once

#include "JokerCore.h"

#include "JokerRendererEnum.h"
#include "JokerRendererType.h"

#include "JokerRHIRenderer.h"

namespace joker
{

extern RHIRenderer* RHIInitRenderer(const RHIRendererDesc& desc);
extern void         RHIExitRenderer(RHIRenderer* pRenderer);

}