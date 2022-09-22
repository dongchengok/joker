#include "JokerRHIPCH.h"
#include "JokerPipelineVulkan.h"

namespace joker::rhi
{

JRHI_IMPL_FUNC_VK(Pipeline*, AddPipeline, const PipelineDesc& desc)
{
    return nullptr;
}

JRHI_IMPL_FUNC_VK(void, RemovePipeline, Pipeline* pPipeline)
{
}

JRHI_IMPL_FUNC_VK(PipelineCache*, AddPipelineCache, const PipelineCacheDesc& desc)
{
    return nullptr;
}

JRHI_IMPL_FUNC_VK(void, RemovePipelineCache, PipelineCache* pPipelineCache)
{
}

JRHI_IMPL_FUNC_VK(void, GetPipelineCacheData, u32* pSize, void* pData)
{
}

}

namespace joker::rhi::vulkan
{

PipelineVulkan::PipelineVulkan(const PipelineDesc& desc)
{
    m_pDesc = JNEW PipelineDesc(desc);
}

PipelineVulkan::~PipelineVulkan()
{
    JDELETE m_pDesc;
}

PipelineCacheVulkan::PipelineCacheVulkan(const PipelineCacheDesc& desc)
{
    m_pDesc = JNEW PipelineCacheDesc(desc);
}

PipelineCacheVulkan::~PipelineCacheVulkan()
{
    JDELETE m_pDesc;
}

}