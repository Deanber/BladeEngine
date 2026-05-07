#include "DX11RenderTexture.h"
#include <cassert>
#include <cstring>
#include <algorithm>

using namespace RHI;

namespace DX11 {
    DX11RenderTexture::DX11RenderTexture(ID3D11Device* device, ID3D11DeviceContext* context, const TextureDesc& desc)
        : m_desc(desc), m_context(context)
    {
        assert(device != nullptr && context != nullptr);
        InternalCreate(device, context);
    }

    void DX11RenderTexture::Update(const TextureUpdateDesc& updateDesc)
    {
        assert(m_texture);

        UINT subresource = D3D11CalcSubresource(updateDesc.mipLevel, updateDesc.arraySlice, m_desc.mipLevels);

        if (m_desc.memory == MemoryType::CPUUpload || m_desc.memory == MemoryType::CPUReadback)
        {
            D3D11_MAPPED_SUBRESOURCE mapped{};
            HRESULT hr = m_context->Map(
                m_texture.Get(),
                subresource,
                (m_desc.memory == MemoryType::CPUUpload) ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_READ,
                0,
                &mapped
            );
            assert(SUCCEEDED(hr));

            uint8_t* dst = static_cast<uint8_t*>(mapped.pData);
            const uint8_t* src = static_cast<const uint8_t*>(updateDesc.data);

            for (uint32_t y = 0; y < updateDesc.height; ++y)
                std::memcpy(dst + y * mapped.RowPitch, src + y * updateDesc.rowPitch, updateDesc.rowPitch);

            m_context->Unmap(m_texture.Get(), subresource);
        }
        else
        {
            // GPUOnly 用 UpdateSubresource
            m_context->UpdateSubresource(
                m_texture.Get(),
                subresource,
                nullptr,
                updateDesc.data,
                updateDesc.rowPitch,
                updateDesc.slicePitch
            );
        }
    }

    void DX11RenderTexture::InternalCreate(ID3D11Device* device, ID3D11DeviceContext* context)
    {
        D3D11_TEXTURE2D_DESC texDesc{};
        texDesc.Width = m_desc.width;
        texDesc.Height = m_desc.height;
        texDesc.MipLevels = m_desc.mipLevels;
        texDesc.ArraySize = m_desc.arraySize;
        texDesc.Format = GetTypelessFormat(m_desc.format);
        texDesc.SampleDesc.Count = m_desc.sampleCount;

        switch (m_desc.memory)
        {
        case MemoryType::GPUOnly:
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.CPUAccessFlags = 0;
            texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            if (IsRenderTarget() && !IsDepthStencil()) texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
            if (IsDepthStencil()) texDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
            break;

        case MemoryType::CPUUpload:
            texDesc.Usage = D3D11_USAGE_DYNAMIC;
            texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            break;

        case MemoryType::CPUReadback:
            texDesc.Usage = D3D11_USAGE_STAGING;
            texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            texDesc.BindFlags = 0; // <-- Staging 不允许 BindFlags
            break;
        }

        HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, &m_texture);
        assert(SUCCEEDED(hr));

        // ---------------- RTV ----------------
        if (IsRenderTarget() && !IsDepthStencil())
        {
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
            rtvDesc.Format = GetRTVFormat(m_desc.format);

            if (m_desc.arraySize > 1)
                rtvDesc.ViewDimension = (m_desc.sampleCount > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            else
                rtvDesc.ViewDimension = (m_desc.sampleCount > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;

            if (rtvDesc.ViewDimension == D3D11_RTV_DIMENSION_TEXTURE2DARRAY || rtvDesc.ViewDimension == D3D11_RTV_DIMENSION_TEXTURE2D)
            {
                rtvDesc.Texture2D.MipSlice = 0;
                if (rtvDesc.ViewDimension == D3D11_RTV_DIMENSION_TEXTURE2DARRAY)
                {
                    rtvDesc.Texture2DArray.FirstArraySlice = 0;
                    rtvDesc.Texture2DArray.ArraySize = m_desc.arraySize;
                }
            }

            hr = device->CreateRenderTargetView(m_texture.Get(), &rtvDesc, &m_rtv);
            assert(SUCCEEDED(hr));

            m_rtvDesc.usage = TextureViewUsage::RTV;
            m_rtvDesc.format = m_desc.format;
            m_rtvDesc.dimension = (m_desc.arraySize > 1) ? TextureViewDimension::Texture2DArray : TextureViewDimension::Texture2D;
            m_rtvDesc.baseMip = 0;
            m_rtvDesc.mipCount = m_desc.mipLevels;
            m_rtvDesc.baseLayer = 0;
            m_rtvDesc.layerCount = m_desc.arraySize;
        }

        // ---------------- DSV ----------------
        if (IsDepthStencil())
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
            dsvDesc.Format = GetDSVFormat(m_desc.format);

            if (m_desc.arraySize > 1)
                dsvDesc.ViewDimension = (m_desc.sampleCount > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            else
                dsvDesc.ViewDimension = (m_desc.sampleCount > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;

            if (dsvDesc.ViewDimension == D3D11_DSV_DIMENSION_TEXTURE2DARRAY)
            {
                dsvDesc.Texture2DArray.FirstArraySlice = 0;
                dsvDesc.Texture2DArray.ArraySize = m_desc.arraySize;
                dsvDesc.Texture2DArray.MipSlice = 0;
            }
            else if (dsvDesc.ViewDimension == D3D11_DSV_DIMENSION_TEXTURE2D)
            {
                dsvDesc.Texture2D.MipSlice = 0;
            }

            hr = device->CreateDepthStencilView(m_texture.Get(), &dsvDesc, &m_dsv);
            assert(SUCCEEDED(hr));

            m_dsvDesc.usage = TextureViewUsage::DSV;
            m_dsvDesc.format = m_desc.format;
            m_dsvDesc.dimension = (m_desc.arraySize > 1) ? TextureViewDimension::Texture2DArray : TextureViewDimension::Texture2D;
            m_dsvDesc.baseMip = 0;
            m_dsvDesc.mipCount = m_desc.mipLevels;
            m_dsvDesc.baseLayer = 0;
            m_dsvDesc.layerCount = m_desc.arraySize;
        }

        // ---------------- SRV ----------------
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = GetSRVFormat(m_desc.format);
        if (m_desc.arraySize > 1)
            srvDesc.ViewDimension = (m_desc.sampleCount > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        else
            srvDesc.ViewDimension = (m_desc.sampleCount > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;

        if (srvDesc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2DARRAY)
        {
            srvDesc.Texture2DArray.MostDetailedMip = 0;
            srvDesc.Texture2DArray.MipLevels = m_desc.mipLevels;
            srvDesc.Texture2DArray.FirstArraySlice = 0;
            srvDesc.Texture2DArray.ArraySize = m_desc.arraySize;
        }
        else if (srvDesc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D)
        {
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = m_desc.mipLevels;
        }

        hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, &m_srv);
        assert(SUCCEEDED(hr));

        m_srvDesc.usage = TextureViewUsage::SRV;
        m_srvDesc.format = m_desc.format;
        m_srvDesc.dimension = (m_desc.arraySize > 1) ? TextureViewDimension::Texture2DArray : TextureViewDimension::Texture2D;
        m_srvDesc.baseMip = 0;
        m_srvDesc.mipCount = m_desc.mipLevels;
        m_srvDesc.baseLayer = 0;
        m_srvDesc.layerCount = m_desc.arraySize;
    }

    // ----------------------------------------
    // Typeless / RTV / DSV / SRV 格式转换
    // ----------------------------------------
    DXGI_FORMAT DX11RenderTexture::GetTypelessFormat(TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::D24_S8_UNorm: return DXGI_FORMAT_R24G8_TYPELESS;
        case TextureFormat::D32_Float:    return DXGI_FORMAT_R32_TYPELESS;
        case TextureFormat::D32_Float_S8_UInt: return DXGI_FORMAT_R32G8X24_TYPELESS;
        default: return MapFormat(format); // 其余直接返回
        }
    }

    DXGI_FORMAT DX11RenderTexture::GetDSVFormat(TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::D24_S8_UNorm: return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::D32_Float:    return DXGI_FORMAT_D32_FLOAT;
        case TextureFormat::D32_Float_S8_UInt: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        default: return MapFormat(format);
        }
    }

    DXGI_FORMAT DX11RenderTexture::GetRTVFormat(TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::D24_S8_UNorm: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        case TextureFormat::D32_Float:    return DXGI_FORMAT_R32_FLOAT; // 纯深度一般不创建 RTV
        case TextureFormat::D32_Float_S8_UInt: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        default: return MapFormat(format);
        }
    }

    DXGI_FORMAT DX11RenderTexture::GetSRVFormat(TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::D24_S8_UNorm: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        case TextureFormat::D32_Float:    return DXGI_FORMAT_R32_FLOAT;
        case TextureFormat::D32_Float_S8_UInt: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        default: return MapFormat(format);
        }
    }

    DXGI_FORMAT DX11RenderTexture::MapFormat(TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::R8_UNorm:         return DXGI_FORMAT_R8_UNORM;
        case TextureFormat::RGBA8_UNorm:      return DXGI_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::RGBA8_UNorm_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case TextureFormat::BGRA8_UNorm:      return DXGI_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::R11G11B10_Float:  return DXGI_FORMAT_R11G11B10_FLOAT;
        case TextureFormat::RGB10A2_UNorm:    return DXGI_FORMAT_R10G10B10A2_UNORM;
        case TextureFormat::R16_Float:        return DXGI_FORMAT_R16_FLOAT;
        case TextureFormat::RG16_Float:       return DXGI_FORMAT_R16G16_FLOAT;
        case TextureFormat::RGBA16_Float:     return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case TextureFormat::R16_UNorm:        return DXGI_FORMAT_R16_UNORM;
        case TextureFormat::R32_Float:        return DXGI_FORMAT_R32_FLOAT;
        case TextureFormat::RGBA32_Float:     return DXGI_FORMAT_R32G32B32A32_FLOAT;

            // 深度/模板
        case TextureFormat::D24_S8_UNorm:     return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::D32_Float:        return DXGI_FORMAT_D32_FLOAT;
        case TextureFormat::D32_Float_S8_UInt:return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

            // 压缩纹理
        case TextureFormat::BC1_UNorm:        return DXGI_FORMAT_BC1_UNORM;
        case TextureFormat::BC3_UNorm:        return DXGI_FORMAT_BC3_UNORM;
        case TextureFormat::BC4_UNorm:        return DXGI_FORMAT_BC4_UNORM;
        case TextureFormat::BC5_UNorm:        return DXGI_FORMAT_BC5_UNORM;
        case TextureFormat::BC7_UNorm:        return DXGI_FORMAT_BC7_UNORM;

        default:
            assert(false && "Unsupported format");
            return DXGI_FORMAT_UNKNOWN;
        }
    }
}