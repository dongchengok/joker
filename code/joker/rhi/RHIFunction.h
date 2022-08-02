#pragma once

#include "CoreType.h"
#include "RHIFunction.h"
#include <vcruntime.h>
#if defined(JOPTION_RHI_MULTI)
#define JDECL_RHI_FUNC(ret, name, ...)    \
	typedef ret (*name##Fn)(__VA_ARGS__); \
	extern name##Fn name;
#define JIMPL_RHI_FUNC(ret, name, ...) \
	name##Fn name;
#else
#define JDECL_RHI_FUNC(ret, name, ...) \
	extern ret name(__VA_ARGS__);
#define JIMPL_RHI_FUNC(ret, name, ...) \
	ret name(__VA_ARGS__);
#endif

struct RHIRenderer;
struct RHIFence;
struct RHIFenceStatus;
struct RHISemaphore;
struct RHIQueueDesc;
struct RHIQueue;
struct RHIQueueSubmitDesc;
struct RHIQueuePresentDesc;
struct RHISwapChainDesc;
struct RHISwapChain;
struct RHICmdPoolDesc;
struct RHICmdPool;
struct RHICmdDesc;
struct RHICmd;
struct RHIRenderTargetDesc;
struct RHIRenderTarget;
struct RHIRenderTargetBarrier;
struct RHISamplerDesc;
struct RHISampler;
struct RHIShaderDesc;
struct RHIShader;
struct RHIRootSignatureDesc;
struct RHIRootSignature;
struct RHICommandSignatureDesc;
struct RHICommandSignature;
struct RHIPipelineDesc;
struct RHIPipeline;
struct RHIPipelineCacheDesc;
struct RHIPipelineCache;
struct RHIDescriptorSetDesc;
struct RHIDescriptorSet;
struct RHIDescriptorData;
struct RHILoadActionsDesc;
struct RHITexture;
struct RHITextureBarrier;
struct RHIBuffer;
struct RHIBufferBarrier;
struct RHIQueryPoolDesc;
struct RHIQueryPool;
struct RHIQueryDesc;

enum class EShadingRate
{

};

enum class EShadingRateCombiner
{

};

enum class EMarkerType
{

};

JDECL_RHI_FUNC(void, RHIAddFence, RHIRenderer* pRenderer, RHIFence** ppFence)
JDECL_RHI_FUNC(void, RHIRemoveFence, RHIRenderer* pRenderer, RHIFence* pFence)

JDECL_RHI_FUNC(void, RHIAddSemaphore, RHIRenderer* pRenderer, RHISemaphore** ppSemaphore)
JDECL_RHI_FUNC(void, RHIRemoveSemaphore, RHIRenderer* pRenderer, RHISemaphore* pSemaphore)

JDECL_RHI_FUNC(void, RHIAddQueue, RHIRenderer* pRenderer, const RHIQueueDesc* pDesc, RHIQueue** ppQueue)
JDECL_RHI_FUNC(void, RHIRemoveQueue, RHIRenderer* pRenderer, RHIQueue* pQueue)

JDECL_RHI_FUNC(void, RHIAddSwapChain, RHIRenderer* pRenderer, const RHISwapChainDesc* pDesc, RHISwapChain** ppSwapChain)
JDECL_RHI_FUNC(void, RHIRemoveSwapChain, RHIRenderer* pRenderer, RHISwapChain* pSwapChain)

JDECL_RHI_FUNC(void, RHIAddCmdPool, RHIRenderer* pRenderer, const RHICmdPoolDesc* pDesc, RHICmdPool** ppCmdPool)
JDECL_RHI_FUNC(void, RHIRemoveCmdPool, RHIRenderer* pRenderer, const RHICmdPool* pCmdPool)
JDECL_RHI_FUNC(void, RHIAddCmd, RHIRenderer* pRender, const RHICmdDesc* pDesc, RHICmd** ppCmd)
JDECL_RHI_FUNC(void, RHIRemoveCmd, RHIRenderer* pRenderer, RHICmd* pCmd)
JDECL_RHI_FUNC(void, RHIAddCmdArray, RHIRenderer* pRenderer, RHICmdDesc* pDesc, u32 uCmdCount, RHICmd*** pppCmds)
JDECL_RHI_FUNC(void, RHIRemoveCmdArray, RHIRenderer* pRenderer, u32 uCmdCount, RHICmd** ppCmds)

JDECL_RHI_FUNC(void, RHIAddRenderTarget, RHIRenderer* pRenderer, const RHIRenderTargetDesc* pDesc, RHIRenderTarget** ppRenderTarget)
JDECL_RHI_FUNC(void, RHIRemoveRenderTarget, RHIRenderer* pRenderer, const RHIRenderTarget* pRenderTarget)
JDECL_RHI_FUNC(void, RHIAddSampler, RHIRenderer* pRenderer, const RHISamplerDesc* pDesc, RHISampler* ppSampler)
JDECL_RHI_FUNC(void, RHIRemoveSampler, RHIRenderer* pRenderer, RHISampler* pSampler)

JDECL_RHI_FUNC(void, RHIAddShader, RHIRenderer* pRenderer, const RHIShaderDesc* pDesc, RHIShader** ppShader)
JDECL_RHI_FUNC(void, RHIRemoveShader, RHIRenderer* pRenderer, RHIShader* pShader)

JDECL_RHI_FUNC(void, RHIAddRootSignature, RHIRenderer* pRenderer, RHIRootSignatureDesc* pDesc, RHIRootSignature** ppRootSignature)
JDECL_RHI_FUNC(void, RHIRemoveRootSignature, RHIRenderer* pRenderer, RHIRootSignature* pRootSignature)

JDECL_RHI_FUNC(void, RHIAddPipeline, RHIRenderer* pRenderer, const RHIPipelineDesc* pDesc, RHIPipeline* ppPipeline)
JDECL_RHI_FUNC(void, RHIRemovePipeline, RHIRenderer* pRenderer, RHIPipeline* pPipeline)
JDECL_RHI_FUNC(void, RHIAddPipelineCache, RHIRenderer* pRenderer, const RHIPipelineCacheDesc* pDesc, RHIPipelineCache*** pppPipelineCache)
JDECL_RHI_FUNC(void, RHIGetPipelineCache, RHIRenderer* pRenderer, RHIPipelineCache* pPipelineCache, size_t* pSize, void* pData)
JDECL_RHI_FUNC(void, RHIRemovePipelineCache, RHIRenderer* pRenderer, RHIPipelineCache* pPipelineCache)

JDECL_RHI_FUNC(void, RHIAddDescriptorSet, RHIRenderer* pRenderer, const RHIDescriptorSetDesc* pDesc, RHIDescriptorSet** ppDescriptorSet)
JDECL_RHI_FUNC(void, RHIRemoveDescriptorSet, RHIRenderer* pRenderer, RHIDescriptorSet* pDescriptorSet)
JDECL_RHI_FUNC(void, RHIUpdateDescriptorSet, RHIRenderer* pRenderer, u32 uIndex, RHIDescriptorSet* pDescriptorSet, u32 uCount, const RHIDescriptorData* pParams)

JDECL_RHI_FUNC(void, RHICmdResetPool, RHIRenderer* pRenderer, RHICmdPool* pCmdPool)
JDECL_RHI_FUNC(void, RHICmdBegin, RHICmd* pCmd)
JDECL_RHI_FUNC(void, RHICmdEnd, RHICmd* pCmd)
JDECL_RHI_FUNC(void, RHICmdBindRenderTargets, RHICmd* pCmd, u32 renderTargetCount, RHIRenderTarget** ppRenderTargets, RHIRenderTarget* pDepthStencil, const RHILoadActionsDesc* pLoadActions, u32 pColorArraySlices, u32* pColorMipSlices, u32 uDepthArraySlice, u32 uDepthMipSlice)
JDECL_RHI_FUNC(void, RHICmdSetShadingRate, RHICmd* pCmd, EShadingRate eShadingRate, RHITexture* pTexture, EShadingRateCombiner ePostRasterizerRate, EShadingRateCombiner* eFinalRate)
JDECL_RHI_FUNC(void, RHICmdSetViewport, RHICmd* pCmd, f32 x, f32 y, f32 width, f32 height, f32 minDepth, f32 maxDepth)
JDECL_RHI_FUNC(void, RHICmdSetScissor, RHICmd* pCmd, u32 x, u32 y, u32 width, u32 height)
JDECL_RHI_FUNC(void, RHICmdSetStencilReferenceValue, RHICmd* pCmd, u32 uVal)
JDECL_RHI_FUNC(void, RHICmdBindPipeline, RHICmd* pCmd, RHIPipeline* pPipeline)
JDECL_RHI_FUNC(void, RHICmdBindDescriptorSet, RHICmd* pCmd, u32 uIndex, RHIDescriptorSet* pDescriptorSet)
JDECL_RHI_FUNC(void, RHICmdBindPushConstants, RHICmd* pCmd, RHIRootSignature* pRootSignature, u32 uParamIndex, const void* pConstants)
JDECL_RHI_FUNC(void, RHICmdBindDescriptorSetWithRootCbvs, RHICmd* pCmd, u32 uIndex, RHIDescriptorSet* pDescriptorSet, u32 uCount, const RHIDescriptorData* pParams)
JDECL_RHI_FUNC(void, RHICmdBindIndexBuffer, RHICmd* pCmd, RHIBuffer* pBuffer, u32 uIndexType, u64 uOffset)
JDECL_RHI_FUNC(void, RHICmdBindVertexBuffer, RHICmd* pCmd, u32 uBufferCount, RHIBuffer** ppBuffers, const u32* pStrides, const u64 uOffsets)
JDECL_RHI_FUNC(void, RHICmdDraw, RHICmd* pCmd, u32 uVertexCount, u32 uFirstVertex)
JDECL_RHI_FUNC(void, RHICmdDrawInstanced, RHICmd* pCmd, u32 uVertexCount, u32 uFirstVertex, u32 uInstanceCount, u32 uFirstInstance)
JDECL_RHI_FUNC(void, RHICmdDrawIndexed, RHICmd* pCmd, u32 uIndexCount, u32 uFirstIndex, u32 uFirstVertex)
JDECL_RHI_FUNC(void, RHICmdDrawIndexedInstanced, RHICmd* pCmd, u32 uIndexCount, u32 uFirstIndex, u32 uInstanceCount, u32 uFirstVertex, u32 uFirstInstance)
JDECL_RHI_FUNC(void, RHICmdDispatch, RHICmd* pCmd, u32 uGroupCountX, u32 uGroupCountY, u32 uGroupCountZ)

JDECL_RHI_FUNC(void, RHICmdResourceBarrier, RHICmd* pCmd, u32 uBufferBarrierCount, RHIBufferBarrier* pBufferBarriers, u32 uTextureBarrierCount, RHITextureBarrier* pTextureBarriers, u32 uRTBarrierCount, RHIRenderTargetBarrier* pRTBarriers)

JDECL_RHI_FUNC(void, RHICmdUpdateVirtualTexture, RHICmd* pCmd, RHITexture* pTexture, u32 uCurrentImage)

JDECL_RHI_FUNC(void, RHIAcquireNextImage, RHIRenderer* pRenderer, RHISwapChain* pSwapChain, RHISemaphore* pSignalSemaphore, RHIFence* pFence, u32 pImageIndex)
JDECL_RHI_FUNC(void, RHIQueueSubmit, RHIQueue* pQueue, const RHIQueueSubmitDesc* pDesc)
JDECL_RHI_FUNC(void, RHIQueuePresent, RHIQueue* pQueue, const RHIQueuePresentDesc* pDesc)
JDECL_RHI_FUNC(void, RHIQueueWaitIdle, RHIQueue* pQueue)
JDECL_RHI_FUNC(void, RHIGetFenceStatus, RHIRenderer* pRenderer, RHIFence* pFence, RHIFenceStatus* pFenceStatus)
JDECL_RHI_FUNC(void, RHIWaitForFences, RHIRenderer* pRenderer, u32 uFenceCount, RHIFence** ppFences)
JDECL_RHI_FUNC(void, RHIToggleVSync, RHIRenderer* pRenderer, RHISwapChain** ppSwapChain)

//Returns the recommended format for the swapchain.
//If true is passed for the hintHDR parameter, it will return an HDR format IF the platform supports it
//If false is passed or the platform does not support HDR a non HDR format is returned.
//If true is passed for the hintSrgb parameter, it will return format that is will do gamma correction automatically
//If false is passed for the hintSrgb parameter the gamma correction should be done as a postprocess step before submitting image to swapchain
// JDECL_RHI_FUNC(EImageFormat, RHIGetRecommendedSwapchainFormat, bool bHDR, bool bSRGB)

JDECL_RHI_FUNC(void, RHIAddIndirectCommandSignature, RHIRenderer* pRender, const RHICommandSignatureDesc* pDesc, RHICommandSignature** ppCommandSignature)
JDECL_RHI_FUNC(void, RHIRemoveIndirectCommandSignature, RHIRenderer* pRenderer, RHICommandSignature* pCommandSignature)
JDECL_RHI_FUNC(void, RHICmdExecuteIndirect, RHICmd* pCmd, RHICommandSignature* pCommandSignature, u32 uMaxCommandCount, RHIBuffer* pIndirectBuffer, u64 uBufferOffset, RHIBuffer* pCounterBuffer, u64 uCounterBufferOffset)

/************************************************************************/
// GPU Query Interface
/************************************************************************/
JDECL_RHI_FUNC(void, RHIGetTimestampFrequency, RHIQueue* pQueue, f64* pFrequency)
JDECL_RHI_FUNC(void, RHIAddQueryPool, RHIRenderer* pRenderer, const RHIQueryPoolDesc* pDesc, RHIQueryPool** ppQueryPool)
JDECL_RHI_FUNC(void, RHIRemoveQueryPool, RHIRenderer* pRenderer, RHIQueryPool* pQueryPool)
JDECL_RHI_FUNC(void, RHIResetQueryPool, RHICmd* pCmd, RHIQueryPool* pQueryPool, u32 uStartQuery, u32 uQueryCount)
JDECL_RHI_FUNC(void, RHICmdBeginQuery, RHICmd* pCmd, RHIQueryPool* pQueryPool, RHIQueryDesc* pQuery)
JDECL_RHI_FUNC(void, RHICmdEndQuery, RHICmd* pCmd, RHIQueryPool* pQueryPool, RHIQueryDesc* pQuery)
JDECL_RHI_FUNC(void, RHICmdResolveQuery, RHICmd* pCmd, RHIQueryPool* pQueryPool, RHIBuffer* pReadbackBuffer, u32 uStartQuery, u32 uQueryCount)

/************************************************************************/
// Stats Info Interface
/************************************************************************/
JDECL_RHI_FUNC(void, RHICalculateMemoryStats, RHIRenderer* pRenderer, char** ppStatus)
JDECL_RHI_FUNC(void, RHICalculateMemoryUse, RHIRenderer* pRenderer, u64* pUsedBytes, u64* pTotalAllocatedBytes)
JDECL_RHI_FUNC(void, RHIFreeMemoryStats, RHIRenderer* pRenderer, char* stats)

/************************************************************************/
// Debug Marker Interface
/************************************************************************/
JDECL_RHI_FUNC(void, RHICmdBeginDebugMarker, RHICmd* pCmd, f32 r, f32 g, f32 b, const char* pName)
JDECL_RHI_FUNC(void, RHICmdEndDebugMarker, RHICmd* pCmd)
JDECL_RHI_FUNC(void, RHICmdAddDebugMarker, RHICmd* pCmd, f32 r, f32 g, f32 b, const char* pName)
JDECL_RHI_FUNC(u32, RHICmdWriteMarker, RHICmd* pCmd, EMarkerType eMarkerType, u32 uMarkerValue, RHIBuffer* pBuffer, size_t uOffset, bool bUseAutoFlags)

/************************************************************************/
// Resource Debug Naming Interface
/************************************************************************/
JDECL_RHI_FUNC(void, RHISetBufferName, RHIRenderer* pRenderer, RHIBuffer* pBuffer, const char* pName)
JDECL_RHI_FUNC(void, RHISetTextureName, RHIRenderer* pRenderer, RHITexture* pTexture, const char* pName)
JDECL_RHI_FUNC(void, RHISetRenderTargetName, RHIRenderer* pRenderer, RHIRenderTarget* pRenderTarget, const char* pName)
JDECL_RHI_FUNC(void, RHISetPipelineName, RHIRenderer* pRenderer, RHIPipeline* pPipeline, const char* pName)
