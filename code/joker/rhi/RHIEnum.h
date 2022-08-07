#pragma once

enum class ERHIRenderer
{
#if JOPTION_RHI_NULL
	Null,
#endif
#if JOPTION_RHI_VULKAN
	Vulkan,
#endif
};

enum class ERHITextureCreationFlags
{
	None				= 0_bit, // Default flag (Texture will use default allocation strategy decided by the api specific allocator)
	OwnMemoryBit		= 1_bit, // Texture will allocate its own memory (COMMITTED resource)
	ExportBit			= 2_bit, // Texture will be allocated in memory which can be shared among multiple processes
	ExportAdapterBit	= 3_bit, // Texture will be allocated in memory which can be shared among multiple gpus
	ImportBit			= 4_bit, // Texture will be imported from a handle created in another process
	ESRAM				= 5_bit, // Use ESRAM to store this texture
	OnTile				= 6_bit, // Use on-tile memory to store this texture
	Compression			= 7_bit, // Prevent compression meta data from generating (XBox)
	Force2D				= 8_bit, // Force 2D instead of automatically determining dimension based on width, height, depth
	Force3D				= 9_bit, // Force 3D instead of automatically determining dimension based on width, height, depth
	AllowDisplayTarget	= 10_bit, // Display target
	SRGB				= 11_bit, // Create an sRGB texture.
	NormalMap			= 12_bit, // Create a normal map texture
	FastClear			= 13_bit, // Fast clear
	FragMask			= 14_bit, // Fragment mask
	VRMultiview			= 15_bit, // Doubles the amount of array layers of the texture when rendering VR. Also forces the texture to be a 2D Array texture.
	VRFoveatedRendering = 16_bit, // Binds the FFR fragment density if this texture is used as a render target.
};