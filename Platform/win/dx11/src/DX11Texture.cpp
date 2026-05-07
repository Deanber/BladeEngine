#include "DX11Texture.h"

#include <d3d11.h>
#include <cassert>
#include <cstring>
#include <vector>

using namespace RHI;
namespace DX11 {

    static DXGI_FORMAT ToDXFormat(RHI::TextureFormat format)
    {
        switch (format)
        {
        case RHI::TextureFormat::RGBA8_UNorm:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    static UINT ToDXBindFlags(uint32_t bindFlags)
    {
        UINT flags = 0;

        if (bindFlags & Bind_ShaderResource)
            flags |= D3D11_BIND_SHADER_RESOURCE;

        if (bindFlags & Bind_RenderTarget)
            flags |= D3D11_BIND_RENDER_TARGET;

        if (bindFlags & Bind_DepthStencil)
            flags |= D3D11_BIND_DEPTH_STENCIL;

        return flags;
    }

    static D3D11_USAGE ToDXUsage(RHI::MemoryType memory)
    {
        switch (memory)
        {
        case RHI::MemoryType::GPUOnly:
            return D3D11_USAGE_DEFAULT;

        case RHI::MemoryType::CPUUpload:
            return D3D11_USAGE_DYNAMIC;

        case RHI::MemoryType::CPUReadback:
            return D3D11_USAGE_STAGING;

        default:
            return D3D11_USAGE_DEFAULT;
        }
    }

    static UINT ToDXCPUAccess(RHI::MemoryType memory)
    {
        switch (memory)
        {
        case RHI::MemoryType::CPUUpload:
            return D3D11_CPU_ACCESS_WRITE;

        case RHI::MemoryType::CPUReadback:
            return D3D11_CPU_ACCESS_READ;

        default:
            return 0;
        }
    }

    static uint32_t BytesPerPixel(TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::RGBA8_UNorm:
            return 4;

        default:
            return 4;
        }
    }
    DX11Texture::~DX11Texture() = default;
    DX11Texture::DX11Texture(
        ID3D11Device* device,
        ID3D11DeviceContext* context,
        const TextureDesc& desc,
        const void* initialData)
        : m_desc(desc), m_context(context)
    {
        D3D11_TEXTURE2D_DESC dxDesc = {};
        dxDesc.Width = desc.width;
        dxDesc.Height = desc.height;
        dxDesc.MipLevels = desc.mipLevels;
        dxDesc.ArraySize = desc.arraySize;
        dxDesc.Format = ToDXFormat(desc.format);
        dxDesc.SampleDesc.Count = desc.sampleCount;

        dxDesc.Usage = ToDXUsage(desc.memory);
        dxDesc.BindFlags = ToDXBindFlags(desc.bindFlags);
        dxDesc.CPUAccessFlags = ToDXCPUAccess(desc.memory);

        D3D11_SUBRESOURCE_DATA subData = {};
        D3D11_SUBRESOURCE_DATA* pData = nullptr;

        if (initialData)
        {
            std::vector<D3D11_SUBRESOURCE_DATA> initDataVec;
            initDataVec.reserve(desc.arraySize * desc.mipLevels);

            const uint8_t* ptr = reinterpret_cast<const uint8_t*>(initialData);

            for (uint32_t slice = 0; slice < desc.arraySize; ++slice)
            {
                uint32_t w = desc.width;
                uint32_t h = desc.height;

                for (uint32_t mip = 0; mip < desc.mipLevels; ++mip)
                {
                    D3D11_SUBRESOURCE_DATA sub = {};
                    sub.pSysMem = ptr;
                    sub.SysMemPitch = w * BytesPerPixel(desc.format);
                    sub.SysMemSlicePitch = w * h * BytesPerPixel(desc.format);

                    initDataVec.push_back(sub);

                    ptr += sub.SysMemSlicePitch;

                    w = (std::max)(1u, w / 2);
                    h = (std::max)(1u, h / 2);
                }
            }

            HRESULT hr = device->CreateTexture2D(&dxDesc, initDataVec.data(), &m_texture);
            assert(SUCCEEDED(hr));
        }
        else
        {
            HRESULT hr = device->CreateTexture2D(&dxDesc, nullptr, &m_texture);
            assert(SUCCEEDED(hr));
        }

        HRESULT hr = device->CreateTexture2D(&dxDesc, pData, &m_texture);
        assert(SUCCEEDED(hr));
    }

    void DX11Texture::Update(const TextureUpdateDesc& desc)
    {
        UINT subresource = D3D11CalcSubresource(
            desc.mipLevel,
            desc.arraySlice,
            m_desc.mipLevels);

        if (m_desc.memory == MemoryType::CPUUpload)
        {
            D3D11_MAPPED_SUBRESOURCE mapped;
            HRESULT hr = m_context->Map(m_texture.Get(), subresource, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            assert(SUCCEEDED(hr));

            // 建议改为逐行拷贝以防 Padding
            uint8_t* dst = static_cast<uint8_t*>(mapped.pData);
            const uint8_t* src = static_cast<const uint8_t*>(desc.data);
            for (uint32_t y = 0; y < desc.height; ++y) {
                std::memcpy(dst + y * mapped.RowPitch, src + y * desc.rowPitch, desc.rowPitch);
            }

            m_context->Unmap(m_texture.Get(), subresource);
        }
        else
        {
            m_context->UpdateSubresource(
                m_texture.Get(),
                subresource,
                nullptr,
                desc.data,
                desc.rowPitch,
                desc.slicePitch);
        }
    }
}