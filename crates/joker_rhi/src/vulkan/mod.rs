use std::sync::Arc;

use crate::Device;

#[derive(Debug,Clone, Copy)]
pub struct Instance{
}

#[derive(Debug,Clone,Copy)]
pub struct DeviceVulkan{

}


impl crate::Instance for Instance
{
    fn initialize()->Self where Self:Sized {
        Instance {  }
    }

    fn create_device(&self)->crate::DeviceRef {
        Arc::new(DeviceVulkan::new())
    }
}

unsafe impl Send for DeviceVulkan{}
unsafe impl Sync for DeviceVulkan{}

impl crate::Device for DeviceVulkan{
    fn new()->Self where Self:Sized{
        DeviceVulkan {  }
    }
}