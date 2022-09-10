#include "JokerRHIPCH.h"
#include "JokerRenderer.h"

namespace joker::rhi
{

Renderer::Renderer(const RendererDesc& desc)
:m_Desc(desc)
{
}

Renderer::~Renderer()
{
}

}