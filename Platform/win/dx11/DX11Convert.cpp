#include "DX11Convert.h"
#include <cassert>

namespace DX11
{
    D3D11_COMPARISON_FUNC CompareFunc(RHI::CompareFunc func)
    {
        switch (func)
        {
        case RHI::CompareFunc::Never: return D3D11_COMPARISON_NEVER;
        case RHI::CompareFunc::Less: return D3D11_COMPARISON_LESS;
        case RHI::CompareFunc::Equal: return D3D11_COMPARISON_EQUAL;
        case RHI::CompareFunc::LessEqual: return D3D11_COMPARISON_LESS_EQUAL;
        case RHI::CompareFunc::Greater: return D3D11_COMPARISON_GREATER;
        case RHI::CompareFunc::NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
        case RHI::CompareFunc::GreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
        case RHI::CompareFunc::Always: return D3D11_COMPARISON_ALWAYS;
        }
        assert(false);
        return D3D11_COMPARISON_ALWAYS;
    }

    D3D11_STENCIL_OP StencilOp(RHI::StencilOp op)
    {
        switch (op)
        {
        case RHI::StencilOp::Keep: return D3D11_STENCIL_OP_KEEP;
        case RHI::StencilOp::Zero: return D3D11_STENCIL_OP_ZERO;
        case RHI::StencilOp::Replace: return D3D11_STENCIL_OP_REPLACE;
        case RHI::StencilOp::Incr: return D3D11_STENCIL_OP_INCR;
        case RHI::StencilOp::IncrSat: return D3D11_STENCIL_OP_INCR_SAT;
        case RHI::StencilOp::Decr: return D3D11_STENCIL_OP_DECR;
        case RHI::StencilOp::DecrSat: return D3D11_STENCIL_OP_DECR_SAT;
        case RHI::StencilOp::Invert: return D3D11_STENCIL_OP_INVERT;
        }
        assert(false);
        return D3D11_STENCIL_OP_KEEP;
    }

    D3D11_BLEND DX11::ToD3D11Blend(RHI::BlendFactor f)
    {
        switch (f)
        {
        case RHI::BlendFactor::Zero:          return D3D11_BLEND_ZERO;
        case RHI::BlendFactor::One:           return D3D11_BLEND_ONE;
        case RHI::BlendFactor::SrcAlpha:      return D3D11_BLEND_SRC_ALPHA;
        case RHI::BlendFactor::InvSrcAlpha:   return D3D11_BLEND_INV_SRC_ALPHA;
        case RHI::BlendFactor::DestAlpha:     return D3D11_BLEND_DEST_ALPHA;
        case RHI::BlendFactor::InvDestAlpha:  return D3D11_BLEND_INV_DEST_ALPHA;
        case RHI::BlendFactor::SrcColor:      return D3D11_BLEND_SRC_COLOR;
        case RHI::BlendFactor::InvSrcColor:   return D3D11_BLEND_INV_SRC_COLOR;
        case RHI::BlendFactor::DestColor:     return D3D11_BLEND_DEST_COLOR;
        case RHI::BlendFactor::InvDestColor:  return D3D11_BLEND_INV_DEST_COLOR;
        case RHI::BlendFactor::SrcAlphaSat:   return D3D11_BLEND_SRC_ALPHA_SAT;
        case RHI::BlendFactor::BlendConst:    return D3D11_BLEND_BLEND_FACTOR;
        case RHI::BlendFactor::InvBlendConst: return D3D11_BLEND_INV_BLEND_FACTOR;
        default: assert(false); return D3D11_BLEND_ONE;
        }
    }

    D3D11_BLEND_OP DX11::ToD3D11BlendOp(RHI::BlendOp op)
    {
        switch (op)
        {
        case RHI::BlendOp::Add:         return D3D11_BLEND_OP_ADD;
        case RHI::BlendOp::Subtract:    return D3D11_BLEND_OP_SUBTRACT;
        case RHI::BlendOp::RevSubtract: return D3D11_BLEND_OP_REV_SUBTRACT;
        case RHI::BlendOp::Min:         return D3D11_BLEND_OP_MIN;
        case RHI::BlendOp::Max:         return D3D11_BLEND_OP_MAX;
        default: assert(false); return D3D11_BLEND_OP_ADD;
        }
    }
}