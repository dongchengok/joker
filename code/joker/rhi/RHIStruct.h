#pragma once

#include "CoreType.h"
#include "RHIEnum.h"
// #include "RHIType.h"
// #if (defined(JOPTION_RHI_MULTI) && JOPTION_RHI_NULL) || (JOPTION_RHI_API == JOPTION_RHI_NULL)
// #include "null/RHIStructNull.h"
// #define JDECL_RHI_PROP_NULL(type) type##Null Null;
// #else
// #define JDECL_RHI_PROP_NULL(type)
// #endif

// #if (defined(JOPTION_RHI_MULTI) && JOPTION_RHI_VULKAN) || (JOPTION_RHI_API == JOPTION_RHI_VULKAN)
// #include "vulkan/RHIStructVulkan.h"
// #define JDECL_RHI_PROP_VULKAN(type) type##Vulkan Vulkan;
// #else
// #define JDECL_RHI_PROP_VULKAN(type)
// #endif

// #if defined(JOPTION_RHI_MULTI)
// #define JDECL_RHI_STRUCT_BEGIN(type)                                                                                                                 \
//     struct type                                                                                                                                      \
//     {                                                                                                                                                \
//         JDECL_RHI_PROP_NULL(type)                                                                                                                    \
//         JDECL_RHI_PROP_VULKAN(type)

// #define JDECL_RHI_STRUCT_ALIGNED_BEGIN(type, a)                                                                                                      \
//     struct alignas(a) type                                                                                                                           \
//     {                                                                                                                                                \
//         JDECL_RHI_PROP_NULL(type)                                                                                                                    \
//         JDECL_RHI_PROP_VULKAN(type)
// #define JDECL_RHI_STRUCT_END                                                                                                                         \
//     }                                                                                                                                                \
//     ;
// #else
// #if (JOPTION_RHI_API == JOPTION_RHI_NULL)
// #define JDECL_RHI_PROP JDECL_RHI_PROP_NULL
// #elif (JOPTION_RHI_API == JOPTION_RHI_VULKAN)
// #define JDECL_RHI_PROP JDECL_RHI_PROP_VULKAN
// #endif

// #define JDECL_RHI_STRUCT_BEGIN(type)                                                                                                                 \
//     struct type                                                                                                                                      \
//     {                                                                                                                                                \
//         JDECL_RHI_PROP(type)

// #define JDECL_RHI_STRUCT_ALIGNED_BEGIN(type, a)                                                                                                      \
//     struct alignas(a) type                                                                                                                           \
//     {                                                                                                                                                \
//         JDECL_RHI_PROP(type)
// #define JDECL_RHI_STRUCT_END                                                                                                                         \
//     }                                                                                                                                                \
//     ;
// #endif

// struct RHINullDescriptors
// {
//     RHITexture* m_pDefaultTextureSRV[RHIConst::kMaxLinkedGPUs][(u32)ERHITextureDimension::Count];
//     RHITexture* m_pDefaultTextureUAV[RHIConst::kMaxLinkedGPUs][(u32)ERHITextureDimension::Count];
//     RHIBuffer*  m_pDefaultBufferSRV[RHIConst::kMaxLinkedGPUs];
//     RHIBuffer*  m_pDefaultBufferUAV[RHIConst::kMaxLinkedGPUs];
//     RHISampler* m_pDefaultSampler;

//     // Mutex  m_SubmitMutex;
//     // Mutex m_InitialTransitionMutex;
//     RHIQueue*   m_pInitialTransitionQueue;
//     RHICmdPool* m_pInitialTransitionCmdPool;
//     RHICmd*     m_pInitialTransitionCmd;
//     RHIFence*   m_pInitialTransitionFence;
// };

// struct RHIGPUVendorPreset
// {
//     char               m_szVenderID[VK_MAX_DESCRIPTION_SIZE];
//     char               m_szModelID[VK_MAX_DESCRIPTION_SIZE];
//     char               m_szRevisionID[VK_MAX_DESCRIPTION_SIZE];
//     ERHIGPUPresetLevel m_ePresetLevel;
//     char               m_szGPUName[VK_MAX_DESCRIPTION_SIZE];
//     char               m_szGPUDriverVersion[VK_MAX_DESCRIPTION_SIZE];
//     char               m_szGPUDriverDate[VK_MAX_DESCRIPTION_SIZE];
// };

// struct RHIGPUSettings
// {
//     u32                     m_uUniformBufferAlignment;
//     u32                     m_uUploadBufferTextureAlignment;
//     u32                     m_uUploadBufferTextureRowAlignment;
//     u32                     m_uMaxVertexInputBindings;
//     u32                     m_uMaxRootSignatureDWORDS;
//     u32                     m_uWaveLaneCount;
//     ERHIWaveOpsSupportFlags m_eWaveOpsSupportFlags;
//     RHIGPUVendorPreset      m_GPUVendorPreset;

//     ERHIShadingRate         m_eShadingRate;
//     ERHIShadingRateCaps     m_eShadingRateCaps;
//     u32                     m_uShadingRateTexelWidth;
//     u32                     m_uShadingRateTexelHeight;
//     u32                     m_uMultiDrawIndirect       : 1;
//     u32                     m_uROVsSupported           : 1;
//     u32                     m_uTessellationSupported   : 1;
//     u32                     m_uGeometryShaderSupported : 1;
//     u32                     m_uGPUBreadcrumbs          : 1;
//     u32                     m_uHDRSupported            : 1;
// };

// JDECL_RHI_STRUCT_BEGIN(RHIGPUInfo)
// RHIGPUSettings m_Settings;
// JDECL_RHI_STRUCT_END

// JDECL_RHI_STRUCT_BEGIN(RHIRendererContextDesc)
// ERHIRenderer m_eRenderer;
// bool         m_bEnableGPUBasedValidation;
// JDECL_RHI_STRUCT_END

// JDECL_RHI_STRUCT_ALIGNED_BEGIN(RHIRendererContext, 64)
// RHIGPUInfo*  m_pGPUs;
// u32          m_uGPUCount;
// ERHIRenderer m_eRenderer;
// u32          m_uRendererCount;
// JDECL_RHI_STRUCT_END

// JDECL_RHI_STRUCT_BEGIN(RHIRendererDesc)
// ERHIRenderer        m_eRenderer;
// ERHIShaderMode      m_eShaderMode;
// ERHIGPUMode         m_eGPUMode;
// RHIRendererContext* m_pRenderContext;
// u32                 m_uGPUIndex;
// //这是个基于GPU的验证，很慢，会打补丁到shader里
// bool m_bEnableGPUBasedValidation;
// JDECL_RHI_STRUCT_END

// JDECL_RHI_STRUCT_ALIGNED_BEGIN(RHIRenderer, 64)
// struct RHINullDescriptors* m_pNullDescriptors;
// char*                      m_szName;
// struct RHIGPUSettings*     m_pActiveGpuSettings;
// struct RHIShaderMacro*     m_pBuiltinShaderDefines;
// struct RHIGPUCapBits*      m_pCapBits;
// u32                        m_uLinkedNodeCount         : 4;
// u32                        m_uUnlinkedRenderIndex     : 4;
// ERHIGPUMode                m_eGPUMode                 : 3;
// ERHIShaderMode             m_eShaderMode              : 4;
// bool                       m_bEnableGpuBaseValidation : 1;
// char*                      szApiName;
// u32                        m_uBuiltinShaderDefinesCount;
// ERHIRenderer               m_eRenderer;
// JDECL_RHI_STRUCT_END

// JDECL_RHI_STRUCT_BEGIN(RHICommandSignature)
// JDECL_RHI_STRUCT_END