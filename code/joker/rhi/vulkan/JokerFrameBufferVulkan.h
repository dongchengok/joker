#pragma once
#include "JokerVulkan.h"

namespace joker::rhi::vulkan
{

class JRHI_ALIGN FrameBufferVulkan
{
public:
    static VkFramebuffer GetOrCreate();

private:

};

}