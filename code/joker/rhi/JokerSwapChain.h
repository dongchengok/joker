#pragma once

namespace joker::rhi
{

struct SwapChainDesc
{
    void* pWindow     = nullptr;
    bool  bVSync      = true;
    bool  bHdr        = true;
    float fScale      = 1.f;
    u32   nImageCount = 2;
};

class JRHI_ALIGN SwapChain
{
  public:
    virtual ~SwapChain();

    const SwapChainDesc& GetDesc() const;

  protected:
    SwapChain(const SwapChainDesc& desc);
    SwapChain()                 = delete;
    SwapChain(const SwapChain&) = delete;

  public:
    void* m_pHWSwapChain = nullptr;
    void* m_pHWSurface   = nullptr;

  protected:
    SwapChainDesc* m_pDesc = nullptr;
};

inline const SwapChainDesc& SwapChain::GetDesc() const
{
    return *m_pDesc;
}

}