#pragma once

#include "JokerVulkan.h"

namespace joker::rhi::vulkan
{

class JRHI_ALIGN CommandPoolVulkan final : public CommandPool
{
  public:
    CommandPoolVulkan(const CommandPoolDesc& desc);
    virtual ~CommandPoolVulkan();
};

class JRHI_ALIGN CommandVulkan final : public Command
{
  public:
    CommandVulkan(const CommandDesc& desc);
    virtual ~CommandVulkan();

  public:
    inline static VkCommandBuffer GetActive()
    {
      return ms_pActiveCommand;
    }

  private:
    CommandPoolVulkan* m_pPool = nullptr;

    static VkCommandBuffer ms_pActiveCommand;
};

}