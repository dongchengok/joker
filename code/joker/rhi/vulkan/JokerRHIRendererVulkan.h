#pragma once

#include "JokerRHIRenderer.h"

namespace joker
{

class RHIRendererVulkan : public RHIRenderer
{
  public:
    RHIRendererVulkan();
    virtual ~RHIRendererVulkan();
};

extern RHIRenderer* RHIInitRendererVulkan(const RHIRendererDesc& desc);
extern void         RHIExitRendererVulkan(RHIRenderer* pRenderer);

}