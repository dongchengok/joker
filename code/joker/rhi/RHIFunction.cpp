#include "JokerRHIPCH.h"
#include "RHIFunction.h"
#include "RHIEnum.h"
#include "RHIStruct.h"

extern void RHIInitRendererContextVK();
extern void RHIInitRendererContextNULL();

void        RHIInitRendererContext(const char* szAppName, const RHIRendererContextDesc* pSettings, RHIRendererContext** ppContext)
{
    JASSERT(szAppName);
    JASSERT(pSettings);
    JASSERT(ppContext);
    switch (pSettings->m_eRenderer)
    {
    case ERHIRenderer::Null:
        break;
    case ERHIRenderer::Vulkan:
        RHIInitRendererContextVK();
        break;
    }
}

void RHIExitRendererContext(RHIRendererContext* pContext)
{
}

void RHIInitRenderer(const char* szAppName, const RHIRendererDesc* pSettings, RHIRenderer* pRenderer)
{
}

void RHIExitRenderer(RHIRenderer* pRenderer)
{
}

JIMPL_RHI_FUNC(void, RHIAddFence, RHIRenderer* pRenderer, RHIFence** ppFence)
JIMPL_RHI_FUNC(void, RHIRemoveFence, RHIRenderer* pRenderer, RHIFence* pFence)

JIMPL_RHI_FUNC(void, RHIAddSemaphore, RHIRenderer* pRenderer, RHISemaphore** ppSemaphore)
JIMPL_RHI_FUNC(void, RHIRemoveSemaphore, RHIRenderer* pRenderer, RHISemaphore* pSemaphore)

JIMPL_RHI_FUNC(void, RHIAddQueue, RHIRenderer* pRenderer, const RHIQueueDesc* pDesc, RHIQueue** ppQueue)
JIMPL_RHI_FUNC(void, RHIRemoveQueue, RHIRenderer* pRenderer, RHIQueue* pQueue)

JIMPL_RHI_FUNC(void, RHIAddSwapChain, RHIRenderer* pRenderer, const RHISwapChainDesc* pDesc, RHISwapChain** ppSwapChain)
JIMPL_RHI_FUNC(void, RHIRemoveSwapChain, RHIRenderer* pRenderer, RHISwapChain* pSwapChain)

JIMPL_RHI_FUNC(void, RHIAddCmdPool, RHIRenderer* pRenderer, const RHICmdPoolDesc* pDesc, RHICmdPool** ppCmdPool)
JIMPL_RHI_FUNC(void, RHIRemoveCmdPool, RHIRenderer* pRenderer, const RHICmdPool* pCmdPool)
JIMPL_RHI_FUNC(void, RHIAddCmd, RHIRenderer* pRender, const RHICmdDesc* pDesc, RHICmd** ppCmd)
JIMPL_RHI_FUNC(void, RHIRemoveCmd, RHIRenderer* pRenderer, RHICmd* pCmd)
JIMPL_RHI_FUNC(void, RHIAddCmdArray, RHIRenderer* pRenderer, RHICmdDesc* pDesc, u32 uCmdCount, RHICmd*** pppCmds)
JIMPL_RHI_FUNC(void, RHIRemoveCmdArray, RHIRenderer* pRenderer, u32 uCmdCount, RHICmd** ppCmds)

JIMPL_RHI_FUNC(void, RHIAddRenderTarget, RHIRenderer* pRenderer, const RHIRenderTargetDesc* pDesc, RHIRenderTarget** ppRenderTarget)
JIMPL_RHI_FUNC(void, RHIRemoveRenderTarget, RHIRenderer* pRenderer, const RHIRenderTarget* pRenderTarget)
JIMPL_RHI_FUNC(void, RHIAddSampler, RHIRenderer* pRenderer, const RHISamplerDesc* pDesc, RHISampler* ppSampler)
JIMPL_RHI_FUNC(void, RHIRemoveSampler, RHIRenderer* pRenderer, RHISampler* pSampler)

JIMPL_RHI_FUNC(void, RHIAddShader, RHIRenderer* pRenderer, const RHIShaderDesc* pDesc, RHIShader** ppShader)
JIMPL_RHI_FUNC(void, RHIRemoveShader, RHIRenderer* pRenderer, RHIShader* pShader)

JIMPL_RHI_FUNC(void, RHIAddRootSignature, RHIRenderer* pRenderer, RHIRootSignatureDesc* pDesc, RHIRootSignature** ppRootSignature)
JIMPL_RHI_FUNC(void, RHIRemoveRootSignature, RHIRenderer* pRenderer, RHIRootSignature* pRootSignature)

JIMPL_RHI_FUNC(void, RHIAddPipeline, RHIRenderer* pRenderer, const RHIPipelineDesc* pDesc, RHIPipeline* ppPipeline)
JIMPL_RHI_FUNC(void, RHIRemovePipeline, RHIRenderer* pRenderer, RHIPipeline* pPipeline)
JIMPL_RHI_FUNC(void, RHIAddPipelineCache, RHIRenderer* pRenderer, const RHIPipelineCacheDesc* pDesc, RHIPipelineCache*** pppPipelineCache)
JIMPL_RHI_FUNC(void, RHIGetPipelineCache, RHIRenderer* pRenderer, RHIPipelineCache* pPipelineCache, size_t* pSize, void* pData)
JIMPL_RHI_FUNC(void, RHIRemovePipelineCache, RHIRenderer* pRenderer, RHIPipelineCache* pPipelineCache)

JIMPL_RHI_FUNC(void, RHIAddDescriptorSet, RHIRenderer* pRenderer, const RHIDescriptorSetDesc* pDesc, RHIDescriptorSet** ppDescriptorSet)
JIMPL_RHI_FUNC(void, RHIRemoveDescriptorSet, RHIRenderer* pRenderer, RHIDescriptorSet* pDescriptorSet)
JIMPL_RHI_FUNC(void, RHIUpdateDescriptorSet, RHIRenderer* pRenderer, u32 uIndex, RHIDescriptorSet* pDescriptorSet, u32 uCount,
               const RHIDescriptorData* pParams)

JIMPL_RHI_FUNC(void, RHICmdResetPool, RHIRenderer* pRenderer, RHICmdPool* pCmdPool)
JIMPL_RHI_FUNC(void, RHICmdBegin, RHICmd* pCmd)
JIMPL_RHI_FUNC(void, RHICmdEnd, RHICmd* pCmd)
JIMPL_RHI_FUNC(void, RHICmdBindRenderTargets, RHICmd* pCmd, u32 renderTargetCount, RHIRenderTarget** ppRenderTargets, RHIRenderTarget* pDepthStencil,
               const RHILoadActionsDesc* pLoadActions, u32 pColorArraySlices, u32* pColorMipSlices, u32 uDepthArraySlice, u32 uDepthMipSlice)
JIMPL_RHI_FUNC(void, RHICmdSetShadingRate, RHICmd* pCmd, EShadingRate eShadingRate, RHITexture* pTexture, EShadingRateCombiner ePostRasterizerRate,
               EShadingRateCombiner* eFinalRate)
JIMPL_RHI_FUNC(void, RHICmdSetViewport, RHICmd* pCmd, f32 x, f32 y, f32 width, f32 height, f32 minDepth, f32 maxDepth)
JIMPL_RHI_FUNC(void, RHICmdSetScissor, RHICmd* pCmd, u32 x, u32 y, u32 width, u32 height)
JIMPL_RHI_FUNC(void, RHICmdSetStencilReferenceValue, RHICmd* pCmd, u32 uVal)
JIMPL_RHI_FUNC(void, RHICmdBindPipeline, RHICmd* pCmd, RHIPipeline* pPipeline)
JIMPL_RHI_FUNC(void, RHICmdBindDescriptorSet, RHICmd* pCmd, u32 uIndex, RHIDescriptorSet* pDescriptorSet)
JIMPL_RHI_FUNC(void, RHICmdBindPushConstants, RHICmd* pCmd, RHIRootSignature* pRootSignature, u32 uParamIndex, const void* pConstants)
JIMPL_RHI_FUNC(void, RHICmdBindDescriptorSetWithRootCbvs, RHICmd* pCmd, u32 uIndex, RHIDescriptorSet* pDescriptorSet, u32 uCount,
               const RHIDescriptorData* pParams)
JIMPL_RHI_FUNC(void, RHICmdBindIndexBuffer, RHICmd* pCmd, RHIBuffer* pBuffer, u32 uIndexType, u64 uOffset)
JIMPL_RHI_FUNC(void, RHICmdBindVertexBuffer, RHICmd* pCmd, u32 uBufferCount, RHIBuffer** ppBuffers, const u32* pStrides, const u64 uOffsets)
JIMPL_RHI_FUNC(void, RHICmdDraw, RHICmd* pCmd, u32 uVertexCount, u32 uFirstVertex)
JIMPL_RHI_FUNC(void, RHICmdDrawInstanced, RHICmd* pCmd, u32 uVertexCount, u32 uFirstVertex, u32 uInstanceCount, u32 uFirstInstance)
JIMPL_RHI_FUNC(void, RHICmdDrawIndexed, RHICmd* pCmd, u32 uIndexCount, u32 uFirstIndex, u32 uFirstVertex)
JIMPL_RHI_FUNC(void, RHICmdDrawIndexedInstanced, RHICmd* pCmd, u32 uIndexCount, u32 uFirstIndex, u32 uInstanceCount, u32 uFirstVertex,
               u32 uFirstInstance)
JIMPL_RHI_FUNC(void, RHICmdDispatch, RHICmd* pCmd, u32 uGroupCountX, u32 uGroupCountY, u32 uGroupCountZ)

JIMPL_RHI_FUNC(void, RHICmdResourceBarrier, RHICmd* pCmd, u32 uBufferBarrierCount, RHIBufferBarrier* pBufferBarriers, u32 uTextureBarrierCount,
               RHITextureBarrier* pTextureBarriers, u32 uRTBarrierCount, RHIRenderTargetBarrier* pRTBarriers)

JIMPL_RHI_FUNC(void, RHICmdUpdateVirtualTexture, RHICmd* pCmd, RHITexture* pTexture, u32 uCurrentImage)

JIMPL_RHI_FUNC(void, RHIAcquireNextImage, RHIRenderer* pRenderer, RHISwapChain* pSwapChain, RHISemaphore* pSignalSemaphore, RHIFence* pFence,
               u32 pImageIndex)
JIMPL_RHI_FUNC(void, RHIQueueSubmit, RHIQueue* pQueue, const RHIQueueSubmitDesc* pDesc)
JIMPL_RHI_FUNC(void, RHIQueuePresent, RHIQueue* pQueue, const RHIQueuePresentDesc* pDesc)
JIMPL_RHI_FUNC(void, RHIQueueWaitIdle, RHIQueue* pQueue)
JIMPL_RHI_FUNC(void, RHIGetFenceStatus, RHIRenderer* pRenderer, RHIFence* pFence, RHIFenceStatus* pFenceStatus)
JIMPL_RHI_FUNC(void, RHIWaitForFences, RHIRenderer* pRenderer, u32 uFenceCount, RHIFence** ppFences)
JIMPL_RHI_FUNC(void, RHIToggleVSync, RHIRenderer* pRenderer, RHISwapChain** ppSwapChain)

// Returns the recommended format for the swapchain.
// If true is passed for the hintHDR parameter, it will return an HDR format IF the platform supports it
// If false is passed or the platform does not support HDR a non HDR format is returned.
// If true is passed for the hintSrgb parameter, it will return format that is will do gamma correction automatically
// If false is passed for the hintSrgb parameter the gamma correction should be done as a postprocess step before submitting image to swapchain
//  JIMPL_RHI_FUNC(EImageFormat, RHIGetRecommendedSwapchainFormat, bool bHDR, bool bSRGB)

JIMPL_RHI_FUNC(void, RHIAddIndirectCommandSignature, RHIRenderer* pRender, const RHICommandSignatureDesc* pDesc,
               RHICommandSignature** ppCommandSignature)
JIMPL_RHI_FUNC(void, RHIRemoveIndirectCommandSignature, RHIRenderer* pRenderer, RHICommandSignature* pCommandSignature)
JIMPL_RHI_FUNC(void, RHICmdExecuteIndirect, RHICmd* pCmd, RHICommandSignature* pCommandSignature, u32 uMaxCommandCount, RHIBuffer* pIndirectBuffer,
               u64 uBufferOffset, RHIBuffer* pCounterBuffer, u64 uCounterBufferOffset)

/************************************************************************/
// GPU Query Interface
/************************************************************************/
JIMPL_RHI_FUNC(void, RHIGetTimestampFrequency, RHIQueue* pQueue, f64* pFrequency)
JIMPL_RHI_FUNC(void, RHIAddQueryPool, RHIRenderer* pRenderer, const RHIQueryPoolDesc* pDesc, RHIQueryPool** ppQueryPool)
JIMPL_RHI_FUNC(void, RHIRemoveQueryPool, RHIRenderer* pRenderer, RHIQueryPool* pQueryPool)
JIMPL_RHI_FUNC(void, RHIResetQueryPool, RHICmd* pCmd, RHIQueryPool* pQueryPool, u32 uStartQuery, u32 uQueryCount)
JIMPL_RHI_FUNC(void, RHICmdBeginQuery, RHICmd* pCmd, RHIQueryPool* pQueryPool, RHIQueryDesc* pQuery)
JIMPL_RHI_FUNC(void, RHICmdEndQuery, RHICmd* pCmd, RHIQueryPool* pQueryPool, RHIQueryDesc* pQuery)
JIMPL_RHI_FUNC(void, RHICmdResolveQuery, RHICmd* pCmd, RHIQueryPool* pQueryPool, RHIBuffer* pReadbackBuffer, u32 uStartQuery, u32 uQueryCount)

/************************************************************************/
// Stats Info Interface
/************************************************************************/
JIMPL_RHI_FUNC(void, RHICalculateMemoryStats, RHIRenderer* pRenderer, char** ppStatus)
JIMPL_RHI_FUNC(void, RHICalculateMemoryUse, RHIRenderer* pRenderer, u64* pUsedBytes, u64* pTotalAllocatedBytes)
JIMPL_RHI_FUNC(void, RHIFreeMemoryStats, RHIRenderer* pRenderer, char* stats)

/************************************************************************/
// Debug Marker Interface
/************************************************************************/
JIMPL_RHI_FUNC(void, RHICmdBeginDebugMarker, RHICmd* pCmd, f32 r, f32 g, f32 b, const char* pName)
JIMPL_RHI_FUNC(void, RHICmdEndDebugMarker, RHICmd* pCmd)
JIMPL_RHI_FUNC(void, RHICmdAddDebugMarker, RHICmd* pCmd, f32 r, f32 g, f32 b, const char* pName)
JIMPL_RHI_FUNC(u32, RHICmdWriteMarker, RHICmd* pCmd, EMarkerType eMarkerType, u32 uMarkerValue, RHIBuffer* pBuffer, size_t uOffset,
               bool bUseAutoFlags)

/************************************************************************/
// Resource Debug Naming Interface
/************************************************************************/
JIMPL_RHI_FUNC(void, RHISetBufferName, RHIRenderer* pRenderer, RHIBuffer* pBuffer, const char* pName)
JIMPL_RHI_FUNC(void, RHISetTextureName, RHIRenderer* pRenderer, RHITexture* pTexture, const char* pName)
JIMPL_RHI_FUNC(void, RHISetRenderTargetName, RHIRenderer* pRenderer, RHIRenderTarget* pRenderTarget, const char* pName)
JIMPL_RHI_FUNC(void, RHISetPipelineName, RHIRenderer* pRenderer, RHIPipeline* pPipeline, const char* pName)
