#include "DX11PipelineState.h"
#include "IShader.h"

using namespace RHI;
namespace DX11 {
    DX11PipelineState::DX11PipelineState(
        ID3D11Device* device,
        DX11StateCache* stateCache,
        DX11InputLayoutCache* layoutCache,
        const PipelineStateDesc& desc)
    {
        // =========================
        // Shader
        // =========================
        m_vs = static_cast<ID3D11VertexShader*>(
            desc.vs->GetNativeHandle());

        m_ps = static_cast<ID3D11PixelShader*>(
            desc.ps->GetNativeHandle());

        // =========================
        // Topology
        // =========================
        switch (desc.topology)
        {
        case PrimitiveTopology::TriangleList:
            m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;
        case PrimitiveTopology::LineList:
            m_topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
        default:
            m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;
        }

        // =========================
        // States (¹Ø¼üÐÞ¸´)
        // =========================
        m_raster = stateCache->GetRaster(device, desc.raster);
        m_depth = stateCache->GetDepth(device, desc.depth);
        m_blend = stateCache->GetBlend(device, desc.blend);

        for (const auto& l : desc.inputLayout.layouts)
        {
            m_strides.push_back(l.stride);
        }

        // =========================
        // Input Layout
        // =========================
        CreateInputLayout(device, layoutCache, desc);

    }

    // =========================
    // Bind
    // =========================
    void DX11PipelineState::Bind(ID3D11DeviceContext* ctx)
    {
        ctx->IASetInputLayout(m_inputLayout.Get());
        ctx->IASetPrimitiveTopology(m_topology);

        ctx->VSSetShader(m_vs.Get(), nullptr, 0);
        ctx->PSSetShader(m_ps.Get(), nullptr, 0);

        ctx->RSSetState(m_raster.Get());
        ctx->OMSetDepthStencilState(m_depth.Get(), 0);
        ctx->OMSetBlendState(m_blend.Get(), m_blendFactor, m_sampleMask);
    }

    // =========================
    // Input Layout
    // =========================
    void DX11PipelineState::CreateInputLayout(
        ID3D11Device* device,
        DX11InputLayoutCache* layoutCache,
        const PipelineStateDesc& desc)
    {
        const void* bytecode = desc.vs->GetByteCode();
        size_t size = desc.vs->GetByteCodeSize();

        m_inputLayout =
            layoutCache->GetOrCreate(
                bytecode,
                size,
                desc.inputLayout);
    }


    DXGI_FORMAT DX11PipelineState::ConvertFormat(RHI::TextureFormat fmt)
    {
        switch (fmt)
        {
        case RHI::TextureFormat::R8_UNorm: return DXGI_FORMAT_R8_UNORM;
        case RHI::TextureFormat::RGBA8_UNorm: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case RHI::TextureFormat::RGBA8_UNorm_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case RHI::TextureFormat::BGRA8_UNorm: return DXGI_FORMAT_B8G8R8A8_UNORM;

        case RHI::TextureFormat::R11G11B10_Float: return DXGI_FORMAT_R11G11B10_FLOAT;
        case RHI::TextureFormat::RGB10A2_UNorm: return DXGI_FORMAT_R10G10B10A2_UNORM;

        case RHI::TextureFormat::R16_Float: return DXGI_FORMAT_R16_FLOAT;
        case RHI::TextureFormat::RG16_Float: return DXGI_FORMAT_R16G16_FLOAT;
        case RHI::TextureFormat::RGBA16_Float: return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case RHI::TextureFormat::R16_UNorm: return DXGI_FORMAT_R16_UNORM;

        case RHI::TextureFormat::R32_Float: return DXGI_FORMAT_R32_FLOAT;
        case RHI::TextureFormat::RGBA32_Float: return DXGI_FORMAT_R32G32B32A32_FLOAT;

        case RHI::TextureFormat::D24_S8_UNorm: return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case RHI::TextureFormat::D32_Float: return DXGI_FORMAT_D32_FLOAT;
        case RHI::TextureFormat::D32_Float_S8_UInt: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

        case RHI::TextureFormat::BC1_UNorm: return DXGI_FORMAT_BC1_UNORM;
        case RHI::TextureFormat::BC3_UNorm: return DXGI_FORMAT_BC3_UNORM;
        case RHI::TextureFormat::BC4_UNorm: return DXGI_FORMAT_BC4_UNORM;
        case RHI::TextureFormat::BC5_UNorm: return DXGI_FORMAT_BC5_UNORM;
        case RHI::TextureFormat::BC7_UNorm: return DXGI_FORMAT_BC7_UNORM;

        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }
}

