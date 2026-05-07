#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "IRenderTexture.h"
#include "RHIEnum.h"

namespace DX11 {

    class DX11RenderTexture : public RHI::IRenderTexture
    {
    public:
        DX11RenderTexture(ID3D11Device* device, ID3D11DeviceContext* context, const RHI::TextureDesc& desc);
        virtual ~DX11RenderTexture() override = default;

        const RHI::TextureDesc& GetDesc() const override { return m_desc; }
        void Update(const RHI::TextureUpdateDesc& updateDesc) override;

        RHI::TextureViewDesc GetRTVDesc() const override { return m_rtvDesc; }
        RHI::TextureViewDesc GetDSVDesc() const override { return m_dsvDesc; }
        RHI::TextureViewDesc GetSRVDesc() const override { return m_srvDesc; }

        // DX11 专用接口
        ID3D11Resource* GetDXResource() const { return m_texture.Get(); }
        ID3D11RenderTargetView* GetRTV() const { return m_rtv.Get(); }
        ID3D11DepthStencilView* GetDSV() const { return m_dsv.Get(); }
        ID3D11ShaderResourceView* GetSRV() const { return m_srv.Get(); }
        bool IsDepthStencil() const {
            return RHI::IsDepthFormat(m_desc.format);
        }

        bool IsRenderTarget() const {
            return !IsDepthStencil() && (m_desc.memory == RHI::MemoryType::GPUOnly);
        }

    private:
        void InternalCreate(ID3D11Device* device, ID3D11DeviceContext* context);
        DXGI_FORMAT GetTypelessFormat(RHI::TextureFormat format);
        DXGI_FORMAT GetDSVFormat(RHI::TextureFormat format);
        DXGI_FORMAT GetRTVFormat(RHI::TextureFormat format);
        DXGI_FORMAT GetSRVFormat(RHI::TextureFormat format);
        DXGI_FORMAT MapFormat(RHI::TextureFormat format);

    private:
        RHI::TextureDesc m_desc;

        RHI::TextureViewDesc m_rtvDesc{};
        RHI::TextureViewDesc m_dsvDesc{};
        RHI::TextureViewDesc m_srvDesc{};

        Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_texture;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_rtv;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   m_dsv;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;

        ID3D11DeviceContext* m_context = nullptr;
    };

    

} // namespace DX11