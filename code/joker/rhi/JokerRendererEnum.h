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

namespace joker
{
enum class ERenderer
{
    Null,
    Vulkan,
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

enum class ETextureDimension
{
    D1,
    D2,
    D2MS,
    D3,
    Cube,
    D1Array,
    D2Array,
    D2MSArray,
    CubeArray,
    Count,
    Undefined,
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

}