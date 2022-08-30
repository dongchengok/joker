#pragma once

namespace joker
{

struct RHIRendererDesc
{
    string szAppName;
    bool   bCPUDebug : 1;
    bool   bGPUDebug : 1;
};

class JRHI_ALIGN RHIRenderer
{
  public:
    virtual ~RHIRenderer()
    {
    }

  public:
    void* m_pHWContext = nullptr;
    void* m_pHWDevice  = nullptr;
};

}