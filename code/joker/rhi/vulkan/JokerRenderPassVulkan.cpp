#include "JokerRHIPCH.h"
#include "JokerRenderPassVulkan.h"
#include "JokerCommandVulkan.h"
#include "JokerFrameBufferVulkan.h"
#include "JokerDeviceVulkan.h"

namespace joker::rhi
{

JRHI_IMPL_FUNC_VK(void, CommandBeginRenderPass, const RenderPassDesc& desc, const char* szName)
{
    // vulkan::RenderPassVulkan* pRenderPass = vulkan::RenderPassVulkan::GetActive();
    VkCommandBuffer command     = vulkan::CommandVulkan::GetActive();
    // VkFramebuffer   frameBuffer = vulkan::FrameBufferVulkan::GetOrCreate();
    // JASSERT(pRenderPass == nullptr);
    vulkan::RenderPassVulkan* pRenderPass = vulkan::RenderPassVulkan::GetOrCreate(desc);
    vulkan::FrameBufferVulkan* pFrameBuffer = vulkan::FrameBufferVulkan::GetOrCreate();

    VkRenderPassBeginInfo     beginInfo;
    beginInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext           = nullptr;
    beginInfo.renderPass      = pRenderPass->m_hRenderPass;
    beginInfo.framebuffer     = pFrameBuffer->m_hFrameBuffer;
    beginInfo.renderArea      = {}; // TODO
    beginInfo.clearValueCount = 1;  // rt count
    beginInfo.pClearValues    = {}; // clear
    vkCmdBeginRenderPass(command, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vulkan::RenderPassVulkan::SetActive(pRenderPass);
}

JRHI_IMPL_FUNC_VK(void, CommandEndRenderPass)
{
    vulkan::RenderPassVulkan* pRenderPass = vulkan::RenderPassVulkan::GetActive();
    JASSERT(pRenderPass != nullptr);
    vkCmdEndRenderPass(vulkan::CommandVulkan::GetActive());
    vulkan::RenderPassVulkan::SetActive(nullptr);
}

}

namespace joker::rhi::vulkan
{

hash_map<u32, RenderPassVulkan*> RenderPassVulkan::ms_mpRenderPass;
RenderPassVulkan*                RenderPassVulkan::ms_pActive = nullptr;

RenderPassVulkan*                RenderPassVulkan::GetOrCreate(const RenderPassDesc& desc)
{
    u32                      uColorTargetCount        = (u32)desc.vColorRenderTargets.size();
    u32                      uDepthStencilTargetCount = desc.DepthStencilTarget.pDepthStencilTarget != nullptr ? 1 : 0;
    u32                      uTargetCount             = uColorTargetCount + uDepthStencilTargetCount;
    VkAttachmentDescription* pAttachments             = (VkAttachmentDescription*)JALLOCA(sizeof(VkAttachmentDescription) * uTargetCount);
    VkAttachmentReference*   pReferences              = (VkAttachmentReference*)JALLOCA(sizeof(VkAttachmentReference) * uTargetCount);
    for (u32 i = 0; i < uTargetCount; ++i)
    {
        const RenderPassDesc::ColorAttachmentDesc& descTarget  = desc.vColorRenderTargets[i];
        const TextureDesc&                         descTexture = descTarget.pRenderTarget->GetDesc();
        VkAttachmentDescription&                   attachment  = pAttachments[i];
        attachment.flags                                       = 0;
        attachment.format                                      = ToFormatVK(descTexture.eFormat);
        attachment.samples                                     = ToSampleVK(descTexture.eNumSamples);
        attachment.loadOp                                      = ToLoadOpVK(descTarget.eLoadAction);
        attachment.storeOp                                     = ToStoreOpVK(descTarget.eStoreAction);
        attachment.stencilLoadOp                               = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp                              = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout                               = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.finalLayout                                 = attachment.initialLayout;

        pReferences[i].attachment                              = i;
        pReferences[i].layout                                  = attachment.initialLayout;
    }

    if (uDepthStencilTargetCount)
    {
        const TextureDesc&       descTexture      = desc.DepthStencilTarget.pDepthStencilTarget->GetDesc();
        VkAttachmentDescription& attachment       = pAttachments[uColorTargetCount];
        attachment.flags                          = 0;
        attachment.format                         = ToFormatVK(descTexture.eFormat);
        attachment.samples                        = ToSampleVK(descTexture.eNumSamples);
        attachment.loadOp                         = ToLoadOpVK(desc.DepthStencilTarget.eLoadActionDepth);
        attachment.storeOp                        = ToStoreOpVK(desc.DepthStencilTarget.eStoreActionDepth);
        attachment.stencilLoadOp                  = ToLoadOpVK(desc.DepthStencilTarget.eLoadActionStencil);
        attachment.stencilStoreOp                 = ToStoreOpVK(desc.DepthStencilTarget.eStoreActionStencil);
        attachment.initialLayout                  = ToDepthStencilImageLayoutVK(desc.DepthStencilTarget.bWriteableDepth, desc.DepthStencilTarget.bWriteableStencil);
        attachment.finalLayout                    = attachment.initialLayout;

        pReferences[uColorTargetCount].attachment = uColorTargetCount;
        pReferences[uColorTargetCount].layout     = attachment.initialLayout;
    }

    VkSubpassDescription subpass{};
    subpass.flags                   = 0;
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount    = 0;
    subpass.pInputAttachments       = nullptr;
    subpass.colorAttachmentCount    = uColorTargetCount;
    subpass.pColorAttachments       = pReferences;
    subpass.pResolveAttachments     = nullptr;
    subpass.pDepthStencilAttachment = uDepthStencilTargetCount > 0 ? (pReferences + uTargetCount) : nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments    = nullptr;

    VkRenderPassCreateInfo createInfo;
    createInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext           = nullptr;
    createInfo.flags           = 0;
    createInfo.attachmentCount = uTargetCount;
    createInfo.pAttachments    = pAttachments;
    createInfo.subpassCount    = 1;
    createInfo.pSubpasses      = &subpass;
    createInfo.dependencyCount = 0; // 有subpass的情况下，这个有依赖
    createInfo.pDependencies   = nullptr;

    u32 uHashCode              = algorithm::Crc32(pAttachments, sizeof(VkAttachmentDescription) * uTargetCount);
    return _GetOrCreate(uHashCode, createInfo);
}

RenderPassVulkan::RenderPassVulkan(u32 uHashCode, const VkRenderPassCreateInfo& info) : m_uHashCode(uHashCode), m_hDevice(JRHI_VK_DEVICE), m_hAlloc(JRHI_VK_ALLOC)
{
    vkCreateRenderPass(m_hDevice, &info, m_hAlloc, &m_hRenderPass);
}

RenderPassVulkan::~RenderPassVulkan()
{
    vkDestroyRenderPass(m_hDevice, m_hRenderPass, m_hAlloc);
}

RenderPassVulkan* RenderPassVulkan::_GetOrCreate(u32 uHashCode, const VkRenderPassCreateInfo& info)
{
    auto it = ms_mpRenderPass.find(uHashCode);
    if (it != ms_mpRenderPass.end())
    {
        return it->second;
    }
    RenderPassVulkan* pRenderPass = JNEW RenderPassVulkan(uHashCode, info);
    ms_mpRenderPass[uHashCode]    = pRenderPass;
    return pRenderPass;
}

RenderPassVulkan* RenderPassVulkan::GetActive()
{
    return ms_pActive;
}

void RenderPassVulkan::SetActive(RenderPassVulkan* pActive)
{
    ms_pActive = pActive;
}

}