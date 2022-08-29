#pragma once

#include "JokerCore.h"
#include "RendererEnums.h"
#include "RendererInit.h"

#include "JokerRHIType.h"

namespace joker
{

extern RHIRenderer* RHIInitRenderer(const RHIRendererDesc& desc);
extern void         RHIExitRenderer(RHIRenderer* pRenderer);

}