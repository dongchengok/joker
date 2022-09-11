#include "JokerRHIPCH.h"
#include "JokerDevice.h"

namespace joker::rhi
{

Device::Device(const DeviceDesc& desc)
:m_Desc(desc)
{
}

Device::~Device()
{
}

}