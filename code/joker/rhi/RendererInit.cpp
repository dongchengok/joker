#include "JokerRHIPCH.h"
#include "RHIEnum.h"
#include "RendererInit.h"

namespace joker::rhi
{

namespace vulkan
{
extern void _vkInitRendererContext(const RendererContextDesc* pDesc, RendererContext** ppContext);
extern void _vkExitRendererContext(RendererContext* pContext);
extern void _vkInitRenderer(const RendererDesc* pDesc, Renderer** ppRenderer);
extern void _vkExitRenderer(Renderer* pRenderer);
}

void InitRendererContext(const RendererContextDesc* pDesc, RendererContext** ppContext)
{
    JASSERT(pDesc);
    switch (pDesc->m_eRenderer)
    {
    case ERenderer::Vulkan:
        vulkan::_vkInitRendererContext(pDesc, ppContext);
        return;
    default:
        JASSERT(false);
    }
}

void ExitRendererContext(RendererContext* pContext)
{
    JASSERT(pContext);
    switch (pContext->m_eRenderer)
    {
    case ERenderer::Vulkan:
        vulkan::_vkExitRendererContext(pContext);
        return;
    default:
        JASSERT(false);
    }
}

void InitRenderer(const RendererDesc* pDesc, Renderer** ppRenderer)
{
    JASSERT(pDesc);
    switch (pDesc->m_eRenderer)
    {
    case ERenderer::Vulkan:
        vulkan::_vkInitRenderer(pDesc, ppRenderer);
        return;
    default:
        JASSERT(false);
    }
}

void ExitRenderer(Renderer* pRenderer)
{
    JASSERT(pRenderer);
    switch (pRenderer->m_eRenderer)
    {
    case ERenderer::Vulkan:
        vulkan::_vkExitRenderer(pRenderer);
        return;
    default:
        JASSERT(false);
    }
}

}