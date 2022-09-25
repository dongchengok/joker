#include "JokerRHIPCH.h"
#include "JokerCommandVulkan.h"
#include "JokerQueueVulkan.h"
#include "JokerDeviceVulkan.h"

namespace joker::rhi
{

JRHI_IMPL_FUNC_VK(CommandPool*, AddCommandPool, const CommandPoolDesc& desc)
{
    return JNEW vulkan::CommandPoolVulkan(desc);
}

JRHI_IMPL_FUNC_VK(void, RemoveCommandPool, CommandPool* pPool)
{
    JDELETE pPool;
}

JRHI_IMPL_FUNC_VK(Command*, AddCommand, const CommandDesc& desc)
{
    return JNEW vulkan::CommandVulkan(desc);
}

JRHI_IMPL_FUNC_VK(void, RemoveCommand, Command* pBuffer)
{
    JDELETE pBuffer;
}

}

namespace joker::rhi::vulkan
{

CommandPoolVulkan::CommandPoolVulkan(const CommandPoolDesc& desc)
{
    m_pDesc = JNEW          CommandPoolDesc(desc);

    VkCommandPoolCreateInfo info;
    info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext            = nullptr;
    info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = ((QueueVulkan*)desc.pQueue)->m_uFamilyIndex;
    JRHI_VK_CHECK(vkCreateCommandPool(JRHI_VK_DEVICE, &info, JRHI_VK_ALLOC, (VkCommandPool*)&m_Handle));
}

CommandPoolVulkan::~CommandPoolVulkan()
{
    vkDestroyCommandPool(JRHI_VK_DEVICE, (VkCommandPool)m_Handle, JRHI_VK_ALLOC);
    JDELETE m_pDesc;
}

VkCommandBuffer CommandVulkan::ms_pActiveCommand = nullptr;

CommandVulkan::CommandVulkan(const CommandDesc& desc) : m_pPool((CommandPoolVulkan*)desc.pPool)
{
    m_pDesc = JNEW              CommandDesc(desc);
    VkCommandBufferAllocateInfo info;
    info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext              = nullptr;
    info.commandBufferCount = 1;
    info.commandPool        = (VkCommandPool)m_pPool->m_Handle;
    info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    JRHI_VK_CHECK(vkAllocateCommandBuffers(JRHI_VK_DEVICE, &info, (VkCommandBuffer*)&m_Handle));
}

CommandVulkan::~CommandVulkan()
{
    vkFreeCommandBuffers(JRHI_VK_DEVICE, (VkCommandPool)m_pPool->m_Handle, 1, (VkCommandBuffer*)&m_Handle);
    JDELETE m_pDesc;
}

}