#pragma once
#include "ICommandList.h"
#include <d3d11.h>
#include <wrl/client.h>
#include "IPipelineLayout.h"
#include "IResource.h"

namespace DX11 {
    class DX11PipelineState;
    class DX11Buffer;
    class DX11TextureView;

    class DX11CommandList : public RHI::ICommandList
    {
    public:
        DX11CommandList(ID3D11DeviceContext* ctx);

        void Begin() override;
        void End() override;

        void Transition(
            RHI::IResource* resource,
            RHI::ResourceAccessState before,
            RHI::ResourceAccessState after) override;

        void SetPipelineState(RHI::IPipelineState*) override;
        void SetPipelineLayout(RHI::IPipelineLayout*) override {}

        void SetBindGroup(uint32_t index, RHI::IBindGroup*) override {}

        void SetVertexBuffer(RHI::IBuffer*) override;
        void SetIndexBuffer(RHI::IBuffer*, RHI::IndexFormat format, uint32_t offset) override;

        void Draw(uint32_t vertexCount, uint32_t firstVertex) override;
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex) override;

        void SetRenderTargets(
            RHI::ITextureView** colorRTs,
            uint32_t count,
            RHI::ITextureView* depthRT) override;

        void ClearRenderTarget(RHI::ITextureView*, const float color[4]) override;

        void SetPushConstants(uint32_t offset, const void* data, size_t size) override;

        void SetViewport(const RHI::Viewport& viewport) override;

        void Dispatch(uint32_t x, uint32_t y, uint32_t z) override;

    private:
        void FlushState();

    private:
        ID3D11DeviceContext* m_ctx = nullptr;

        // ===== µ±Ç°×´Ì¬»º´æ =====
        DX11PipelineState* m_curPSO = nullptr;

        ID3D11Buffer* m_curVB = nullptr;
        UINT m_vbStride = 0;
        UINT m_vbOffset = 0;

        ID3D11Buffer* m_curIB = nullptr;
        DXGI_FORMAT m_ibFormat = DXGI_FORMAT_R32_UINT;
        UINT m_ibOffset = 0;

        static const uint32_t MAX_RTV = 8;
        ID3D11RenderTargetView* m_curRTVs[MAX_RTV] = {};
        uint32_t m_numRTVs = 0;

        ID3D11DepthStencilView* m_curDSV = nullptr;

        // ===== Dirty Flags =====
        bool m_psoDirty = false;
        bool m_vbDirty = false;
        bool m_ibDirty = false;
        bool m_rtDirty = false;
    };
}