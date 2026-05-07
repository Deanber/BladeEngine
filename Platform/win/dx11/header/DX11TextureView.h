#pragma once
#include "ITexture.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <cstdint>

namespace DX11 {

    using Microsoft::WRL::ComPtr;
    

    class DX11TextureView : public RHI::ITextureView
    {
    public:
        DX11TextureView(RHI::ITexture* texture, const RHI::TextureViewDesc& desc);

        // ITextureView 接口
        RHI::ITexture* GetTexture() const override;
        const RHI::TextureViewDesc& GetDesc() const override;

        // DX11 专用接口
        void* GetSRV() const override;
        void* GetUAV() const override;
        void* GetNativeRTV() const override;
        void* GetNativeDSV() const override;
        DX11TextureView(ID3D11RenderTargetView* rtv);
        void SetDSV(ID3D11DepthStencilView* dsv);

        // 工厂函数，创建 SRV / UAV
        static DX11TextureView* Create(ID3D11Device* device, RHI::ITexture* texture,
            const RHI::TextureViewDesc& desc, ID3D11Texture2D* dxTexture);

    private:
        RHI::ITexture* m_texture = nullptr;
        RHI::TextureViewDesc m_desc;

        ComPtr<ID3D11ShaderResourceView> m_srv;
        ComPtr<ID3D11UnorderedAccessView> m_uav;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;

        static DXGI_FORMAT DX11TextureFormat(RHI::TextureFormat format);
    };

} // namespace DX11