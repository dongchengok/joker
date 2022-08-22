#pragma once
#include "RendererConfig.h"
#include "RendererEnums.h"

namespace joker::rhi
{
struct GPUInfo;
struct GPUSettings;
struct GPUCapBits;
struct ShaderMacro;

struct RendererContextDesc;
struct RendererContext;
struct RendererDesc;
struct Renderer;

struct GPUVendorPreset
{
    EGPUPresetLevel m_ePresetLevel;
    char            m_szVendorId[JMAX_NAME_LENGTH];
    char            m_szModelId[JMAX_NAME_LENGTH];
    char            m_szRevisionId[JMAX_NAME_LENGTH]; // Optional as not all gpu's have that. Default is : 0x00
    char            m_szGPUName[JMAX_NAME_LENGTH];    // If GPU Name is missing then value will be empty string
    char            m_szGPUDriverVersion[JMAX_NAME_LENGTH];
    char            m_szGPUDriverDate[JMAX_NAME_LENGTH];
};

struct GPUSettings
{
    GPUVendorPreset      m_GPUVendorPreset;
    u32                  m_uUniformBufferAlignment;
    u32                  m_uUploadBufferTextureAlignment;
    u32                  m_uUploadBufferTextureRowAlignment;
    u32                  m_uMaxVertexInputBindings;
    u32                  m_uMaxRootSignatureDWORDS;
    u32                  m_uWaveLaneCount;
    EWaveOpsSupportFlags m_eWaveOpsSupportFlags;
    EShadingRate         m_eShadingRates;
    EShadingRateCaps     m_eShadingRateCaps;
    u32                  m_uShadingRateTexelWidth;
    u32                  m_uShadingRateTexelHeight;
    u32                  m_bMultiDrawIndirect       : 1;
    u32                  m_bROVsSupported           : 1;
    u32                  m_bTessellationSupported   : 1;
    u32                  m_bGeometryShaderSupported : 1;
    u32                  m_bGpuBreadcrumbs          : 1;
    u32                  m_bHDRSupported            : 1;
};

JDECL_RHI_STRUCT_BEGIN(GPUInfo)
GPUSettings m_Settings;
JDECL_RHI_STRUCT_END

JDECL_RHI_STRUCT_BEGIN(RendererContextDesc)
const char* m_szAppName;
ERenderer   m_eRenderer;
bool        m_bDebug;
bool        m_bGPUDebug;
JDECL_RHI_STRUCT_END

JDECL_RHI_STRUCT_BEGIN(RendererContext)
ERenderer m_eRenderer;
GPUInfo*  m_pGPUs;
u32       m_uGPUCount;
u32       m_uRendererCount;
JDECL_RHI_STRUCT_END

JDECL_RHI_STRUCT_BEGIN(RendererDesc)
const char*      m_szName;
GPUSettings*     m_pActiveGPUSettings;
ERenderer        m_eRenderer;
EShaderMode      m_eShaderMode;
EGPUMode         m_eGPUMode;
bool             m_bDebug;
bool             m_bGPUDebug;
RendererContext* m_pRenderContext;
u32              m_uGPUIndex;
JDECL_RHI_STRUCT_END

struct NullDescriptors
{
    // Texture* pDefaultTextureSRV[MAX_LINKED_GPUS][TEXTURE_DIM_COUNT];
    // Texture* pDefaultTextureUAV[MAX_LINKED_GPUS][TEXTURE_DIM_COUNT];
    // Buffer*  pDefaultBufferSRV[MAX_LINKED_GPUS];
    // Buffer*  pDefaultBufferUAV[MAX_LINKED_GPUS];
    // Sampler* pDefaultSampler;
    // Mutex    mSubmitMutex;

    // // #TODO - Remove after we have a better way to specify initial resource state
    // // Unlike DX12, Vulkan textures start in undefined layout.
    // // With this, we transition them to the specified layout so app code doesn't have to worry about this
    // Mutex    mInitialTransitionMutex;
    // Queue*   pInitialTransitionQueue;
    // CmdPool* pInitialTransitionCmdPool;
    // Cmd*     pInitialTransitionCmd;
    // Fence*   pInitialTransitionFence;
};

JDECL_RHI_STRUCT_BEGIN(Renderer)
RendererContext*    m_pContext;
NullDescriptors*    m_pNullDescriptors;
char*               m_szName;
char*               m_szApiName;
struct GPUSettings* m_pActiveGpuSettings;
struct ShaderMacro* m_pBuiltinShaderDefines;
struct GPUCapBits*  m_pCapBits;
u32                 m_uLinkedNodeCount     : 4;
u32                 m_uUnlinkedRenderIndex : 4;
EGPUMode            m_eGPUMode             : 4;
EShaderMode         m_eShaderMode          : 4;
u32                 m_uBuiltinShaderDefinesCount;
ERenderer           m_eRenderer;
JDECL_RHI_STRUCT_END

}