#pragma once

#include "JokerCore.h"

#include "JokerRHIEnum.h"
#include "JokerRHIType.h"
#include "JokerRHIResource.h"

namespace joker::rhi
{

extern Device* InitDevice(const DeviceDesc& desc);
extern void    ExitDevice(Device* pRenderer);

// SwapChain
JRHI_DECL_FUNC(SwapChain*, AddSwapChain, const SwapChainDesc&);
JRHI_DECL_FUNC(void, RemoveSwapChain, SwapChain*);

JRHI_DECL_FUNC(Queue*, AddQueue, const QueueDesc&);
JRHI_DECL_FUNC(void, RemoveQueue, Queue*);

// CommandBufferPool
JRHI_DECL_FUNC(CommandPool*, AddCommandPool, const CommandPoolDesc&);
JRHI_DECL_FUNC(void, RemoveCommandPool, CommandPool*);
JRHI_DECL_FUNC(Command*, AddCommand, const CommandDesc& desc);
JRHI_DECL_FUNC(void, RemoveCommand, Command*);

JRHI_DECL_FUNC(void, CommandBeginRenderPass, const RenderPassDesc&, const char* szName = nullptr);
JRHI_DECL_FUNC(void, CommandEndRenderPass);

// Pipeline
JRHI_DECL_FUNC(Pipeline*, AddPipeline, const PipelineDesc&);
JRHI_DECL_FUNC(void, RemovePipeline, Pipeline*);
JRHI_DECL_FUNC(PipelineCache*, AddPipelineCache, const PipelineCacheDesc&);
JRHI_DECL_FUNC(void, RemovePipelineCache, PipelineCache*);
JRHI_DECL_FUNC(bool, GetPipelineCacheData, PipelineCache*, u32*, void*);

}