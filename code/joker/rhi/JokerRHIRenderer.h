#pragma once

namespace joker
{

struct RHIRendererDesc
{
    string szAppName;
    u32    uMaxQueueCount = 8;
    bool   bUseAllQueue : 1;
    bool   bCPUDebug    : 1;
    bool   bGPUDebug    : 1;
};

class JRHI_ALIGN RHIRenderer
{
  public:
    virtual ~RHIRenderer();

    const RHIRendererDesc& GetDesc() const;

    virtual n32            GetGPUCount() const              = 0;
    virtual n32            GetGPUUsingIndex() const         = 0;
    virtual const string&  GetGPUName(n32 idx = -1) const   = 0;
    virtual const string&  GetGPUVendor(n32 idx = -1) const = 0;
    virtual const string&  GetGPUModel(n32 idx = -1) const  = 0;

  protected:
    RHIRenderer(const RHIRendererDesc& desc);
    RHIRenderer() = delete;

  protected:
    RHIRendererDesc m_Desc;

  public:
    void* m_pHWContext = nullptr;
    void* m_pHWDevice  = nullptr;
};

inline const RHIRendererDesc& RHIRenderer::GetDesc() const
{
    return m_Desc;
}

}