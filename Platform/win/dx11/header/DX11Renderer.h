#pragma once
#include <Windows.h>
#include <d3d11.h>
#include "IRenderer.h"
#include <dxgi.h>
#include <wrl/client.h>
#include <memory>
#include "DX11StateCache.h"
#include "DX11InputLayoutCache.h"
#include "IBuffer.h"

namespace DX11 {

    class DX11CommandList;
    class DX11Texture;
    class DX11TextureView;

    class DX11Renderer : public RHI::IRenderer
    {
    public:
        DX11Renderer() = default;
        ~DX11Renderer() override = default;

        void Initialize(void* windowHandle) override;
        void Shutdown() override;

        // Resource
        std::shared_ptr<RHI::IBuffer> CreateBuffer(const RHI::BufferDesc& desc) override;
        std::shared_ptr<RHI::ITexture> CreateTexture(const RHI::TextureDesc& desc, const void* initialData) override;
        std::shared_ptr<RHI::ITextureView> CreateTextureView(std::shared_ptr<RHI::ITexture> texture, const RHI::TextureViewDesc& desc) override;
        std::shared_ptr<RHI::IShader> CreateShader(const RHI::ShaderDesc& desc) override;

        std::shared_ptr<RHI::IPipelineLayout> CreatePipelineLayout(const RHI::PipelineLayoutDesc& desc) override;
        std::shared_ptr<RHI::IPipelineState> CreatePipelineState(const RHI::PipelineStateDesc& desc) override;

        std::shared_ptr<RHI::ICommandList> CreateCommandList() override;

        void Submit(RHI::ICommandList* cmd) override;

        // Sync（DX11 空实现）
        std::shared_ptr<RHI::IFence> CreateFence(uint64_t initialValue) override { return nullptr; }
        void QueueSignal(RHI::IFence*, uint64_t) override {}
        void QueueWait(RHI::IFence*, uint64_t) override {}

        // Frame
        void BeginFrame() override;
        void EndFrame() override;

        RHI::ITextureView* GetCurrentBackBufferRTV() override;
        RHI::ITextureView* GetDepthStencil() override;

        void WaitIdle() override {}

    private:
        void CreateSwapChain(void* windowHandle);
        void CreateBackBuffer();
        void CreateDepthBuffer();
        void Resize(UINT width, UINT height);

    private:
        Microsoft::WRL::ComPtr<ID3D11Device> m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
        Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;

        // BackBuffer
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_backBuffer;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthBuffer;
        std::shared_ptr<DX11TextureView> m_backBufferRTV;

        // Depth（暂时可选）
        std::shared_ptr<DX11TextureView> m_depthDSV;

        std::unique_ptr<DX11StateCache> m_stateCache;
        std::unique_ptr<DX11InputLayoutCache> m_layoutCache;

        UINT m_width = 0;
        UINT m_height = 0;
    };

}