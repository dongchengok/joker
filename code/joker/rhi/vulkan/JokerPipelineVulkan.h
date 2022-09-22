#pragma once

#include "JokerVulkan.h"

namespace joker::rhi::vulkan
{

class JRHI_ALIGN PipelineVulkan final : public Pipeline
{
  public:
    PipelineVulkan(const PipelineDesc& desc);
    virtual ~PipelineVulkan();

};

class JRHI_ALIGN PipelineCacheVulkan final : public PipelineCache
{
    public:
    PipelineCacheVulkan(const PipelineCacheDesc& desc);
    virtual ~PipelineCacheVulkan();
};

}
