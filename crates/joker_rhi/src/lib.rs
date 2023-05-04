use std::sync::Arc;

#[macro_use]
mod macros;

mod null;
mod vulkan;

#[derive(Copy, Clone, Debug, Eq, PartialEq, Ord, PartialOrd)]
#[repr(i32)]
pub enum EGraphicAPI {
    Null = 0,
    Vulkan = 1,
}

pub fn initialize(desc: &InstanceDesc) -> InstanceRef {
    Arc::new(vulkan::InstanceVulkan::new(desc))
}

pub fn default() -> InstanceRef {
    initialize(&InstanceDesc::default())
}

pub struct InstanceDesc {
    pub api : EGraphicAPI,
    pub app_name: String,
}

impl Default for InstanceDesc {
    fn default() -> Self {
        InstanceDesc {
            api : EGraphicAPI::Vulkan,
            app_name: "joker".to_string(),
        }
    }
}

pub trait Instance: Send + Sync {
    fn new(desc: &InstanceDesc) -> Self
    where
        Self: Sized;

    fn create_device(&self, desc: &DeviceDesc) -> DeviceRef;
}
type InstanceRef = Arc<dyn Instance>;

pub struct DeviceDesc {
    pub width: u32,
    pub height: u32,
}

impl Default for DeviceDesc {
    fn default() -> Self {
        DeviceDesc {
            width: 1280u32,
            height: 720u32,
        }
    }
}

pub trait Device: Send + Sync {
    // fn new()->Self where Self:Sized;
}
type DeviceRef = Arc<dyn Device>;

#[derive(Copy, Clone, Debug, Eq, PartialEq, Ord, PartialOrd)]
#[repr(i32)]
pub enum EPixelFormat {
    Undefined = 0,
    R4G4UniformPack8 = 1,
    R4G4B4A4UniformPack16 = 2,
    B4G4R4A4UinformPack16 = 3,
    R5G6B5UnormPack16 = 4,
    B5G6R5UnormPack16 = 5,
    R5G5B5A1UnormPack16 = 6,
    B5G5R5A1UnormPack16 = 7,
    A1R5G5B5UnormPack16 = 8,
    R8Unorm = 9,
    R8Snorm = 10,
    R8Uscaled = 11,
    R8SScaled = 12,
    R8Uint = 13,
    R8Sint = 14,
    R8SRGB = 15,
    R8G8Unorm = 16,
    R8G8Snorm = 17,
    R8G8Uscaled = 18,
    R8G8SScaled = 19,
    R8G8Uint = 20,
    R8G8Sint = 21,
    R8G8Srgb = 22,
    R8G8B8Unorm = 23,
    R8G8B8Snorm = 24,
    R8G8B8Uscaled = 25,
    R8G8B8SScaled = 26,
    R8G8B8Uint = 27,
    R8G8B8Sint = 28,
    R8G8B8Srgb = 29,
    B8G8R8Unorm = 30,
    B8G8R8Snorm = 31,
    B8G8R8Uscaled = 32,
    B8G8R8Sscaled = 33,
    B8G8R8Uint = 34,
    B8G8R8Sint = 35,
    B8G8R8Srgb = 36,
    R8G8B8A8Unorm = 37,
    R8G8B8A8Snorm = 38,
    R8G8B8A8Uscaled = 39,
    R8G8B8A8Sscaled = 40,
    R8G8B8A8Uint = 41,
    R8G8B8A8Sint = 42,
    R8G8B8A8Srgb = 43,
    B8G8R8A8Unorm = 44,
    B8G8R8A8Snorm = 45,
    B8G8R8A8Uscaled = 46,
    B8G8R8A8Sscaled = 47,
    B8G8R8A8Uint = 48,
    B8G8R8A8Sint = 49,
    B8G8R8A8Srgb = 50,
    A8B8G8R8UnormPack32 = 51,
    A8B8G8R8SnormPack32 = 52,
    A8B8G8R8UscaledPack32 = 53,
    A8B8G8R8SscaledPack32 = 54,
    A8B8G8R8UintPack32 = 55,
    A8B8G8R8SintPack32 = 56,
    A8B8G8R8SrgbPack32 = 57,
    A2R10G10B10UnormPack32 = 58,
    A2R10G10B10SnormPack32 = 59,
    A2R10G10B10UscaledPack32 = 60,
    A2R10G10B10SscaledPack32 = 61,
    A2R10G10B10UintPack32 = 62,
    A2R10G10B10SintPack32 = 63,
    A2B10G10R10UnormPack32 = 64,
    A2B10G10R10SnormPack32 = 65,
    A2B10G10R10UscaledPack32 = 66,
    A2B10G10R10SscaledPack32 = 67,
    A2B10G10R10UintPack32 = 68,
    A2B10G10R10SintPack32 = 69,
    R16Unorm = 70,
    R16Snorm = 71,
    R16Uscaled = 72,
    R16Sscaled = 73,
    R16Uint = 74,
    R16Sint = 75,
    R16Sfloat = 76,
    R16G16Unorm = 77,
    R16G16Snorm = 78,
    R16G16Uscaled = 79,
    R16G16Sscaled = 80,
    R16G16Uint = 81,
    R16G16Sint = 82,
    R16G16Sfloat = 83,
    R16G16B16Unorm = 84,
    R16G16B16Snorm = 85,
    R16G16B16Uscaled = 86,
    R16G16B16Sscaled = 87,
    R16G16B16Uint = 88,
    R16G16B16Sint = 89,
    R16G16B16Sfloat = 90,
    R16G16B16A16Unorm = 91,
    R16G16B16A16Snorm = 92,
    R16G16B16A16Uscaled = 93,
    R16G16B16A16Sscaled = 94,
    R16G16B16A16Uint = 95,
    R16G16B16A16Sint = 96,
    R16G16B16A16Sfloat = 97,
    R32Uint = 98,
    R32Sint = 99,
    R32Sfloat = 100,
    R32G32Uint = 101,
    R32G32Sint = 102,
    R32G32Sfloat = 103,
    R32G32B32Uint = 104,
    R32G32B32Sint = 105,
    R32G32B32Sfloat = 106,
    R32G32B32A32Uint = 107,
    R32G32B32A32Sint = 108,
    R32G32B32A32Sfloat = 109,
    R64Uint = 110,
    R64Sint = 111,
    R64Sfloat = 112,
    R64G64Uint = 113,
    R64G64Sint = 114,
    R64G64Sfloat = 115,
    R64G64B64Uint = 116,
    R64G64B64Sint = 117,
    R64G64B64Sfloat = 118,
    R64G64B64A64Uint = 119,
    R64G64B64A64Sint = 120,
    R64G64B64A64Sfloat = 121,
    B10G11R11UfloatPack32 = 122,
    E5B9G9R9UfloatPack32 = 123,
    D16Unorm = 124,
    X8D24UnormPack32 = 125,
    D32Sfloat = 126,
    S8Uint = 127,
    D16UnormS8Uint = 128,
    D24UnormS8Uint = 129,
    D32SfloatS8Uint = 130,
    BC1RGBUnormBlock = 131,
    BC1RGBSrgbBlock = 132,
    BC1RGBAUnormBlock = 133,
    BC1RGBASrgbBlock = 134,
    BC2UnormBlock = 135,
    BC2SrgbBlock = 136,
    BC3UnormBlock = 137,
    BC3SrgbBlock = 138,
    BC4UnormBlock = 139,
    BC4SnormBlock = 140,
    BC5UnormBlock = 141,
    BC5SnormBlock = 142,
    BC6HUfloatBlock = 143,
    BC6HSfloatBlock = 144,
    BC7UnormBlock = 145,
    BC7SrgbBlock = 146,
    ETC2R8G8B8UnormBlock = 147,
    ETC2R8G8B8SrgbBlock = 148,
    ETC2R8G8B8A1UnormBlock = 149,
    ETC2R8G8B8A1SrgbBlock = 150,
    ETC2R8G8B8A8UnormBlock = 151,
    ETC2R8G8B8A8SrgbBlock = 152,
    EACR11UnormBlock = 153,
    EACR11SnormBlock = 154,
    EACR11G11UnormBlock = 155,
    EACR11G11SnormBlock = 156,
    ASTC4X4UnormBlock = 157,
    ASTC4X4SrgbBlock = 158,
    ASTC5X4UnormBlock = 159,
    ASTC5X4SrgbBlock = 160,
    ASTC5X5UnormBlock = 161,
    ASTC5X5SrgbBlock = 162,
    ASTC6X5UnormBlock = 163,
    ASTC6X5SrgbBlock = 164,
    ASTC6X6UnormBlock = 165,
    ASTC6X6SrgbBlock = 166,
    ASTC8X5UnormBlock = 167,
    ASTC8X5SrgbBlock = 168,
    ASTC8X6UnormBlock = 169,
    ASTC8X6SrgbBlock = 170,
    ASTC8X8UnormBlock = 171,
    ASTC8X8SrgbBlock = 172,
    ASTC10X5UnormBlock = 173,
    ASTC10X5SrgbBlock = 174,
    ASTC10X6UnormBlock = 175,
    ASTC10X6SrgbBlock = 176,
    ASTC10X8UnormnBlock = 177,
    ASTC10X8SrgbBlock = 178,
    ASTC10X10UnormBlock = 179,
    ASTC10X10SrgbBlock = 180,
    ASTC12X10UnormBlock = 181,
    ASTC12X10SrgbBlock = 182,
    ASTC12X12UnormBlock = 183,
    ASTC12X12SrgbBlock = 184,
}

#[test]
fn test_init() {
    //let context = crate::RHI::
}
