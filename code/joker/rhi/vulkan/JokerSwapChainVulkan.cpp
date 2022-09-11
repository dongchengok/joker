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

SwapChainVulkan::SwapChainVulkan(const SwapChainDesc& desc) : SwapChain(desc)
{
    SDL_Window* pWindow = (SDL_Window*)desc.pWindow;
    JASSERT(pWindow);
    VkSurfaceKHR surface;
    JRHI_SDL_CHECK(SDL_Vulkan_CreateSurface(pWindow, JRHI_VK_INSTANCE, &surface));
    *(VkSurfaceKHR*)&m_pHWSurface = surface;
    VkSurfaceCapabilitiesKHR caps;
    JRHI_VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(JRHI_VK_GPU, surface, &caps));

    m_pDesc->nImageCount                      = JCLAMP(m_pDesc->nImageCount, caps.minImageCount, caps.maxImageCount);
    VkSurfaceFormatKHR          surfaceFormat = _SelectSurfaceFomrat(*m_pDesc, surface);
    VkPresentModeKHR            presentMode   = _SelectPresentMode(desc, surface);
    VkExtent2D                  extent        = _SelectExtent(desc, caps);
    VkCompositeAlphaFlagBitsKHR alpha         = _SelectCompositeAlpha(caps);
    //不考虑90°，270°翻转的情况
    JASSERT(caps.currentTransform != VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR && caps.currentTransform != VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR &&
            caps.currentTransform != VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR &&
            caps.currentTransform != VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR);
    VkSurfaceTransformFlagBitsKHR preTransform = (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : caps.currentTransform;

    JRHI_VK_DESC(VkSwapchainCreateInfoKHR, infoSwapChain, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
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
    JRHI_VK_CHECK(vkCreateSwapchainKHR(JRHI_VK_DEVICE, &infoSwapChain, JRHI_VK_ALLOC, (VkSwapchainKHR*)&m_pHWSwapChain));
}

SwapChainVulkan::~SwapChainVulkan()
{
    vkDestroySwapchainKHR(JRHI_VK_DEVICE, (VkSwapchainKHR)m_pHWSwapChain, JRHI_VK_ALLOC);
    vkDestroySurfaceKHR(JRHI_VK_INSTANCE, *(VkSurfaceKHR*)&m_pHWSurface, JRHI_VK_ALLOC);
}

VkSurfaceFormatKHR SwapChainVulkan::_SelectSurfaceFomrat(SwapChainDesc& desc, VkSurfaceKHR surface)
{
    u32 uSupportFormatCount = 0;
    JRHI_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(JRHI_VK_GPU, surface, &uSupportFormatCount, nullptr));
    // alloca在栈上分配内存不用回收
    VkSurfaceFormatKHR* pFormats = (VkSurfaceFormatKHR*)JALLOCA(uSupportFormatCount * sizeof(VkSurfaceFormatKHR));
    JRHI_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(JRHI_VK_GPU, surface, &uSupportFormatCount, pFormats));

    // TODO SRGB LINER UNIFORM

    // 优先使用
    // 1.HDR VK_FORMAT_A2B10G10R10_UNORM_PACK32 VK_COLOR_SPACE_HDR10_ST2084_EXT 提高性能，并且支持HDR
    // 2.VK_FORMAT_B8G8R8A8_SRGB VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    // 3.VK_FORMAT_B8G8R8A8_UNORM VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    // 在没有的话随便选一个
    JASSERT(uSupportFormatCount > 0);
    VkSurfaceFormatKHR surfaceFormat = pFormats[0];
    for (uint32_t i = 0; i < uSupportFormatCount; ++i)
    {
        if (VK_FORMAT_A2B10G10R10_UNORM_PACK32 == pFormats[i].format && VK_COLOR_SPACE_SRGB_NONLINEAR_KHR == pFormats[i].colorSpace)
        {
            surfaceFormat = pFormats[i];
            break;
        }
        else if (VK_FORMAT_A2B10G10R10_UNORM_PACK32 == pFormats[i].format && VK_COLOR_SPACE_HDR10_ST2084_EXT == pFormats[i].colorSpace)
        {
            surfaceFormat = pFormats[i];
        }
        else if (VK_FORMAT_B8G8R8A8_UNORM == pFormats[i].format && VK_COLOR_SPACE_SRGB_NONLINEAR_KHR == pFormats[i].colorSpace)
        {
            surfaceFormat = pFormats[i];
        }
    }
    return surfaceFormat;
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