#pragma once

enum class ETextureCreationFlags
{
	NONE				  = 0_bit, // Default flag (Texture will use default allocation strategy decided by the api specific allocator)
	OWN_MEMORY_BIT		  = 1_bit, // Texture will allocate its own memory (COMMITTED resource)
	EXPORT_BIT			  = 2_bit, // Texture will be allocated in memory which can be shared among multiple processes
	EXPORT_ADAPTER_BIT	  = 3_bit, // Texture will be allocated in memory which can be shared among multiple gpus
	IMPORT_BIT			  = 4_bit, // Texture will be imported from a handle created in another process
	ESRAM				  = 5_bit, // Use ESRAM to store this texture
	ON_TILE				  = 6_bit, // Use on-tile memory to store this texture
	COMPRESSION			  = 7_bit, // Prevent compression meta data from generating (XBox)
	FORCE_2D			  = 8_bit, // Force 2D instead of automatically determining dimension based on width, height, depth
	FORCE_3D			  = 9_bit, // Force 3D instead of automatically determining dimension based on width, height, depth
	ALLOW_DISPLAY_TARGET  = 10_bit, // Display target
	SRGB				  = 11_bit, // Create an sRGB texture.
	NORMAL_MAP			  = 12_bit, // Create a normal map texture
	FAST_CLEAR			  = 13_bit, // Fast clear
	FRAG_MASK			  = 14_bit, // Fragment mask
	VR_MULTIVIEW		  = 15_bit, // Doubles the amount of array layers of the texture when rendering VR. Also forces the texture to be a 2D Array texture.
	VR_FOVEATED_RENDERING = 16_bit, // Binds the FFR fragment density if this texture is used as a render target.
};