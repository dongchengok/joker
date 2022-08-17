#pragma once
#include "RendererConfig.h"
#include "RendererEnums.h"
#include "RendererTypes.h"

namespace joker::rhi
{
struct RendererContextDesc
{
    const char* m_szAppName;
    ERenderer   m_eRenderer;
    bool        m_bEnableDebugUtilsMessager;
    bool        m_bEnableValidation;
    bool        m_bEnableGPUBasedValidation;
};

struct RendererDesc
{
    const char*      m_szAppName;
    ERenderer        m_eRenderer;
    EShaderMode      m_eShaderMode;
    EGPUMode         m_eGPUMode;
    bool             m_bEnableDebugUtilsMessager;
    bool             m_bEnableValidation;
    bool             m_bEnableGPUBaseValidation;
    RendererContext* m_pRenderContext;
    u32              m_uGPUIndex;
};

JDECL_RHI_STRUCT_BEGIN(RendererContext)
ERenderer m_eRenderer;
bool      m_bEnableGPUBasedValidation;
bool      m_bSupportGroupCreation;
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
NullDescriptors*    m_pNullDescriptors;
char*               m_szName;
struct GPUSettings* m_pActiveGpuSettings;
struct ShaderMacro* m_pBuiltinShaderDefines;
struct GPUCapBits*  m_pCapBits;
u32                 m_uLinkedNodeCount         : 4;
u32                 m_uUnlinkedRenderIndex     : 4;
EGPUMode            m_eGPUMode                 : 3;
EShaderMode         m_eShaderMode              : 4;
bool                m_bEnableGpuBaseValidation : 1;
char*               szApiName;
u32                 m_uBuiltinShaderDefinesCount;
ERenderer           m_eRenderer;
JDECL_RHI_STRUCT_END

extern RendererContext* InitRendererContext(const RendererContextDesc* pDesc);
extern void             ExitRendererContext(RendererContext* pCotnext);
extern Renderer*        InitRenderer(const RendererDesc* pDesc);
extern void             ExitRenderer(Renderer* pRenderer);

}