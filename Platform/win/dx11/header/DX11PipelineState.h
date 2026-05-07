#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "RHIEnum.h"
#include "IPipelineState.h"
#include "DX11StateCache.h"
#include "DX11InputLayoutCache.h"

namespace DX11
{
    class DX11PipelineState : public RHI::IPipelineState
    {
    public:
        DX11PipelineState(
            ID3D11Device* device,
            DX11StateCache* stateCache,
            DX11InputLayoutCache* layoutCache,
            const RHI::PipelineStateDesc& desc);

        void Bind(ID3D11DeviceContext* ctx);

        DXGI_FORMAT ConvertFormat(RHI::TextureFormat fmt);

        UINT GetStride(uint32_t slot) const
        {
            assert(slot < m_strides.size());
            return m_strides[slot];
        }


    private:

        // =========================
        // Shaders
        // =========================
        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_ps;

        // =========================
        // Input Layout (from cache)
        // =========================
        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

        // =========================
        // Render States (cached)
        // =========================
        DX11StateCache::RasterPtr m_raster;
        DX11StateCache::DepthPtr  m_depth;
        DX11StateCache::BlendPtr  m_blend;

        // =========================
        // Fixed pipeline params
        // =========================
        D3D11_PRIMITIVE_TOPOLOGY m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        float m_blendFactor[4] = { 1, 1, 1, 1 };
        UINT  m_sampleMask = 0xffffffff;

        std::vector<UINT> m_strides;

    private:

        // =========================
        // InputLayout creation helper
        // =========================
        void CreateInputLayout(
            ID3D11Device* device,
            DX11InputLayoutCache* layoutCache,
            const RHI::PipelineStateDesc& desc);
    };
}