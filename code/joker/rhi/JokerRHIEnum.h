#pragma once

#define JMAKE_ENUM_FLAG(ENUM_TYPE)                                                                                                                                                 \
    static inline ENUM_TYPE operator|(ENUM_TYPE a, ENUM_TYPE b)                                                                                                                    \
    {                                                                                                                                                                              \
        return (ENUM_TYPE)((u32)(a) | (u32)(b));                                                                                                                                   \
    }                                                                                                                                                                              \
    static inline ENUM_TYPE operator&(ENUM_TYPE a, ENUM_TYPE b)                                                                                                                    \
    {                                                                                                                                                                              \
        return (ENUM_TYPE)((u32)(a) & (u32)(b));                                                                                                                                   \
    }                                                                                                                                                                              \
    static inline ENUM_TYPE operator|(u32 a, ENUM_TYPE b)                                                                                                                          \
    {                                                                                                                                                                              \
        return (ENUM_TYPE)((u32)(a) | (u32)(b));                                                                                                                                   \
    }                                                                                                                                                                              \
    static inline ENUM_TYPE operator&(u32 a, ENUM_TYPE b)                                                                                                                          \
    {                                                                                                                                                                              \
        return (ENUM_TYPE)((u32)(a) & (u32)(b));                                                                                                                                   \
    }                                                                                                                                                                              \
    static inline ENUM_TYPE operator|(ENUM_TYPE a, u32 b)                                                                                                                          \
    {                                                                                                                                                                              \
        return (ENUM_TYPE)((u32)(a) | (u32)(b));                                                                                                                                   \
    }                                                                                                                                                                              \
    static inline ENUM_TYPE operator&(ENUM_TYPE a, u32 b)                                                                                                                          \
    {                                                                                                                                                                              \
        return (ENUM_TYPE)((u32)(a) & (u32)(b));                                                                                                                                   \
    }                                                                                                                                                                              \
    static inline ENUM_TYPE operator|=(ENUM_TYPE& a, ENUM_TYPE b)                                                                                                                  \
    {                                                                                                                                                                              \
        return a = (a | b);                                                                                                                                                        \
    }                                                                                                                                                                              \
    static inline ENUM_TYPE operator&=(ENUM_TYPE& a, ENUM_TYPE b)                                                                                                                  \
    {                                                                                                                                                                              \
        return a = (a & b);                                                                                                                                                        \
    }                                                                                                                                                                              \
    static inline ENUM_TYPE operator|=(ENUM_TYPE& a, u32 b)                                                                                                                        \
    {                                                                                                                                                                              \
        return a = (a | b);                                                                                                                                                        \
    }                                                                                                                                                                              \
    static inline ENUM_TYPE operator&=(ENUM_TYPE& a, u32 b)                                                                                                                        \
    {                                                                                                                                                                              \
        return a = (a & b);                                                                                                                                                        \
    }                                                                                                                                                                              \
    static inline bool any(ENUM_TYPE e)                                                                                                                                            \
    {                                                                                                                                                                              \
        return e != (ENUM_TYPE)0;                                                                                                                                                  \
    }

namespace joker::rhi
{
enum class ERenderer
{
    Null,
    Vulkan,
};

enum class EColorSpace
{
    sRGB,
    HDR10,
    P3,
    BT709,
    BT2020,
    AdobeRGB,
    DolbVision,
};

enum class EQueueType
{
    Graphics,
    Compute,
    Transfer,
};

enum class EGPUType
{
    Other          = 0,
    IntergratedGPU = 1,
    DiscreteGPU    = 2,
    VirtualGPU     = 3,
    CPU            = 4,
    Unknow         = 5,
};

enum class ERHITextureCreationFlags
{
    None                = 0_bit,  // Default flag (Texture will use default allocation strategy decided by the api specific allocator)
    OwnMemoryBit        = 1_bit,  // Texture will allocate its own memory (COMMITTED resource)
    ExportBit           = 2_bit,  // Texture will be allocated in memory which can be shared among multiple processes
    ExportAdapterBit    = 3_bit,  // Texture will be allocated in memory which can be shared among multiple gpus
    ImportBit           = 4_bit,  // Texture will be imported from a handle created in another process
    ESRAM               = 5_bit,  // Use ESRAM to store this texture
    OnTile              = 6_bit,  // Use on-tile memory to store this texture
    Compression         = 7_bit,  // Prevent compression meta data from generating (XBox)
    Force2D             = 8_bit,  // Force 2D instead of automatically determining dimension based on width, height, depth
    Force3D             = 9_bit,  // Force 3D instead of automatically determining dimension based on width, height, depth
    AllowDisplayTarget  = 10_bit, // Display target
    SRGB                = 11_bit, // Create an sRGB texture.
    NormalMap           = 12_bit, // Create a normal map texture
    FastClear           = 13_bit, // Fast clear
    FragMask            = 14_bit, // Fragment mask
    VRMultiview         = 15_bit, // Doubles the amount of array layers of the texture when rendering VR. Also forces the texture to be a 2D Array texture.
    VRFoveatedRendering = 16_bit, // Binds the FFR fragment density if this texture is used as a render target.
};

enum class EWaveOpsSupportFlags
{
    None               = 0_bit,
    BasicBit           = 1_bit,
    VoteBit            = 2_bit,
    ArithmeticBit      = 3_bit,
    BallotBit          = 4_bit,
    ShuffleBit         = 5_bit,
    ShuffleRelativeBit = 6_bit,
    ClusteredBit       = 7_bit,
    QuadBit            = 8_bit,
    PartitionedBitNV   = 9_bit,
    All                = 0x7FFFFFFF,
};
JMAKE_ENUM_FLAG(EWaveOpsSupportFlags);

enum class EGPUPresetLevel
{
    None = 0,
    Office,
    Low,
    Medium,
    High,
    Ultra,
    Count,
};

enum class EShadingRate
{
    NotSupported = 0_bit,
    Full         = 1_bit,
    Half         = 2_bit,
    Quarter      = 3_bit,
    Eighth       = 4_bit,
    _1X2         = 5_bit,
    _2X1         = 6_bit,
    _2X4         = 7_bit,
    _4X2         = 8_bit,
};

enum class EShadingRateCaps
{
    NotSupported = 0_bit,
    PerDraw      = 1_bit,
    PerTile      = 2_bit,
};

enum class EGPUVendor
{
    Nvidia,
    Amd,
    Intel,
    Unknown,
    Count,
};

enum class ETextureDimension : u8
{
    Texture1D,
    Texture1DArray,
    Texture2D,
    Texture2DArray,
    Texture3D,
    TextureCube,
    TextureCubeArray,
    Undefined,
};

enum class ETextureCreateFlags : u32
{
    None               = 0_bit,
    RenderTarget       = 1_bit,
    ResolveTarget      = 2_bit,
    DepthStencilTarget = 3_bit,
    ShaderResource     = 4_bit,
    sRGB               = 5_bit,
    MemoryLess         = 6_bit,
};

enum class EGPUMode
{
    Single = 0,
    Linked,
    Unlinked,
};

enum class EShaderMode
{
    SM_5_0,
    SM_5_1,
    SM_6_0,
    SM_6_1,
    SM_6_2,
    SM_6_3, //光追需要
    SM_6_4, // VRS需要
};

enum class ELoadAction : u8
{
    Load     = 0,
    Clear    = 1,
    NoAction = 2,
};

enum class EStoreAction : u8
{
    Store    = 0,
    NoAction = 1,
};

enum class ESubpassHint : u8
{
    None,
    DepthRead,
    DeferredShading,
};

enum class EPixelFormat
{
    UNDEFINED                                     = 0,
    R4G4_UNORM_PACK8                              = 1,
    R4G4B4A4_UNORM_PACK16                         = 2,
    B4G4R4A4_UNORM_PACK16                         = 3,
    R5G6B5_UNORM_PACK16                           = 4,
    B5G6R5_UNORM_PACK16                           = 5,
    R5G5B5A1_UNORM_PACK16                         = 6,
    B5G5R5A1_UNORM_PACK16                         = 7,
    A1R5G5B5_UNORM_PACK16                         = 8,
    R8_UNORM                                      = 9,
    R8_SNORM                                      = 10,
    R8_USCALED                                    = 11,
    R8_SSCALED                                    = 12,
    R8_UINT                                       = 13,
    R8_SINT                                       = 14,
    R8_SRGB                                       = 15,
    R8G8_UNORM                                    = 16,
    R8G8_SNORM                                    = 17,
    R8G8_USCALED                                  = 18,
    R8G8_SSCALED                                  = 19,
    R8G8_UINT                                     = 20,
    R8G8_SINT                                     = 21,
    R8G8_SRGB                                     = 22,
    R8G8B8_UNORM                                  = 23,
    R8G8B8_SNORM                                  = 24,
    R8G8B8_USCALED                                = 25,
    R8G8B8_SSCALED                                = 26,
    R8G8B8_UINT                                   = 27,
    R8G8B8_SINT                                   = 28,
    R8G8B8_SRGB                                   = 29,
    B8G8R8_UNORM                                  = 30,
    B8G8R8_SNORM                                  = 31,
    B8G8R8_USCALED                                = 32,
    B8G8R8_SSCALED                                = 33,
    B8G8R8_UINT                                   = 34,
    B8G8R8_SINT                                   = 35,
    B8G8R8_SRGB                                   = 36,
    R8G8B8A8_UNORM                                = 37,
    R8G8B8A8_SNORM                                = 38,
    R8G8B8A8_USCALED                              = 39,
    R8G8B8A8_SSCALED                              = 40,
    R8G8B8A8_UINT                                 = 41,
    R8G8B8A8_SINT                                 = 42,
    R8G8B8A8_SRGB                                 = 43,
    B8G8R8A8_UNORM                                = 44,
    B8G8R8A8_SNORM                                = 45,
    B8G8R8A8_USCALED                              = 46,
    B8G8R8A8_SSCALED                              = 47,
    B8G8R8A8_UINT                                 = 48,
    B8G8R8A8_SINT                                 = 49,
    B8G8R8A8_SRGB                                 = 50,
    A8B8G8R8_UNORM_PACK32                         = 51,
    A8B8G8R8_SNORM_PACK32                         = 52,
    A8B8G8R8_USCALED_PACK32                       = 53,
    A8B8G8R8_SSCALED_PACK32                       = 54,
    A8B8G8R8_UINT_PACK32                          = 55,
    A8B8G8R8_SINT_PACK32                          = 56,
    A8B8G8R8_SRGB_PACK32                          = 57,
    A2R10G10B10_UNORM_PACK32                      = 58,
    A2R10G10B10_SNORM_PACK32                      = 59,
    A2R10G10B10_USCALED_PACK32                    = 60,
    A2R10G10B10_SSCALED_PACK32                    = 61,
    A2R10G10B10_UINT_PACK32                       = 62,
    A2R10G10B10_SINT_PACK32                       = 63,
    A2B10G10R10_UNORM_PACK32                      = 64,
    A2B10G10R10_SNORM_PACK32                      = 65,
    A2B10G10R10_USCALED_PACK32                    = 66,
    A2B10G10R10_SSCALED_PACK32                    = 67,
    A2B10G10R10_UINT_PACK32                       = 68,
    A2B10G10R10_SINT_PACK32                       = 69,
    R16_UNORM                                     = 70,
    R16_SNORM                                     = 71,
    R16_USCALED                                   = 72,
    R16_SSCALED                                   = 73,
    R16_UINT                                      = 74,
    R16_SINT                                      = 75,
    R16_SFLOAT                                    = 76,
    R16G16_UNORM                                  = 77,
    R16G16_SNORM                                  = 78,
    R16G16_USCALED                                = 79,
    R16G16_SSCALED                                = 80,
    R16G16_UINT                                   = 81,
    R16G16_SINT                                   = 82,
    R16G16_SFLOAT                                 = 83,
    R16G16B16_UNORM                               = 84,
    R16G16B16_SNORM                               = 85,
    R16G16B16_USCALED                             = 86,
    R16G16B16_SSCALED                             = 87,
    R16G16B16_UINT                                = 88,
    R16G16B16_SINT                                = 89,
    R16G16B16_SFLOAT                              = 90,
    R16G16B16A16_UNORM                            = 91,
    R16G16B16A16_SNORM                            = 92,
    R16G16B16A16_USCALED                          = 93,
    R16G16B16A16_SSCALED                          = 94,
    R16G16B16A16_UINT                             = 95,
    R16G16B16A16_SINT                             = 96,
    R16G16B16A16_SFLOAT                           = 97,
    R32_UINT                                      = 98,
    R32_SINT                                      = 99,
    R32_SFLOAT                                    = 100,
    R32G32_UINT                                   = 101,
    R32G32_SINT                                   = 102,
    R32G32_SFLOAT                                 = 103,
    R32G32B32_UINT                                = 104,
    R32G32B32_SINT                                = 105,
    R32G32B32_SFLOAT                              = 106,
    R32G32B32A32_UINT                             = 107,
    R32G32B32A32_SINT                             = 108,
    R32G32B32A32_SFLOAT                           = 109,
    R64_UINT                                      = 110,
    R64_SINT                                      = 111,
    R64_SFLOAT                                    = 112,
    R64G64_UINT                                   = 113,
    R64G64_SINT                                   = 114,
    R64G64_SFLOAT                                 = 115,
    R64G64B64_UINT                                = 116,
    R64G64B64_SINT                                = 117,
    R64G64B64_SFLOAT                              = 118,
    R64G64B64A64_UINT                             = 119,
    R64G64B64A64_SINT                             = 120,
    R64G64B64A64_SFLOAT                           = 121,
    B10G11R11_UFLOAT_PACK32                       = 122,
    E5B9G9R9_UFLOAT_PACK32                        = 123,
    D16_UNORM                                     = 124,
    X8_D24_UNORM_PACK32                           = 125,
    D32_SFLOAT                                    = 126,
    S8_UINT                                       = 127,
    D16_UNORM_S8_UINT                             = 128,
    D24_UNORM_S8_UINT                             = 129,
    D32_SFLOAT_S8_UINT                            = 130,
    BC1_RGB_UNORM_BLOCK                           = 131,
    BC1_RGB_SRGB_BLOCK                            = 132,
    BC1_RGBA_UNORM_BLOCK                          = 133,
    BC1_RGBA_SRGB_BLOCK                           = 134,
    BC2_UNORM_BLOCK                               = 135,
    BC2_SRGB_BLOCK                                = 136,
    BC3_UNORM_BLOCK                               = 137,
    BC3_SRGB_BLOCK                                = 138,
    BC4_UNORM_BLOCK                               = 139,
    BC4_SNORM_BLOCK                               = 140,
    BC5_UNORM_BLOCK                               = 141,
    BC5_SNORM_BLOCK                               = 142,
    BC6H_UFLOAT_BLOCK                             = 143,
    BC6H_SFLOAT_BLOCK                             = 144,
    BC7_UNORM_BLOCK                               = 145,
    BC7_SRGB_BLOCK                                = 146,
    ETC2_R8G8B8_UNORM_BLOCK                       = 147,
    ETC2_R8G8B8_SRGB_BLOCK                        = 148,
    ETC2_R8G8B8A1_UNORM_BLOCK                     = 149,
    ETC2_R8G8B8A1_SRGB_BLOCK                      = 150,
    ETC2_R8G8B8A8_UNORM_BLOCK                     = 151,
    ETC2_R8G8B8A8_SRGB_BLOCK                      = 152,
    EAC_R11_UNORM_BLOCK                           = 153,
    EAC_R11_SNORM_BLOCK                           = 154,
    EAC_R11G11_UNORM_BLOCK                        = 155,
    EAC_R11G11_SNORM_BLOCK                        = 156,
    ASTC_4x4_UNORM_BLOCK                          = 157,
    ASTC_4x4_SRGB_BLOCK                           = 158,
    ASTC_5x4_UNORM_BLOCK                          = 159,
    ASTC_5x4_SRGB_BLOCK                           = 160,
    ASTC_5x5_UNORM_BLOCK                          = 161,
    ASTC_5x5_SRGB_BLOCK                           = 162,
    ASTC_6x5_UNORM_BLOCK                          = 163,
    ASTC_6x5_SRGB_BLOCK                           = 164,
    ASTC_6x6_UNORM_BLOCK                          = 165,
    ASTC_6x6_SRGB_BLOCK                           = 166,
    ASTC_8x5_UNORM_BLOCK                          = 167,
    ASTC_8x5_SRGB_BLOCK                           = 168,
    ASTC_8x6_UNORM_BLOCK                          = 169,
    ASTC_8x6_SRGB_BLOCK                           = 170,
    ASTC_8x8_UNORM_BLOCK                          = 171,
    ASTC_8x8_SRGB_BLOCK                           = 172,
    ASTC_10x5_UNORM_BLOCK                         = 173,
    ASTC_10x5_SRGB_BLOCK                          = 174,
    ASTC_10x6_UNORM_BLOCK                         = 175,
    ASTC_10x6_SRGB_BLOCK                          = 176,
    ASTC_10x8_UNORM_BLOCK                         = 177,
    ASTC_10x8_SRGB_BLOCK                          = 178,
    ASTC_10x10_UNORM_BLOCK                        = 179,
    ASTC_10x10_SRGB_BLOCK                         = 180,
    ASTC_12x10_UNORM_BLOCK                        = 181,
    ASTC_12x10_SRGB_BLOCK                         = 182,
    ASTC_12x12_UNORM_BLOCK                        = 183,
    ASTC_12x12_SRGB_BLOCK                         = 184,
    G8B8G8R8_422_UNORM                            = 1000156000,
    B8G8R8G8_422_UNORM                            = 1000156001,
    G8_B8_R8_3PLANE_420_UNORM                     = 1000156002,
    G8_B8R8_2PLANE_420_UNORM                      = 1000156003,
    G8_B8_R8_3PLANE_422_UNORM                     = 1000156004,
    G8_B8R8_2PLANE_422_UNORM                      = 1000156005,
    G8_B8_R8_3PLANE_444_UNORM                     = 1000156006,
    R10X6_UNORM_PACK16                            = 1000156007,
    R10X6G10X6_UNORM_2PACK16                      = 1000156008,
    R10X6G10X6B10X6A10X6_UNORM_4PACK16            = 1000156009,
    G10X6B10X6G10X6R10X6_422_UNORM_4PACK16        = 1000156010,
    B10X6G10X6R10X6G10X6_422_UNORM_4PACK16        = 1000156011,
    G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16    = 1000156012,
    G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16     = 1000156013,
    G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16    = 1000156014,
    G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16     = 1000156015,
    G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16    = 1000156016,
    R12X4_UNORM_PACK16                            = 1000156017,
    R12X4G12X4_UNORM_2PACK16                      = 1000156018,
    R12X4G12X4B12X4A12X4_UNORM_4PACK16            = 1000156019,
    G12X4B12X4G12X4R12X4_422_UNORM_4PACK16        = 1000156020,
    B12X4G12X4R12X4G12X4_422_UNORM_4PACK16        = 1000156021,
    G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16    = 1000156022,
    G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16     = 1000156023,
    G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16    = 1000156024,
    G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16     = 1000156025,
    G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16    = 1000156026,
    G16B16G16R16_422_UNORM                        = 1000156027,
    B16G16R16G16_422_UNORM                        = 1000156028,
    G16_B16_R16_3PLANE_420_UNORM                  = 1000156029,
    G16_B16R16_2PLANE_420_UNORM                   = 1000156030,
    G16_B16_R16_3PLANE_422_UNORM                  = 1000156031,
    G16_B16R16_2PLANE_422_UNORM                   = 1000156032,
    G16_B16_R16_3PLANE_444_UNORM                  = 1000156033,
    PVRTC1_2BPP_UNORM_BLOCK_IMG                   = 1000054000,
    PVRTC1_4BPP_UNORM_BLOCK_IMG                   = 1000054001,
    PVRTC2_2BPP_UNORM_BLOCK_IMG                   = 1000054002,
    PVRTC2_4BPP_UNORM_BLOCK_IMG                   = 1000054003,
    PVRTC1_2BPP_SRGB_BLOCK_IMG                    = 1000054004,
    PVRTC1_4BPP_SRGB_BLOCK_IMG                    = 1000054005,
    PVRTC2_2BPP_SRGB_BLOCK_IMG                    = 1000054006,
    PVRTC2_4BPP_SRGB_BLOCK_IMG                    = 1000054007,
    ASTC_4x4_SFLOAT_BLOCK_EXT                     = 1000066000,
    ASTC_5x4_SFLOAT_BLOCK_EXT                     = 1000066001,
    ASTC_5x5_SFLOAT_BLOCK_EXT                     = 1000066002,
    ASTC_6x5_SFLOAT_BLOCK_EXT                     = 1000066003,
    ASTC_6x6_SFLOAT_BLOCK_EXT                     = 1000066004,
    ASTC_8x5_SFLOAT_BLOCK_EXT                     = 1000066005,
    ASTC_8x6_SFLOAT_BLOCK_EXT                     = 1000066006,
    ASTC_8x8_SFLOAT_BLOCK_EXT                     = 1000066007,
    ASTC_10x5_SFLOAT_BLOCK_EXT                    = 1000066008,
    ASTC_10x6_SFLOAT_BLOCK_EXT                    = 1000066009,
    ASTC_10x8_SFLOAT_BLOCK_EXT                    = 1000066010,
    ASTC_10x10_SFLOAT_BLOCK_EXT                   = 1000066011,
    ASTC_12x10_SFLOAT_BLOCK_EXT                   = 1000066012,
    ASTC_12x12_SFLOAT_BLOCK_EXT                   = 1000066013,
    G8_B8R8_2PLANE_444_UNORM_EXT                  = 1000330000,
    G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT = 1000330001,
    G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT = 1000330002,
    G16_B16R16_2PLANE_444_UNORM_EXT               = 1000330003,
    A4R4G4B4_UNORM_PACK16_EXT                     = 1000340000,
    A4B4G4R4_UNORM_PACK16_EXT                     = 1000340001,
};

enum class ESampleCount : u8
{
    X1  = 1,
    X2  = 2,
    X4  = 4,
    X8  = 8,
    X16 = 16,
    X32 = 32,
    X64 = 64,
};

}