#include "DX11CommandList.h"
#include "DX11PipelineState.h"
#include "DX11Buffer.h"
#include "DX11TextureView.h"
#include "DX11Viewport.h"

using namespace RHI;
namespace DX11 {

    DX11CommandList::DX11CommandList(ID3D11DeviceContext* ctx)
        : m_ctx(ctx)
    {
    }

    // Frame Control

    void DX11CommandList::Begin()
    {
        m_psoDirty = true;
        m_vbDirty = true;
        m_ibDirty = true;
        m_rtDirty = true;
    }

    // Transition (DX11无用)

    void DX11CommandList::Transition(
        RHI::IResource*,
        ResourceAccessState,
        ResourceAccessState)
    {
        // DX11 无资源屏障
    }

    void DX11CommandList::End() {
        FlushState();
    }

    // Pipeline

    void DX11CommandList::SetPipelineState(IPipelineState* pso)
    {
        auto* dx = static_cast<DX11PipelineState*>(pso);

        if (m_curPSO != dx)
        {
            m_curPSO = dx;
            m_psoDirty = true;
        }
    }

    // Vertex / Index

    void DX11CommandList::SetVertexBuffer(IBuffer* buffer)
    {
        auto* dx = static_cast<DX11Buffer*>(buffer);

        ID3D11Buffer* vb = dx->GetNative();

        if (m_curVB != vb)
        {
            m_curVB = vb;
            m_vbOffset = 0;
            m_vbDirty = true;
        }
    }

    void DX11CommandList::SetIndexBuffer(IBuffer* buffer, IndexFormat format, uint32_t offset)
    {
        auto* dx = static_cast<DX11Buffer*>(buffer);

        ID3D11Buffer* ib = dx->GetNative();

        DXGI_FORMAT fmt = (format == IndexFormat::Uint16)
            ? DXGI_FORMAT_R16_UINT
            : DXGI_FORMAT_R32_UINT;

        if (m_curIB != ib || m_ibFormat != fmt || m_ibOffset != offset)
        {
            m_curIB = ib;
            m_ibFormat = fmt;
            m_ibOffset = offset;
            m_ibDirty = true;
        }
    }

    // Draw

    void DX11CommandList::Draw(uint32_t vertexCount, uint32_t firstVertex)
    {
        FlushState();
        ID3D11VertexShader* tempVS = nullptr;
        m_ctx->VSGetShader(&tempVS, nullptr, nullptr);
        if (tempVS == nullptr) {
            OutputDebugStringA("ERROR: Vertex Shader is NULL before Draw!\n");
        }
        else {
            tempVS->Release();
        }
        m_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_ctx->Draw(vertexCount, firstVertex);
    }

    void DX11CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex)
    {
        FlushState();

        if (instanceCount <= 1)
            m_ctx->DrawIndexed(indexCount, firstIndex, 0);
        else
            m_ctx->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, 0, 0);
    }

    // Render Target

    void DX11CommandList::SetRenderTargets(
        RHI::ITextureView** colorRTs,
        uint32_t count,
        RHI::ITextureView* depthRT)
    {
        count = (count > MAX_RTV) ? MAX_RTV : count;

        bool changed = false;

        for (uint32_t i = 0; i < count; ++i)
        {
            auto rtv = static_cast<ID3D11RenderTargetView*>(colorRTs[i]->GetNativeRTV());

            if (m_curRTVs[i] != rtv)
            {
                m_curRTVs[i] = rtv;
                changed = true;
            }
        }

        for (uint32_t i = count; i < m_numRTVs; ++i)
        {
            m_curRTVs[i] = nullptr;
            changed = true;
        }

        m_numRTVs = count;

        ID3D11DepthStencilView* dsv = nullptr;
        if (depthRT)
        {
            dsv = static_cast<ID3D11DepthStencilView*>(depthRT->GetNativeDSV());
        }

        if (m_curDSV != dsv)
        {
            m_curDSV = dsv;
            changed = true;
        }

        if (changed)
            m_rtDirty = true;
    }

    void DX11CommandList::ClearRenderTarget(ITextureView* rt, const float color[4])
    {
        auto rtv = static_cast<ID3D11RenderTargetView*>(rt->GetNativeRTV());
        m_ctx->ClearRenderTargetView(rtv, color);
    }

    // Push Constants

    void DX11CommandList::SetPushConstants(uint32_t, const void* data, size_t size)
    {

        ID3D11Buffer* cb = nullptr;

        if (cb)
        {
            m_ctx->UpdateSubresource(cb, 0, nullptr, data, 0, 0);
            m_ctx->VSSetConstantBuffers(0, 1, &cb);
            m_ctx->PSSetConstantBuffers(0, 1, &cb);
        }
    }

    // Viewport

    void DX11CommandList::SetViewport(const RHI::Viewport& vp)
    {
        D3D11_VIEWPORT dxVp = DX11ViewportUtils::ToDirectX(vp);

        m_ctx->RSSetViewports(1, &dxVp);
    }

    // Compute

    void DX11CommandList::Dispatch(uint32_t x, uint32_t y, uint32_t z)
    {
        FlushState();
        m_ctx->Dispatch(x, y, z);
    }

    // Flush（核心）

    void DX11CommandList::FlushState()
    {
        if (!m_curPSO)
            return;

        if (m_vbDirty && m_curVB)
        {
            UINT stride = m_curPSO->GetStride(0);
            UINT offset = m_vbOffset;

            m_ctx->IASetVertexBuffers(
                0,
                1,
                &m_curVB,
                &stride,
                &offset
            );

            m_vbDirty = false;
        }

        if (m_psoDirty && m_curPSO)
        {
            m_curPSO->Bind(m_ctx);
            m_psoDirty = false;
        }
        if (m_psoDirty && m_curPSO)
        {
            m_curPSO->Bind(m_ctx);
            m_psoDirty = false;
        }

        if (m_vbDirty && m_curVB)
        {
            m_ctx->IASetVertexBuffers(0, 1, &m_curVB, &m_vbStride, &m_vbOffset);
            m_vbDirty = false;
        }

        if (m_ibDirty && m_curIB)
        {
            m_ctx->IASetIndexBuffer(m_curIB, m_ibFormat, m_ibOffset);
            m_ibDirty = false;
        }

        if (m_rtDirty)
        {
            m_ctx->OMSetRenderTargets(
                m_numRTVs,
                m_curRTVs,
                m_curDSV
            );

            m_rtDirty = false;
        }
    }
}