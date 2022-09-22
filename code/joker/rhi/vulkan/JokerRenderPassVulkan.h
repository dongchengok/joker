#pragma once
#include "JokerVulkan.h"

namespace joker::rhi::vulkan
{

class JRHI_ALIGN RenderPassVulkan : public Resource
{
  public:
    static VkRenderPass GetOrCreate(const RenderPassDesc& desc);
    static VkRenderPass GetActive();
    static VkRenderPass SetActive();

  public:
    
};

}