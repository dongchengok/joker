#pragma once

namespace joker::rhi
{

struct DeviceDesc
{
    string szAppName;
    u32    uMaxQueueCount = 8;
    bool   bUseAllQueue : 1;
    bool   bCPUDebug    : 1;
    bool   bGPUDebug    : 1;
};

class JRHI_ALIGN Device
{
  public:
    virtual ~Device();

    const DeviceDesc&     GetDesc() const;

    virtual n32           GetGPUCount() const              = 0;
    virtual n32           GetGPUUsingIndex() const         = 0;
    virtual const string& GetGPUName(n32 idx = -1) const   = 0;
    virtual const string& GetGPUVendor(n32 idx = -1) const = 0;
    virtual const string& GetGPUModel(n32 idx = -1) const  = 0;

  protected:
    Device(const DeviceDesc& desc);
    Device() = delete;

  public:
    void* m_pHWContext = nullptr;
    void* m_pHWDevice  = nullptr;

  protected:
    DeviceDesc m_Desc;
};

inline const DeviceDesc& Device::GetDesc() const
{
    return m_Desc;
}

}