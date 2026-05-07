#pragma once
#include <d3d11.h>
#include "RHIEnum.h"
#include <PipelineStateDesc.h>

namespace DX11
{
    D3D11_COMPARISON_FUNC CompareFunc(RHI::CompareFunc func);
    D3D11_STENCIL_OP StencilOp(RHI::StencilOp op);
    D3D11_BLEND     ToD3D11Blend(RHI::BlendFactor f);
    D3D11_BLEND_OP  ToD3D11BlendOp(RHI::BlendOp op);
}
