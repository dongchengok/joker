#include "JokerRHIPCH.h"
#include "JokerRenderPassVulkan.h"
#include "JokerCommandVulkan.h"
#include "JokerFrameBufferVulkan.h"

namespace joker::rhi
{

JRHI_IMPL_FUNC_VK(void, CommandBeginRenderPass, const RenderPassDesc& desc, const char* szName)
{
    VkRenderPass    renderPass  = vulkan::RenderPassVulkan::GetActive();
    VkCommandBuffer command     = vulkan::CommandVulkan::GetActive();
    VkFramebuffer   frameBuffer = vulkan::FrameBufferVulkan::GetOrCreate();
    JASSERT(renderPass == nullptr);
    renderPass = vulkan::RenderPassVulkan::GetOrCreate(desc);

    JRHI_VK_DESC(VkRenderPassBeginInfo, beginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
    beginInfo.renderPass      = renderPass;
    beginInfo.framebuffer     = frameBuffer;
    beginInfo.renderArea      = {}; // TODO
    beginInfo.clearValueCount = 1;  // rt count
    beginInfo.pClearValues    = {}; // clear
    vkCmdBeginRenderPass(command, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vulkan::RenderPassVulkan::SetActive();
}

JRHI_IMPL_FUNC_VK(void, CommandEndRenderPass)
{
    VkRenderPass renderPass = vulkan::RenderPassVulkan::GetActive();
    JASSERT(renderPass != nullptr);
    vkCmdEndRenderPass(vulkan::CommandVulkan::GetActive());
    vulkan::RenderPassVulkan::SetActive();
}

}

namespace joker::rhi::vulkan
{

VkRenderPass RenderPassVulkan::GetOrCreate(const RenderPassDesc& desc)
{

    // desc.vColorRenderTargets

    VkAttachmentDescription colorAttachment{};
    // colorAttachment.flags =
    // colorAttachment.format =
    colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT; // MSAA
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.flags                   = 0;
    subpass.inputAttachmentCount    = 0;
    subpass.pInputAttachments       = nullptr;
    subpass.colorAttachmentCount    = 0;
    subpass.pColorAttachments       = nullptr;
    subpass.pResolveAttachments     = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments    = nullptr;

    JRHI_VK_DESC(VkRenderPassCreateInfo, createInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
    createInfo.flags           = 0;
    createInfo.attachmentCount = 0;
    createInfo.pAttachments    = nullptr;
    createInfo.subpassCount    = 1;
    createInfo.pSubpasses      = nullptr;
    createInfo.dependencyCount = 0; // 有subpass的情况下，这个有依赖
    createInfo.pDependencies   = nullptr;

    return nullptr;
}

}