#pragma once
#include "JokerVulkan.h"

namespace joker::rhi::vulkan
{

class JRHI_ALIGN QueueVulkan final : public Queue
{

  public:
    QueueVulkan(const QueueDesc& desc);
    virtual ~QueueVulkan();

  public:
    u32 m_uFamilyIndex = 0xFFFFFFFF;
    u32 m_uIndex       = 0xFFFFFFFF;
};

}