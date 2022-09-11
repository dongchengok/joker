#include "JokerRHIPCH.h"
#include "JokerSwapChain.h"

namespace joker::rhi {

SwapChain::SwapChain(const SwapChainDesc& desc)
{
    m_pDesc = new SwapChainDesc(desc);
}

SwapChain::~SwapChain()
{
    JFREE(m_pDesc);    
}

}