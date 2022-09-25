#pragma once
#include "JokerVulkan.h"

namespace joker::rhi::vulkan
{

class JRHI_ALIGN FrameBufferVulkan final : public Resource
{
  public:
    static FrameBufferVulkan* GetOrCreate();

  protected:
    static FrameBufferVulkan* _GetOrCreate(u32 uHashCode, const VkFramebufferCreateInfo& infO);

  public:
    u32 m_uHashCode = 0;
    VkDevice m_hDevice = nullptr;
    VkAllocationCallbacks* m_hAlloc = nullptr;
    VkFramebuffer m_hFrameBuffer = nullptr;
};

}