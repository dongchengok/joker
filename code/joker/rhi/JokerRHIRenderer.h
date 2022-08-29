#pragma once

namespace joker
{

struct RHIRendererDesc
{
    string szAppName;
    bool   bCPUDebug : 1;
    bool   bGPUDebug : 1;
};

class RHIRenderer
{
  public:
    virtual ~RHIRenderer()
    {
    }
};

}