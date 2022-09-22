#include "JokerRHIPCH.h"
#include "JokerQueueVulkan.h"
#include "JokerDeviceVulkan.h"

namespace joker::rhi
{

JRHI_IMPL_FUNC_VK(Queue*, AddQueue, const QueueDesc& desc)
{
    return JNEW vulkan::QueueVulkan(desc);
}

JRHI_IMPL_FUNC_VK(void, RemoveQueue, Queue* pQueue)
{
    JDELETE pQueue;
}

}

namespace joker::rhi::vulkan
{

QueueVulkan::QueueVulkan(const QueueDesc& desc)
{
    m_pDesc = JNEW QueueDesc(desc);
    JASSERT(g_pRendererVulkan->AcquireQueue(desc.eType, m_uFamilyIndex, m_uIndex));
    vkGetDeviceQueue(JRHI_VK_DEVICE, m_uFamilyIndex, m_uIndex, (VkQueue*)&m_Handle);
}

QueueVulkan::~QueueVulkan()
{
    g_pRendererVulkan->ReleaseQueue(GetDesc().eType, m_uFamilyIndex, m_uIndex);
    JDELETE m_pDesc;
}

}