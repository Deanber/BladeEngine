#include "DX11Renderer.h"
#include "DX11CommandList.h"
#include "DX11TextureView.h"
#include <cassert>
#include "IBuffer.h"
#include "DX11Buffer.h"
#include "DX11Texture.h"
#include "DX11PipelineState.h"
#include "IShader.h"
#include "DX11Shader.h"
#include "DX11PipelineLayout.h"
#include "DX11StateCache.h"
#include "DX11InputLayoutCache.h"

using namespace RHI;
using Microsoft::WRL::ComPtr;

namespace DX11 {
    class IdAllocator
    {
    public:
        uint64_t Allocate()
        {
            return m_next.fetch_add(1, std::memory_order_relaxed);
        }

    private:
        std::atomic<uint64_t> m_next{ 1 };
    };
}

namespace DX11 {
    void DX11Renderer::Initialize(void* windowHandle)
    {
        UINT createFlags = 0;

        D3D_FEATURE_LEVEL featureLevel;

        DXGI_SWAP_CHAIN_DESC scDesc{};
        scDesc.BufferCount = 1;
        scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.OutputWindow = (HWND)windowHandle;
        scDesc.SampleDesc.Count = 1;
        scDesc.Windowed = TRUE;

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createFlags,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &scDesc,
            &m_swapChain,
            &m_device,
            &featureLevel,
            &m_context
        );

        assert(SUCCEEDED(hr));
        m_stateCache = std::make_unique<DX11StateCache>(m_device.Get());
        m_layoutCache = std::make_unique<DX11InputLayoutCache>(m_device.Get());

        CreateBackBuffer();

        RECT rc;
        GetClientRect((HWND)windowHandle, &rc);

        m_width = rc.right - rc.left;
        m_height = rc.bottom - rc.top;

        //CreateBackBuffer();
        CreateDepthBuffer();
    }

    void CreateSwapChain(void* windowHandle)
    {
    }

    std::shared_ptr<RHI::IBuffer> DX11Renderer::CreateBuffer(const BufferDesc& desc)
    {
        return std::make_shared<DX11Buffer>(
            m_device.Get(),
            m_context.Get(),
            desc
        );
    }

    std::shared_ptr<RHI::ITexture> DX11Renderer::CreateTexture(
        const RHI::TextureDesc& desc,
        const void* initialData)
    {
        return std::make_shared<DX11Texture>(m_device.Get(), m_context.Get(), desc, initialData);
    }

    std::shared_ptr<RHI::ITextureView> DX11Renderer::CreateTextureView(std::shared_ptr<RHI::ITexture> texture, const RHI::TextureViewDesc& desc)
    {
        auto dxTex = static_cast<DX11Texture*>(texture.get());
        auto viewPtr = DX11TextureView::Create(m_device.Get(), texture.get(), desc, dxTex->GetNative());
        return std::shared_ptr<RHI::ITextureView>(viewPtr);
    }


    std::shared_ptr<RHI::IShader> DX11Renderer::CreateShader(const RHI::ShaderDesc& desc)
    {
        return std::make_shared<DX11Shader>(m_device.Get(), desc);
    }


    std::shared_ptr<RHI::IPipelineLayout> DX11Renderer::CreatePipelineLayout(const RHI::PipelineLayoutDesc& desc)
    {
        return std::make_shared<DX11PipelineLayout>(desc);
    }


    std::shared_ptr<RHI::IPipelineState> DX11Renderer::CreatePipelineState(const RHI::PipelineStateDesc& desc)
    {
        return std::make_shared<DX11PipelineState>(m_device.Get(), m_stateCache.get(), m_layoutCache.get(), desc);
    }


    void DX11Renderer::Submit(RHI::ICommandList* cmd)
    {
        auto dxCmd = static_cast<DX11::DX11CommandList*>(cmd);
    }


    void DX11Renderer::Shutdown()
    {
        m_backBufferRTV.reset();
        m_backBuffer.Reset();
        m_swapChain.Reset();
        m_context.Reset();
        m_device.Reset();
    }


    void DX11Renderer::CreateBackBuffer()
    {
        HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_backBuffer);
        assert(SUCCEEDED(hr));

        // 创建 RTV
        ComPtr<ID3D11RenderTargetView> rtv;
        hr = m_device->CreateRenderTargetView(m_backBuffer.Get(), nullptr, &rtv);
        assert(SUCCEEDED(hr));

        m_backBufferRTV = std::make_shared<DX11TextureView>(rtv.Get());
    }


    void DX11Renderer::CreateDepthBuffer()
    {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = m_width;
        desc.Height = m_height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.SampleDesc.Count = 1;
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        HRESULT hr = m_device->CreateTexture2D(&desc, nullptr, &m_depthBuffer);
        assert(SUCCEEDED(hr));

        ComPtr<ID3D11DepthStencilView> dsv;
        hr = m_device->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, &dsv);
        assert(SUCCEEDED(hr));

        m_depthDSV = std::make_shared<DX11TextureView>(nullptr, RHI::TextureViewDesc{});
        m_depthDSV->SetDSV(dsv.Get());
    }

    std::shared_ptr<ICommandList> DX11Renderer::CreateCommandList()
    {
        return std::make_shared<DX11CommandList>(m_context.Get());
    }

    void DX11Renderer::BeginFrame()
    {
        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

        auto rtv = static_cast<ID3D11RenderTargetView*>(m_backBufferRTV->GetNativeRTV());
        auto dsv = static_cast<ID3D11DepthStencilView*>(m_depthDSV->GetNativeDSV());

        // 绑定
        m_context->OMSetRenderTargets(1, &rtv, dsv);

        // 清屏
        m_context->ClearRenderTargetView(rtv, clearColor);
        m_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }

    void DX11Renderer::Resize(UINT width, UINT height)
    {
        if (width == 0 || height == 0)
            return;

        m_width = width;
        m_height = height;

        // 解绑
        ID3D11RenderTargetView* nullRTV = nullptr;
        m_context->OMSetRenderTargets(1, &nullRTV, nullptr);

        // 释放旧资源
        m_backBufferRTV.reset();
        m_backBuffer.Reset();
        m_depthDSV.reset();
        m_depthBuffer.Reset();

        // Resize SwapChain
        HRESULT hr = m_swapChain->ResizeBuffers(
            0,
            width,
            height,
            DXGI_FORMAT_UNKNOWN,
            0
        );
        assert(SUCCEEDED(hr));

        // 重建
        CreateBackBuffer();
        CreateDepthBuffer();
    }

    void DX11Renderer::EndFrame()
    {
        m_swapChain->Present(1, 0);
    }

    ITextureView* DX11Renderer::GetCurrentBackBufferRTV()
    {
        return m_backBufferRTV.get();
    }

    ITextureView* DX11Renderer::GetDepthStencil()
    {
        return m_depthDSV.get();
    }
}