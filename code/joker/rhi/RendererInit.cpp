#include "JokerRHIPCH.h"
#include "RHIEnum.h"
#include "RendererInit.h"

namespace joker::rhi
{

RendererContext* InitRendererContext(const RendererContextDesc* pDesc)
{
    JASSERT(pDesc);
    switch (pDesc->m_eRenderer)
    {
    case ERenderer::Vulkan:
        return vulkan::vkInitRendererContext(pDesc);
    default:
        JASSERT(false);
    }
    return nullptr;
}

void ExitRendererContext(RendererContext* pContext)
{
    JASSERT(pContext);
    switch (pContext->m_eRenderer)
    {
    case ERenderer::Vulkan:
        vulkan::vkExitRendererContext(pContext);
        return;
    default:
        JASSERT(false);
    }
}

Renderer* InitRenderer(const RendererDesc* pDesc)
{
    JASSERT(pDesc);
    switch(pDesc->m_eRenderer)
    {
        case ERenderer::Vulkan:
        return vulkan::vkInitRenderer(pDesc);
        default:
        JASSERT(false);
    }
    return nullptr;
}

void ExitRenderer(Renderer* pRenderer)
{
    JASSERT(pRenderer);
    switch(pRenderer->m_eRenderer)
    {
        case ERenderer::Vulkan:
        vulkan::vkExitRenderer(pRenderer);
        return;
        default:
        JASSERT(false);
    }
}

}