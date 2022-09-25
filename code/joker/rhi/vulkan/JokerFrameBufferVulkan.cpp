#include "JokerRHIPCH.h"
#include "JokerFrameBufferVulkan.h"

namespace joker::rhi::vulkan
{

FrameBufferVulkan* FrameBufferVulkan::GetOrCreate()
{
    VkFramebufferCreateInfo info;
    info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.pNext           = nullptr;
    info.flags           = 0;
    info.renderPass      = nullptr;
    info.attachmentCount = 0;
    info.pAttachments    = nullptr;
    info.width           = 0;
    info.height          = 0;
    info.layers          = 0;


    return nullptr;
}

FrameBufferVulkan* FrameBufferVulkan::_GetOrCreate(u32 uHashCode, const VkFramebufferCreateInfo& info)
{
    return nullptr;
}

}