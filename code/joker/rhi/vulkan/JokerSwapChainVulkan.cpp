#include "JokerRHIPCH.h"
#include "JokerSwapChainVulkan.h"
#include "JokerDeviceVulkan.h"

namespace joker::rhi
{

JRHI_IMPL_FUNC_VK(SwapChain*, AddSwapChain, const SwapChainDesc& desc)
{
    vulkan::SwapChainVulkan* pSwapChain = JNEW vulkan::SwapChainVulkan(desc);
    return pSwapChain;
}

JRHI_IMPL_FUNC_VK(void, RemoveSwapChain, SwapChain* pSwapChain)
{
    JDELETE(pSwapChain);
}

}

namespace joker::rhi::vulkan
{

SwapChainVulkan::SwapChainVulkan(const SwapChainDesc& desc)
{
    m_pDesc = JNEW SwapChainDesc(desc);
    SDL_Window* pWindow = (SDL_Window*)desc.pWindow;
    JASSERT(pWindow);
    VkSurfaceKHR surface;
    JRHI_SDL_CHECK(SDL_Vulkan_CreateSurface(pWindow, JRHI_VK_INSTANCE, &surface));
    *(VkSurfaceKHR*)&m_HandleSurface = surface;
    VkSurfaceCapabilitiesKHR caps;
    JRHI_VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(JRHI_VK_GPU, surface, &caps));

    m_pDesc->nImageCount                      = JCLAMP(m_pDesc->nImageCount, caps.minImageCount, caps.maxImageCount);
    VkSurfaceFormatKHR          surfaceFormat = _SelectSurfaceFomrat(desc, surface);
    VkPresentModeKHR            presentMode   = _SelectPresentMode(desc, surface);
    VkExtent2D                  extent        = _SelectExtent(desc, caps);
    VkCompositeAlphaFlagBitsKHR alpha         = _SelectCompositeAlpha(caps);
    //不考虑90°，270°翻转的情况
    JASSERT(caps.currentTransform != VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR && caps.currentTransform != VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR &&
            caps.currentTransform != VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR &&
            caps.currentTransform != VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR);
    VkSurfaceTransformFlagBitsKHR preTransform = (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : caps.currentTransform;

    VkSwapchainCreateInfoKHR infoSwapChain;
    infoSwapChain.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    infoSwapChain.pNext                 = nullptr;
    infoSwapChain.flags                 = 0; // 用0就行了，几个flag不需要
    infoSwapChain.surface               = surface;
    infoSwapChain.minImageCount         = m_pDesc->nImageCount;
    infoSwapChain.imageFormat           = surfaceFormat.format;
    infoSwapChain.imageColorSpace       = surfaceFormat.colorSpace;
    infoSwapChain.imageExtent           = extent;
    infoSwapChain.imageArrayLayers      = 1; //不是VR用1就行了
    infoSwapChain.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    infoSwapChain.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE; //被队列共享的模式有额外的开销，游戏也用不到
    infoSwapChain.queueFamilyIndexCount = 0;                         // VK_SHARING_MODE_EXCLUSIVE模式为0就行了
    infoSwapChain.pQueueFamilyIndices   = nullptr;                   // VK_SHARING_MODE_EXCLUSIVE模式为0就行了
    infoSwapChain.preTransform          = preTransform;              // swapchain的翻转方式
    infoSwapChain.compositeAlpha        = alpha;
    infoSwapChain.presentMode           = presentMode;
    infoSwapChain.clipped               = VK_TRUE;
    infoSwapChain.oldSwapchain          = VK_NULL_HANDLE;
    JRHI_VK_CHECK(vkCreateSwapchainKHR(JRHI_VK_DEVICE, &infoSwapChain, JRHI_VK_ALLOC, (VkSwapchainKHR*)&m_HandleSwapChain));
}

SwapChainVulkan::~SwapChainVulkan()
{
    vkDestroySwapchainKHR(JRHI_VK_DEVICE, (VkSwapchainKHR)m_HandleSwapChain, JRHI_VK_ALLOC);
    vkDestroySurfaceKHR(JRHI_VK_INSTANCE, *(VkSurfaceKHR*)&m_HandleSurface, JRHI_VK_ALLOC);
    JDELETE m_pDesc;
}

VkSurfaceFormatKHR SwapChainVulkan::_SelectSurfaceFomrat(const SwapChainDesc& desc, VkSurfaceKHR surface)
{
    u32 uSupportFormatCount = 0;
    JRHI_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(JRHI_VK_GPU, surface, &uSupportFormatCount, nullptr));
    // alloca在栈上分配内存不用回收
    VkSurfaceFormatKHR* pFormats = (VkSurfaceFormatKHR*)JALLOCA(uSupportFormatCount * sizeof(VkSurfaceFormatKHR));
    JRHI_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(JRHI_VK_GPU, surface, &uSupportFormatCount, pFormats));

    // 优先使用
    // 1.VK_FORMAT_A2B10G10R10_UNORM_PACK32 VK_COLOR_SPACE_HDR10_ST2084_EXT 提高性能
    // 1.VK_FORMAT_A2B10G10R10_UNORM_PACK32 VK_COLOR_SPACE_SRGB_NONLINEAR_KHR 提高性能
    // 2.VK_FORMAT_B8G8R8A8_SRGB VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    // 2.VK_FORMAT_B8G8R8A8_UNORM VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    VkColorSpaceKHR colorSpace;
    switch(desc.eColorSpace)
    {
        case EColorSpace::sRGB:
            colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            break;
        case EColorSpace::HDR10:
            colorSpace = VK_COLOR_SPACE_HDR10_ST2084_EXT;
            break;
        case EColorSpace::P3:
            colorSpace = VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT;
            break;
        case EColorSpace::BT709:
            colorSpace = VK_COLOR_SPACE_BT709_LINEAR_EXT;
            break;
        case EColorSpace::BT2020:
            colorSpace = VK_COLOR_SPACE_BT2020_LINEAR_EXT;
            break;
        case EColorSpace::AdobeRGB:
            colorSpace = VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT;
            break;
        case EColorSpace::DolbVision:
            colorSpace = VK_COLOR_SPACE_DOLBYVISION_EXT;
            break;
        default:
            colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            break;
    }
    JASSERT(uSupportFormatCount > 0);
    VkSurfaceFormatKHR fmt {VK_FORMAT_MAX_ENUM,VK_COLOR_SPACE_MAX_ENUM_KHR};
    for (uint32_t i = 0; i < uSupportFormatCount; ++i)
    {
        if(colorSpace != pFormats[i].colorSpace)
        {
            continue;
        }
        if(VK_FORMAT_A2B10G10R10_UNORM_PACK32==pFormats[i].format)
        {
            return pFormats[i];
        }
        else if(VK_FORMAT_B8G8R8A8_SRGB==pFormats[i].format)
        {
            fmt = pFormats[i];
        }
        else if(VK_FORMAT_B8G8R8A8_UNORM==pFormats[i].format)
        {
            if(VK_FORMAT_B8G8R8A8_SRGB!=pFormats[i].format)
            {
                fmt = pFormats[i];
            }
        }
    }
    JASSERT(fmt.format!=VK_FORMAT_MAX_ENUM);
    return fmt;
}

VkPresentModeKHR SwapChainVulkan::_SelectPresentMode(const SwapChainDesc& desc, VkSurfaceKHR surface)
{
    // 垂直同步处理方式，不考虑支持情况下,优先级为
    // 1.VK_PRESENT_MODE_FIFO_RELAXED_KHR 比上面好的点是速度慢了也会立即提交，知识画面可能有撕裂，我喜欢这个
    // 2.VK_PRESENT_MODE_FIFO_KHR 典型双缓冲，如果速度慢，队列满了，可能有空帧
    // 3.VK_PRESENT_MODE_IMMEDIATE_KHR 立即提交，延迟最小，但是不等显示设备刷新率，画面会撕裂
    // 4.VK_PRESENT_MODE_MAILBOX_KHR 三缓冲，延迟太高了，不考虑
    u32 uSupportPresentCount = 0;
    JRHI_VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(JRHI_VK_GPU, surface, &uSupportPresentCount, nullptr));
    VkPresentModeKHR* pPresents = (VkPresentModeKHR*)JALLOCA(uSupportPresentCount * sizeof(VkPresentModeKHR));
    JRHI_VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(JRHI_VK_GPU, surface, &uSupportPresentCount, pPresents));
    VkPresentModeKHR mode = pPresents[0];
    for (u32 i = 0; i < uSupportPresentCount; ++i)
    {
        VkPresentModeKHR supportMode = pPresents[i];
        if ((desc.bVSync && VK_PRESENT_MODE_IMMEDIATE_KHR == pPresents[i]) || (!desc.bVSync && VK_PRESENT_MODE_FIFO_RELAXED_KHR == pPresents[i]))
        {
            return supportMode;
        }
        switch (supportMode)
        {
        case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
            mode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
            break;
        case VK_PRESENT_MODE_FIFO_KHR:
            if (mode != VK_PRESENT_MODE_FIFO_RELAXED_KHR)
            {
                mode = VK_PRESENT_MODE_FIFO_KHR;
            }
            break;
        case VK_PRESENT_MODE_IMMEDIATE_KHR:
            if (mode != VK_PRESENT_MODE_FIFO_RELAXED_KHR && mode != VK_PRESENT_MODE_FIFO_KHR)
            {
                mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
            break;
        case VK_PRESENT_MODE_MAILBOX_KHR:
            if (mode != VK_PRESENT_MODE_FIFO_RELAXED_KHR && mode != VK_PRESENT_MODE_FIFO_KHR && mode != VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                mode = VK_PRESENT_MODE_MAILBOX_KHR;
            }
            break;
        default:
            break;
        }
    }
    return mode;
}

VkExtent2D SwapChainVulkan::_SelectExtent(const SwapChainDesc& desc, const VkSurfaceCapabilitiesKHR& caps)
{
    VkExtent2D extent;
    extent.width  = desc.fScale <= 0 ? caps.currentExtent.width : (JCLAMP((u32)(desc.fScale * caps.currentExtent.width), caps.minImageExtent.width, caps.maxImageExtent.width));
    extent.height = desc.fScale <= 0 ? caps.currentExtent.height : (JCLAMP((u32)(desc.fScale * caps.currentExtent.height), caps.minImageExtent.height, caps.maxImageExtent.height));
    return extent;
}

VkCompositeAlphaFlagBitsKHR SwapChainVulkan::_SelectCompositeAlpha(const VkSurfaceCapabilitiesKHR& caps)
{
    // 到硬件的混合方式，游戏吗，默认不透明就完了
    if (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
    {
        return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }
    else if (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
    {
        return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }
    else if (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
    {
        return VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
    }
    else
    {
        return VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    }
}

}