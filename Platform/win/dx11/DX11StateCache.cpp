#include "DX11StateCache.h"
#include <cassert>
#include "DX11Convert.h"
#include "PipelineStateDesc.h"
namespace DX11 {

    // Raster
    DX11StateCache::RasterPtr
        DX11StateCache::GetRaster(ID3D11Device* device, const RHI::RasterState& rs)
    {
        RasterKey key{
            rs.wireframe,
            rs.cull,
            rs.frontCCW,
            rs.depthBias,
            rs.slopeScaledDepthBias,
            rs.depthClip,
            rs.scissor,
            rs.multisample,
            rs.antialiasedLine
        };

        auto it = m_rasterCache.find(key);
        if (it != m_rasterCache.end())
            return it->second;

        D3D11_RASTERIZER_DESC desc = {};
        desc.FillMode = rs.wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;

        switch (rs.cull)
        {
        case RHI::CullMode::None:  desc.CullMode = D3D11_CULL_NONE; break;
        case RHI::CullMode::Back:  desc.CullMode = D3D11_CULL_BACK; break;
        case RHI::CullMode::Front: desc.CullMode = D3D11_CULL_FRONT; break;
        default: assert(false);
        }

        desc.FrontCounterClockwise = rs.frontCCW;
        desc.DepthBias = rs.depthBias;
        desc.SlopeScaledDepthBias = rs.slopeScaledDepthBias;

        desc.DepthClipEnable = rs.depthClip;
        desc.ScissorEnable = rs.scissor;

        desc.MultisampleEnable = rs.multisample;
        desc.AntialiasedLineEnable = rs.antialiasedLine;

        RasterPtr state;
        HRESULT hr = device->CreateRasterizerState(&desc, state.GetAddressOf());
        assert(SUCCEEDED(hr));

        m_rasterCache[key] = state;
        return state;
    }

    // Depth / Stencil
    DX11StateCache::DepthPtr
        DX11StateCache::GetDepth(ID3D11Device* device, const RHI::DepthState& ds)
    {
        DepthKey key{
            ds.enable,
            ds.write,
            ds.func,
            ds.stencilEnable,
            StencilKey{
                ds.front.func,
                ds.front.failOp,
                ds.front.depthFailOp,
                ds.front.passOp,
                ds.front.readMask,
                ds.front.writeMask
            },
            StencilKey{
                ds.back.func,
                ds.back.failOp,
                ds.back.depthFailOp,
                ds.back.passOp,
                ds.back.readMask,
                ds.back.writeMask
            }
        };

        auto it = m_depthCache.find(key);
        if (it != m_depthCache.end())
            return it->second;

        D3D11_DEPTH_STENCIL_DESC desc = {};
        ZeroMemory(&desc, sizeof(desc));

        desc.DepthEnable = ds.enable;
        desc.DepthWriteMask = ds.write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = DX11::CompareFunc(ds.func);

        desc.StencilEnable = ds.stencilEnable;
        desc.StencilReadMask = ds.front.readMask;
        desc.StencilWriteMask = ds.front.writeMask;

        desc.FrontFace.StencilFunc = DX11::CompareFunc(ds.front.func);
        desc.FrontFace.StencilFailOp = DX11::StencilOp(ds.front.failOp);
        desc.FrontFace.StencilDepthFailOp = DX11::StencilOp(ds.front.depthFailOp);
        desc.FrontFace.StencilPassOp = DX11::StencilOp(ds.front.passOp);

        // BackFace
        desc.BackFace.StencilFunc = DX11::CompareFunc(ds.back.func);
        desc.BackFace.StencilFailOp = DX11::StencilOp(ds.back.failOp);
        desc.BackFace.StencilDepthFailOp = DX11::StencilOp(ds.back.depthFailOp);
        desc.BackFace.StencilPassOp = DX11::StencilOp(ds.back.passOp);

        DepthPtr state;
        HRESULT hr = device->CreateDepthStencilState(&desc, state.GetAddressOf());
        assert(SUCCEEDED(hr));

        m_depthCache[key] = state;
        return state;
    }

    // Blend
    DX11StateCache::BlendPtr
        DX11StateCache::GetBlend(ID3D11Device* device, const RHI::BlendState& bs)
    {
        BlendKey key;
        key.independentBlend = bs.independentBlend;
        key.targetCount = bs.targetCount;
        for (uint32_t i = 0; i < bs.targetCount; ++i) {
            key.targets[i].enable = bs.targets[i].enable;
            key.targets[i].srcColor = bs.targets[i].srcColor;
            key.targets[i].dstColor = bs.targets[i].dstColor;
            key.targets[i].colorOp = bs.targets[i].colorOp;
            key.targets[i].srcAlpha = bs.targets[i].srcAlpha;
            key.targets[i].dstAlpha = bs.targets[i].dstAlpha;
            key.targets[i].alphaOp = bs.targets[i].alphaOp;
            key.targets[i].writeMask = bs.targets[i].writeMask;
        }

        auto it = m_blendCache.find(key);
        if (it != m_blendCache.end()) return it->second;

        D3D11_BLEND_DESC desc = {};
        desc.AlphaToCoverageEnable = FALSE;
        desc.IndependentBlendEnable = bs.independentBlend;

        auto ConvertBlend = [](RHI::BlendFactor f) -> D3D11_BLEND {
            switch (f)
            {
            case RHI::BlendFactor::Zero: return D3D11_BLEND_ZERO;
            case RHI::BlendFactor::One: return D3D11_BLEND_ONE;
            case RHI::BlendFactor::SrcAlpha: return D3D11_BLEND_SRC_ALPHA;
            case RHI::BlendFactor::InvSrcAlpha: return D3D11_BLEND_INV_SRC_ALPHA;
            case RHI::BlendFactor::DestAlpha: return D3D11_BLEND_DEST_ALPHA;
            case RHI::BlendFactor::InvDestAlpha: return D3D11_BLEND_INV_DEST_ALPHA;
            case RHI::BlendFactor::SrcColor: return D3D11_BLEND_SRC_COLOR;
            case RHI::BlendFactor::InvSrcColor: return D3D11_BLEND_INV_SRC_COLOR;
            case RHI::BlendFactor::DestColor: return D3D11_BLEND_DEST_COLOR;
            case RHI::BlendFactor::InvDestColor: return D3D11_BLEND_INV_DEST_COLOR;
            case RHI::BlendFactor::SrcAlphaSat: return D3D11_BLEND_SRC_ALPHA_SAT;
            case RHI::BlendFactor::BlendConst: return D3D11_BLEND_BLEND_FACTOR;
            case RHI::BlendFactor::InvBlendConst: return D3D11_BLEND_INV_BLEND_FACTOR;
            default: assert(false); return D3D11_BLEND_ONE;
            }
            };

        auto ConvertOp = [](RHI::BlendOp op) -> D3D11_BLEND_OP {
            switch (op)
            {
            case RHI::BlendOp::Add: return D3D11_BLEND_OP_ADD;
            case RHI::BlendOp::Subtract: return D3D11_BLEND_OP_SUBTRACT;
            case RHI::BlendOp::RevSubtract: return D3D11_BLEND_OP_REV_SUBTRACT;
            case RHI::BlendOp::Min: return D3D11_BLEND_OP_MIN;
            case RHI::BlendOp::Max: return D3D11_BLEND_OP_MAX;
            default: assert(false); return D3D11_BLEND_OP_ADD;
            }
            };

        for (uint32_t i = 0; i < bs.targetCount; ++i)
        {
            const auto& t = bs.targets[i];
            auto& rt = desc.RenderTarget[i];

            rt.BlendEnable = t.enable;
            rt.RenderTargetWriteMask = t.writeMask;

            rt.SrcBlend = ConvertBlend(t.srcColor);
            rt.DestBlend = ConvertBlend(t.dstColor);
            rt.BlendOp = ConvertOp(t.colorOp);

            rt.SrcBlendAlpha = ConvertBlend(t.srcAlpha);
            rt.DestBlendAlpha = ConvertBlend(t.dstAlpha);
            rt.BlendOpAlpha = ConvertOp(t.alphaOp);
        }

        BlendPtr state;
        HRESULT hr = device->CreateBlendState(&desc, state.GetAddressOf());
        assert(SUCCEEDED(hr));

        m_blendCache[key] = state;
        return state;
    }
}