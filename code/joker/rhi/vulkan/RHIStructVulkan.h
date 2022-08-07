#pragma once

#include <vulkan/vulkan_core.h>
struct RHIRendererContextDescVulkan
{
	const char** m_ppInstanceLayers;
	const char** m_ppInstanceExtensions;
	u32			 m_uInstanceLayerCount;
	u32			 m_uInstanceExtensionCount;
};

struct RHIRendererContextVulkan
{
};

struct RHIRendererDescVulkan
{
	const char** m_ppInstanceLayers;
	const char** m_ppInstanceExtensions;
	const char** m_ppDeviceExtensions;
	u32			 m_uInstanceLayerCount;
	u32			 m_uInstanceExtensionCount;
	u32			 m_uDeviceExtensionCount;
	bool		 m_bRequestAllAvailableQueue; //每种类型的queue请求一个，还是请求全部，影响内存占用约200mb
};

struct RHIRendererVulkan
{
	VkInstance					m_pVkInstance;
	VkPhysicalDevice			m_pVkActiveGPU;
	VkPhysicalDeviceProperties2 m_pVKActiveGPUProperties;
#ifdef JENABLE_DEBUG_UTILS_EXTENSION
	VkDebugUtilsMessengerEXT m_pVkDebugUtilsMessenger;
#else
	VkDebugReportCallbackEXT m_pVkDebugReport;
#endif
	u32**				   m_ppAvailableQueueCount;
	u32**				   m_ppUsedQueueCount;
	VkDescriptorPool	   m_pEmptyDescriptorPool;
	VkDescriptorSetLayout  m_pEmptyDescriptorSetLayout;
	VkDescriptorSet		   m_pEmptyDescriptorSet;
	struct VmaAllocator_T* m_pVmaAllocator;
	u32					   m_uRaytracingSupported				: 1;
	u32					   m_uYCbCrExtension					: 1;
	u32					   m_uKHRSpirv14Extension				: 1;
	u32					   m_uKHRAccelerationStructureExtension : 1;
	u32					   m_uKHRRayQueryExtension				: 1;
	u32					   m_uAMDGCNShaderExtension				: 1;
	u32					   m_uAMDDrawIndirectCountExtensions	: 1;
	u32					   m_uDescriptorIndexingExtension		: 1;
	u32					   m_uBufferDeviceAddressExtension		: 1;
	u32					   m_uDeferredHostOperationsExtension	: 1;
	u32					   m_uDrawIndirectCountExtension		: 1;
	u32					   m_uDedicatedAllocationExtension		: 1;
	u32					   m_uExternalMemoryExtension			: 1;
	u32					   m_uDebugMarkerSupport				: 1;
	u32					   m_uOwnInstance						: 1;
	union
	{
		struct
		{
			u8 m_uGraphicsQueueFamilyIndex;
			u8 m_uTransferQueueFamilyIndex;
			u8 m_uComputeQueueFamilyIndex;
		};
		u8 m_uQueueFamilyIndices[3];
	};
};

struct RHICommandSignatureVulkan
{
};