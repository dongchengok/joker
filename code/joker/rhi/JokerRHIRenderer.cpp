#include "JokerRHIPCH.h"
#include "JokerRHIRenderer.h"

namespace joker
{

RHIRenderer::RHIRenderer(const RHIRendererDesc& desc)
:m_Desc(desc)
{
}

RHIRenderer::~RHIRenderer()
{
}

}