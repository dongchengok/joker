#pragma once
#include "JokerVulkan.h"

namespace joker::rhi::vulkan
{

class JRHI_ALIGN RenderPassVulkan final : public Resource
{
  public:
    static RenderPassVulkan* GetOrCreate(const RenderPassDesc& desc);
    static RenderPassVulkan* GetActive();
    static void              SetActive(RenderPassVulkan* pActive);

  protected:
    RenderPassVulkan(u32 uHashCode, const VkRenderPassCreateInfo& info);
    ~RenderPassVulkan();

    static RenderPassVulkan* _GetOrCreate(u32 uHashCode, const VkRenderPassCreateInfo& info);

  public:
    u32                    m_uHashCode   = 0;
    VkDevice               m_hDevice     = nullptr;
    VkAllocationCallbacks* m_hAlloc      = nullptr;
    VkRenderPass           m_hRenderPass = nullptr;

  protected:
    static hash_map<u32, RenderPassVulkan*> ms_mpRenderPass;
    static RenderPassVulkan*                ms_pActive;
};

}