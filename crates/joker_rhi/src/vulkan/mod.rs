use ash::extensions::{
    ext::DebugUtils,
    khr::{Surface, Swapchain},
};
use ash::*;
use std::ffi::CStr;
use std::ffi::CString;
use std::sync::Arc;

use raw_window_handle::{HasRawDisplayHandle, HasRawWindowHandle};
use winit::{
    event::{ElementState, Event, KeyboardInput, VirtualKeyCode, WindowEvent},
    event_loop::{ControlFlow, EventLoop},
    platform::run_return::EventLoopExtRunReturn,
    window::{self, WindowBuilder},
};

use crate::Device;
use crate::DeviceDesc;
use crate::InstanceDesc;

pub struct InstanceVulkan {
    entry: Entry,
    instance: Instance,
}

impl crate::Instance for InstanceVulkan {
    fn new(desc: &InstanceDesc) -> Self
    where
        Self: Sized,
    {
        let entry = Entry::linked();
        let app_name =
            CString::new(desc.app_name.clone()).unwrap_or(CString::new("joker_app").unwrap());

        let app_info = vk::ApplicationInfo {
            p_application_name: app_name.as_ptr(),
            application_version: 0,
            p_engine_name: unsafe { CStr::from_bytes_with_nul_unchecked(b"joker\0").as_ptr() },
            engine_version: 0,
            api_version: 0,
            ..Default::default()
        };

        let create_flags = if cfg!(any(target_os = "macos", target_os = "ios")) {
            vk::InstanceCreateFlags::ENUMERATE_PORTABILITY_KHR
        } else {
            vk::InstanceCreateFlags::default()
        };

        let create_info = vk::InstanceCreateInfo::builder()
            .application_info(&app_info)
            .flags(create_flags)
            .build();

        let instance = unsafe {
            entry
                .create_instance(&create_info, Option::None)
                .expect("create vulkan instance failed!")
        };

        InstanceVulkan { entry, instance }
    }

    fn create_device(&self, desc: &DeviceDesc) -> crate::DeviceRef {
        let event_loop = EventLoop::new();
        let window = WindowBuilder::new()
            .with_title("Ash - Example")
            .with_inner_size(winit::dpi::LogicalSize::new(
                f64::from(desc.width),
                f64::from(desc.height),
            ))
            .build(&event_loop)
            .unwrap();

        let surface = unsafe {
            ash_window::create_surface(
                &self.entry,
                &self.instance,
                window.raw_display_handle(),
                window.raw_window_handle(),
                None,
            )
            .unwrap()
        };

        let pdevices = unsafe {
            self.instance
                .enumerate_physical_devices()
                .expect("Physical device error")
        };

        let surface_loader = Surface::new(&self.entry, &self.instance);
        let (pdevice, queue_family_index) = unsafe {
            pdevices
                .iter()
                .find_map(|pdevice| {
                    self.instance
                        .get_physical_device_queue_family_properties(*pdevice)
                        .iter()
                        .enumerate()
                        .find_map(|(index, info)| {
                            let supports_graphic_and_surface =
                                info.queue_flags.contains(vk::QueueFlags::GRAPHICS)
                                    && surface_loader
                                        .get_physical_device_surface_support(
                                            *pdevice,
                                            index as u32,
                                            surface,
                                        )
                                        .unwrap();
                            if supports_graphic_and_surface {
                                Some((*pdevice, index))
                            } else {
                                None
                            }
                        })
                })
                .expect("Couldn't find suitable device.")
        };

        let queue_family_index = queue_family_index as u32;
        let device_extension_names_raw = [
            Swapchain::name().as_ptr(),
            #[cfg(any(target_os = "macos", target_os = "ios"))]
            KhrPortabilitySubsetFn::NAME.as_ptr(),
        ];

        let features = vk::PhysicalDeviceFeatures {
            shader_clip_distance: 1,
            ..Default::default()
        };

        let priorities = [1.0];

        let queue_info = vk::DeviceQueueCreateInfo::builder()
            .queue_family_index(queue_family_index)
            .queue_priorities(&priorities)
            .build();

        let device_create_info = vk::DeviceCreateInfo::builder()
            .queue_create_infos(std::slice::from_ref(&queue_info))
            .enabled_extension_names(&device_extension_names_raw)
            .enabled_features(&features)
            .build();

        let device = unsafe {
            self.instance
                .create_device(pdevice, &device_create_info, None)
                .unwrap()
        };

        // let device = DeviceVulkan {
        //     device_physical: vk::PhysicalDevice::default(),
        //     device_logic: vk::Device::null(),
        // };
        Arc::new(DeviceVulkan {
            device_physical: pdevice,
            device_logic: device,
        })
    }
}

pub struct DeviceVulkan {
    device_physical: vk::PhysicalDevice,
    device_logic: ash::Device,
}

unsafe impl Send for DeviceVulkan {}
unsafe impl Sync for DeviceVulkan {}

impl crate::Device for DeviceVulkan {
    // fn new() -> Self
    // where
    //     Self: Sized,
    // {
    //     DeviceVulkan {
    //         device_physical: vk::PhysicalDevice::default(),
    //         device_logic: vk::Device::default(),
    //     }
    // }
}
