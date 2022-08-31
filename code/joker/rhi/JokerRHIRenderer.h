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
    virtual ~RHIRenderer();

    const RHIRendererDesc& GetDesc() const;
    void*                  GetContextHandle() const;
    void*                  GetDeviceHandle() const;

  protected:
    RHIRendererDesc m_Desc;
    void*           m_pHWContext = nullptr;
    void*           m_pHWDevice  = nullptr;
};

inline const RHIRendererDesc& RHIRenderer::GetDesc() const
{
    return m_Desc;
}

inline void* RHIRenderer::GetContextHandle() const
{
    return m_pHWContext;
}

inline void* RHIRenderer::GetDeviceHandle() const
{
    return m_pHWDevice;
}

}