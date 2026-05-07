#include "DX11Buffer.h"

#include <d3d11.h>
#include <cassert>
#include <cstring>

using namespace RHI;
namespace DX11 {

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

    static UINT ToDXBindFlags(uint32_t bindFlags)
    {
        UINT flags = 0;

        if (bindFlags & Bind_VertexBuffer)   flags |= D3D11_BIND_VERTEX_BUFFER;
        if (bindFlags & Bind_IndexBuffer)    flags |= D3D11_BIND_INDEX_BUFFER;
        if (bindFlags & Bind_ConstantBuffer) flags |= D3D11_BIND_CONSTANT_BUFFER;
        if (bindFlags & Bind_ShaderResource) flags |= D3D11_BIND_SHADER_RESOURCE;
        if (bindFlags & Bind_UnorderedAccess)flags |= D3D11_BIND_UNORDERED_ACCESS;

        return flags;
    }

    DX11Buffer::DX11Buffer(
        ID3D11Device* device,
        ID3D11DeviceContext* context,
        const BufferDesc& desc,
        const void* initialData)
        : m_desc(desc), m_context(context)
    {
        D3D11_BUFFER_DESC dxDesc = {};
        dxDesc.ByteWidth = static_cast<UINT>(desc.size);
        dxDesc.BindFlags = ToDXBindFlags(desc.bindFlags);
        dxDesc.Usage = ToDXUsage(desc.memory);
        dxDesc.CPUAccessFlags = ToDXCPUAccess(desc.memory);
        dxDesc.MiscFlags = 0;
        dxDesc.StructureByteStride = 0;

        if (desc.type == BufferType::Structured)
        {
            dxDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
            dxDesc.StructureByteStride = static_cast<UINT>(desc.stride);
        }

        D3D11_SUBRESOURCE_DATA subData = {};
        D3D11_SUBRESOURCE_DATA* pData = nullptr;

        if (initialData)
        {
            subData.pSysMem = initialData;
            pData = &subData;
        }

        HRESULT hr = device->CreateBuffer(&dxDesc, pData, &m_buffer);
        assert(SUCCEEDED(hr));
    }

    const RHI::BufferDesc& DX11Buffer::GetDesc() const {
        return m_desc;
    }

    DX11Buffer::~DX11Buffer() = default;

    void DX11Buffer::Update(const void* data, size_t size, size_t offset)
    {
        assert(offset + size <= m_desc.size);

        switch (m_desc.memory)
        {
        case MemoryType::CPUUpload:
        {
            D3D11_MAPPED_SUBRESOURCE mapped;
            HRESULT hr = m_context->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            assert(SUCCEEDED(hr));

            std::memcpy((uint8_t*)mapped.pData + offset, data, size);

            m_context->Unmap(m_buffer.Get(), 0);
            break;
        }

        case MemoryType::GPUOnly:
        {
            D3D11_BOX box = {};
            box.left = static_cast<UINT>(offset);
            box.right = static_cast<UINT>(offset + size);
            box.top = 0;
            box.bottom = 1;
            box.front = 0;
            box.back = 1;

            m_context->UpdateSubresource(m_buffer.Get(), 0, &box, data, 0, 0);
            break;
        }

        case MemoryType::CPUReadback:
            assert(false && "Cannot Update a readback buffer!");
            break;
        }
    }

    void* DX11Buffer::Map()
    {
        D3D11_MAP dxType;

        switch (m_desc.memory)
        {
        case RHI::MemoryType::CPUUpload:
            dxType = D3D11_MAP_WRITE_DISCARD;
            break;

        case RHI::MemoryType::CPUReadback:
            dxType = D3D11_MAP_READ;
            break;

        default:
            assert(false && "GPUOnly buffer cannot be mapped!");
            return nullptr;
        }

        D3D11_MAPPED_SUBRESOURCE mapped;
        HRESULT hr = m_context->Map(m_buffer.Get(), 0, dxType, 0, &mapped);

        if (FAILED(hr)) return nullptr;

        return mapped.pData;
    }

    void DX11Buffer::Unmap()
    {
        m_context->Unmap(m_buffer.Get(), 0);
    }

    DX11BufferView::DX11BufferView(
        ID3D11Device* device,
        DX11Buffer* buffer,
        const RHI::BufferViewDesc& desc) : m_buffer(buffer), m_desc(desc)
    {
        if (desc.type == RHI::BufferViewDesc::ViewType::Structured)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;

            srvDesc.Buffer.FirstElement = static_cast<UINT>(desc.offset / desc.stride);
            srvDesc.Buffer.NumElements = static_cast<UINT>(desc.size / desc.stride);

            device->CreateShaderResourceView(buffer->GetNative(), &srvDesc, &m_srv);
        }
    }
}