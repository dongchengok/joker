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
    ERenderer        m_eRenderer;
    EShaderMode      m_eShaderMode;
    EGPUMode         m_eGPUMode;
    RendererContext* m_pRenderContext;
    u32              m_uGPUIndex;
};

JDECL_RHI_STRUCT_BEGIN(RendererContext)
ERenderer m_eRenderer;
bool      m_bEnableGPUBasedValidation;
bool      m_bSupportGroupCreation;
JDECL_RHI_STRUCT_END

JDECL_RHI_STRUCT_BEGIN(Renderer)
struct NullDescriptors* m_pNullDescriptors;
char*                   m_szName;
struct GPUSettings*     m_pActiveGpuSettings;
struct ShaderMacro*     m_pBuiltinShaderDefines;
struct GPUCapBits*      m_pCapBits;
u32                     m_uLinkedNodeCount         : 4;
u32                     m_uUnlinkedRenderIndex     : 4;
EGPUMode                m_eGPUMode                 : 3;
EShaderMode             m_eShaderMode              : 4;
bool                    m_bEnableGpuBaseValidation : 1;
char*                   szApiName;
u32                     m_uBuiltinShaderDefinesCount;
ERenderer               m_eRenderer;
JDECL_RHI_STRUCT_END

extern RendererContext* InitRendererContext(const RendererContextDesc* pDesc);
extern void             ExitRendererContext(RendererContext* pCotnext);
extern Renderer*        InitRenderer(const RendererDesc* pDesc);
extern void             ExitRenderer(Renderer* pRenderer);

}