#pragma once
#include "JokerVulkan.h"
#include "JokerSwapChain.h"

namespace joker::rhi::vulkan
{

class JRHI_ALIGN SwapChainVulkan final : public SwapChain
{
  public:
    SwapChainVulkan(const SwapChainDesc& desc);
    virtual ~SwapChainVulkan();

  private:
    static VkSurfaceFormatKHR          _SelectSurfaceFomrat(SwapChainDesc& desc, VkSurfaceKHR surface);
    static VkPresentModeKHR            _SelectPresentMode(const SwapChainDesc& desc, VkSurfaceKHR surface);
    static VkExtent2D                  _SelectExtent(const SwapChainDesc& desc, const VkSurfaceCapabilitiesKHR& caps);
    static VkCompositeAlphaFlagBitsKHR _SelectCompositeAlpha(const VkSurfaceCapabilitiesKHR& caps);
};

}