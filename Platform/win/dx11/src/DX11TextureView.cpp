#include "DX11TextureView.h"
#include <cassert>

namespace DX11 {

    DX11TextureView::DX11TextureView(RHI::ITexture* texture, const RHI::TextureViewDesc& desc)
        : m_texture(texture), m_desc(desc)
    {
    }

    RHI::ITexture* DX11TextureView::GetTexture() const
    {
        return m_texture;
    }

    const RHI::TextureViewDesc& DX11TextureView::GetDesc() const
    {
        return m_desc;
    }

    void* DX11TextureView::GetSRV() const
    {
        return static_cast<void*>(m_srv.Get());
    }

    void* DX11TextureView::GetUAV() const
    {
        return static_cast<void*>(m_uav.Get());
    }

    void* DX11TextureView::GetNativeRTV() const
    {
        return static_cast<void*>(m_rtv.Get());
    }

    void* DX11TextureView::GetNativeDSV() const
    {
        return m_dsv.Get();
    }
    void DX11TextureView::SetDSV(ID3D11DepthStencilView* dsv)
    {
        m_dsv = dsv;
    }

    DX11TextureView::DX11TextureView(ID3D11RenderTargetView* rtv)
    {
        m_rtv = rtv;
        m_desc.usage = RHI::TextureViewUsage::RTV;
        m_desc.dimension = RHI::TextureViewDimension::Texture2D;
        m_desc.format = RHI::TextureFormat::RGBA8_UNorm;
    }

    DX11TextureView* DX11TextureView::Create(ID3D11Device* device, RHI::ITexture* texture,
        const RHI::TextureViewDesc& desc, ID3D11Texture2D* dxTexture)
    {
        auto view = new DX11TextureView(texture, desc);

        // --- SRV ---
        if (desc.usage == RHI::TextureViewUsage::SRV || desc.usage == RHI::TextureViewUsage::RTV)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
            srvDesc.Format = DX11TextureFormat(desc.format);
            auto& texDescRef = texture->GetDesc();
            switch (desc.dimension)
            {
            case RHI::TextureViewDimension::Texture2D:
            {
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MostDetailedMip = desc.baseMip;
                srvDesc.Texture2D.MipLevels = (desc.mipCount > 0) ? desc.mipCount : (texture->GetDesc().mipLevels - desc.baseMip);
                break;
            }
            case RHI::TextureViewDimension::Texture2DArray:
            {
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                srvDesc.Texture2DArray.MostDetailedMip = desc.baseMip;
                srvDesc.Texture2DArray.MipLevels = (desc.mipCount > 0) ? desc.mipCount : (texture->GetDesc().mipLevels - desc.baseMip);
                srvDesc.Texture2DArray.FirstArraySlice = desc.baseLayer;
                srvDesc.Texture2DArray.ArraySize = desc.layerCount;
                srvDesc.Texture2DArray.ArraySize = (desc.layerCount > 0) ? desc.layerCount : (texDescRef.arraySize - desc.baseLayer);
                break;
            }
            default:
                assert(false && "Unsupported texture view dimension for SRV");
            }

            // 确保原始纹理支持 SRV
            D3D11_TEXTURE2D_DESC texDesc{};
            dxTexture->GetDesc(&texDesc);
            assert(texDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE);

            HRESULT hr = device->CreateShaderResourceView(dxTexture, &srvDesc, &view->m_srv);
            assert(SUCCEEDED(hr));
        }

        // --- UAV ---
        if (desc.usage == RHI::TextureViewUsage::UAV)
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
            uavDesc.Format = DX11TextureFormat(desc.format);

            switch (desc.dimension)
            {
            case RHI::TextureViewDimension::Texture2D:
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Texture2D.MipSlice = desc.baseMip;
                break;
            case RHI::TextureViewDimension::Texture2DArray:
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
                uavDesc.Texture2DArray.MipSlice = desc.baseMip;
                uavDesc.Texture2DArray.FirstArraySlice = desc.baseLayer;
                uavDesc.Texture2DArray.ArraySize = desc.layerCount;
                break;
            default:
                assert(false && "Unsupported texture view dimension for UAV");
            }

            D3D11_TEXTURE2D_DESC texDesc{};
            dxTexture->GetDesc(&texDesc);
            assert(texDesc.BindFlags & D3D11_BIND_UNORDERED_ACCESS);

            HRESULT hr = device->CreateUnorderedAccessView(dxTexture, &uavDesc, &view->m_uav);
            assert(SUCCEEDED(hr));
        }

        return view;
    }

    DXGI_FORMAT DX11TextureView::DX11TextureFormat(RHI::TextureFormat format)
    {
        switch (format)
        {
            // --- 8-bit ---
        case RHI::TextureFormat::R8_UNorm:          return DXGI_FORMAT_R8_UNORM;
        case RHI::TextureFormat::RGBA8_UNorm:       return DXGI_FORMAT_R8G8B8A8_UNORM;
        case RHI::TextureFormat::RGBA8_UNorm_SRGB:  return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case RHI::TextureFormat::BGRA8_UNorm:       return DXGI_FORMAT_B8G8R8A8_UNORM;

            // --- 10/11-bit HDR ---
        case RHI::TextureFormat::R11G11B10_Float:  return DXGI_FORMAT_R11G11B10_FLOAT;
        case RHI::TextureFormat::RGB10A2_UNorm:    return DXGI_FORMAT_R10G10B10A2_UNORM;

            // --- 16-bit ---
        case RHI::TextureFormat::R16_Float:        return DXGI_FORMAT_R16_FLOAT;
        case RHI::TextureFormat::RG16_Float:       return DXGI_FORMAT_R16G16_FLOAT;
        case RHI::TextureFormat::RGBA16_Float:     return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case RHI::TextureFormat::R16_UNorm:        return DXGI_FORMAT_R16_UNORM;

            // --- 32-bit ---
        case RHI::TextureFormat::R32_Float:        return DXGI_FORMAT_R32_FLOAT;
        case RHI::TextureFormat::RGBA32_Float:     return DXGI_FORMAT_R32G32B32A32_FLOAT;

            // --- Depth / Stencil ---
        case RHI::TextureFormat::D24_S8_UNorm:     return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case RHI::TextureFormat::D32_Float:        return DXGI_FORMAT_D32_FLOAT;
        case RHI::TextureFormat::D32_Float_S8_UInt:return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

            // --- Block Compression ---
        case RHI::TextureFormat::BC1_UNorm:        return DXGI_FORMAT_BC1_UNORM;
        case RHI::TextureFormat::BC3_UNorm:        return DXGI_FORMAT_BC3_UNORM;
        case RHI::TextureFormat::BC4_UNorm:        return DXGI_FORMAT_BC4_UNORM;
        case RHI::TextureFormat::BC5_UNorm:        return DXGI_FORMAT_BC5_UNORM;
        case RHI::TextureFormat::BC7_UNorm:        return DXGI_FORMAT_BC7_UNORM;

            // --- ASTC (DirectX 不原生支持，需要在 DX12/Shader 中解码) ---
        case RHI::TextureFormat::ASTC_4x4_UNorm:
        case RHI::TextureFormat::ASTC_6x6_UNorm:
            assert(false && "ASTC textures are not natively supported in DX11");
            return DXGI_FORMAT_UNKNOWN;

        default:
            assert(false && "Unsupported RHI::TextureFormat in DX11TextureView");
            return DXGI_FORMAT_UNKNOWN;
        }
    }
}