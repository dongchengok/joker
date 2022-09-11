#pragma once

#include "JokerCore.h"

#include "JokerRHIEnum.h"
#include "JokerRHIType.h"

#include "JokerDevice.h"
#include "JokerSwapChain.h"

namespace joker::rhi
{

extern Device* InitDevice(const DeviceDesc& desc);
extern void    ExitDevice(Device* pRenderer);

// SwapChain
JRHI_DECL_FUNC(SwapChain*, AddSwapChain, const SwapChainDesc&);
JRHI_DECL_FUNC(void, RemoveSwapChain, SwapChain*);

}