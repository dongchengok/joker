#pragma once

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
struct RHISemaphore;
struct RHIQueueDesc;
struct RHIQueue;
struct RHISwapChainDesc;
struct RHISwapChain;
struct RHICmdPoolDesc;
struct RHICmdPool;
struct RHICmdDesc;
struct RHICmd;
struct RHIRenderTargetDesc;
struct RHIRenderTarget;
struct RHISamplerDesc;
struct RHISampler;
struct RHIShaderDesc;
struct RHIShader;
struct RHIRootSignatureDesc;
struct RHIRootSignature;
struct RHIPipelineDesc;
struct RHIPipeline;
struct RHIPipelineCacheDesc;
struct RHIPipelineCache;
struct RHIDescriptorSetDesc;
struct RHIDescriptorSet;
struct RHIDescriptorData;

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