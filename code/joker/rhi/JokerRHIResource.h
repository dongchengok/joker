#pragma once

namespace joker::rhi
{

class JRHI_ALIGN Resource
{
  public:
    virtual ~Resource()
    {
    }

  protected:
    Resource()
    {
    }
    Resource(const Resource&) = delete;
};

struct DeviceDesc
{
    string szAppName;
    u32    uMaxQueueCount = 1;
    bool   bCPUDebug : 1;
    bool   bGPUDebug : 1;
};

class JRHI_ALIGN Device : public Resource
{
  public:
    const DeviceDesc&     GetDesc() const;

    virtual n32           GetGPUCount() const              = 0;
    virtual n32           GetGPUUsingIndex() const         = 0;
    virtual const string& GetGPUName(n32 idx = -1) const   = 0;
    virtual const string& GetGPUVendor(n32 idx = -1) const = 0;
    virtual const string& GetGPUModel(n32 idx = -1) const  = 0;

  public:
    void* m_HandleContext = nullptr;
    void* m_HandleDevice  = nullptr;

  protected:
    DeviceDesc m_Desc;
};

inline const DeviceDesc& Device::GetDesc() const
{
    return m_Desc;
}

struct SwapChainDesc
{
    void*       pWindow     = nullptr;
    bool        bVSync      = true;
    EColorSpace eColorSpace = EColorSpace::sRGB;
    float       fScale      = 1.f;
    u32         nImageCount = 2;
};

class JRHI_ALIGN SwapChain : public Resource
{
  public:
    const SwapChainDesc& GetDesc() const;

  public:
    void* m_HandleSwapChain = nullptr;
    void* m_HandleSurface   = nullptr;

  protected:
    SwapChainDesc* m_pDesc = nullptr;
};

inline const SwapChainDesc& SwapChain::GetDesc() const
{
    return *m_pDesc;
}

struct QueueDesc
{
    EQueueType eType;
};

class JRHI_ALIGN Queue : public Resource
{
  public:
    const QueueDesc& GetDesc() const;

  public:
    void* m_Handle;

  protected:
    QueueDesc* m_pDesc;
};

inline const QueueDesc& Queue::GetDesc() const
{
    return *m_pDesc;
}

struct CommandDesc
{
    class CommandPool* pPool;
};

struct CommandPoolDesc
{
    class Queue* pQueue;
};

class JRHI_ALIGN Command : public Resource
{
  public:
    const CommandDesc& GetDesc() const;

  public:
    void* m_Handle = nullptr;

  protected:
    CommandDesc* m_pDesc = nullptr;
};

class JRHI_ALIGN CommandPool : public Resource
{
  public:
    const CommandPoolDesc& GetDesc() const;

  public:
    void* m_Handle = nullptr;

  protected:
    CommandPoolDesc* m_pDesc = nullptr;
};

inline const CommandDesc& Command::GetDesc() const
{
    return *m_pDesc;
}

inline const CommandPoolDesc& CommandPool::GetDesc() const
{
    return *m_pDesc;
}

struct RenderPassDesc
{
    struct ColorAttachmentDesc
    {
        Texture*     pRenderTarget = nullptr;
        n32          nArraySlice   = -1;
        n8           uMipIndex     = 0;
        ELoadAction  eLoadAction   = ELoadAction::NoAction;
        EStoreAction eStoreAction  = EStoreAction::NoAction;
        // Texture*            pResolveTarget = nullptr; 自己resolve需要VkSubpassDescription2
    };
    fixed_vector<ColorAttachmentDesc, kMaxRenderTargetNum> vColorRenderTargets;

    struct DepthStencilAttachmentDesc
    {
        Texture*     pDepthStencilTarget = nullptr;
        ELoadAction  eLoadActionDepth    = ELoadAction::NoAction;
        EStoreAction eStoreActionDepth   = EStoreAction::NoAction;
        ELoadAction  eLoadActionStencil  = ELoadAction::NoAction;
        EStoreAction eStoreActionStencil = EStoreAction::NoAction;
        // Texture*                  pResolveTarget      = nullptr; 自己resolve需要VkSubpassDescription2
    };
    DepthStencilAttachmentDesc DepthStencilTarget;
    ESubpassHint               eSubpassHint;
};

struct PipelineRaytracingDesc
{
};

struct PipelineGraphicsDesc
{
};

struct PipelineComputeDesc
{
};

struct PipelineTransferDesc
{
};

struct PipelineDesc
{
};

class JRHI_ALIGN Pipeline : public Resource
{
  public:
    inline const PipelineDesc& GetDesc() const
    {
        return *m_pDesc;
    }

  public:
    void* m_Handle = nullptr;

  protected:
    PipelineDesc* m_pDesc = nullptr;
};

struct PipelineCacheDesc
{
};

class JRHI_ALIGN PipelineCache : public Resource
{
  public:
    inline const PipelineCacheDesc& GetDesc() const
    {
        return *m_pDesc;
    }

  public:
    void* m_Handle = nullptr;

  protected:
    PipelineCacheDesc* m_pDesc = nullptr;
};

union ClearColorValue {
    float fValue[4];
    n32   nValue[4];
    u32   uValue[4];
};

struct ClearDepthStencilValue
{
    float fDepth;
    u32   uStencil;
};

union ClearValue {
    ClearColorValue        Color;
    ClearDepthStencilValue DepthStencil;
};

struct TextureDesc
{
    ClearValue          ClearValue;
    ETextureCreateFlags eFlags     = ETextureCreateFlags::None;
    n32                 nWidth     = 1;
    n32                 nHeight    = 1;
    u16                 uDepth     = 1;
    u16                 uArraySize = 1;
    u8                  uNumMips   = 1;
    u8                  NumSamples = 1; // MSAA多重采样
    ETextureDimension   eDimension = ETextureDimension::Texture2D;
    EPixelFormat        eFormat    = EPixelFormat::UNKNOWN;
};

class JRHI_ALIGN Texture : public Resource
{
  public:
    inline const TextureDesc& GetDesc() const
    {
        return m_pDesc;
    }

  protected:
    TextureDesc m_pDesc;
};

}