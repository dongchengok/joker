#pragma once

#if defined(JOPTION_RHI_MULTI)
#define JDECL_RHI_FUNC(ret, name, ...)                                                                                                                                             \
    typedef ret (*name##Fn)(__VA_ARGS__);                                                                                                                                          \
    extern name##Fn name;
#define JIMPL_RHI_FUNC(ret, name, ...)           name##Fn name;
#define JIMPL_RHI_FUNC_API(ret, type, name, ...) ret name##type(__VA_ARGS__);
#else
#define JDECL_RHI_FUNC(ret, name, ...) extern ret name(__VA_ARGS__);
#define JIMPL_RHI_FUNC(ret, name, ...)
#endif


namespace joker
{

struct RHIRendererDesc;

class RHIRenderer;

// struct RHIRendererContextDesc;
// struct RHIRendererContext;
// struct RHIRendererDesc;
// struct RHIRenderer;
// struct RHIFence;
// struct RHIFenceStatus;
// struct RHISemaphore;
// struct RHIQueueDesc;
// struct RHIQueue;
// struct RHIQueueSubmitDesc;
// struct RHIQueuePresentDesc;
// struct RHISwapChainDesc;
// struct RHISwapChain;
// struct RHICmdPoolDesc;
// struct RHICmdPool;
// struct RHICmdDesc;
// struct RHICmd;
// struct RHIRenderTargetDesc;
// struct RHIRenderTarget;
// struct RHIRenderTargetBarrier;
// struct RHISamplerDesc;
// struct RHISampler;
// struct RHIShaderDesc;
// struct RHIShader;
// struct RHIRootSignatureDesc;
// struct RHIRootSignature;
// struct RHICommandSignatureDesc;
// struct RHICommandSignature;
// struct RHIPipelineDesc;
// struct RHIPipeline;
// struct RHIPipelineCacheDesc;
// struct RHIPipelineCache;
// struct RHIDescriptorSetDesc;
// struct RHIDescriptorSet;
// struct RHIDescriptorData;
// struct RHILoadActionsDesc;
// struct RHITexture;
// struct RHITextureBarrier;
// struct RHIBuffer;
// struct RHIBufferBarrier;
// struct RHIQueryPoolDesc;
// struct RHIQueryPool;
// struct RHIQueryDesc;

}