#pragma once

#include "IBuffer.h"
#include <wrl/client.h>
#include "IResource.h"
#include <d3d11.h>

namespace DX11
{
    class DX11Buffer : public RHI::IBuffer
    {
    public:
        DX11Buffer(
            ID3D11Device* device,
            ID3D11DeviceContext* context,
            const RHI::BufferDesc& desc,
            const void* initialData = nullptr);

        ~DX11Buffer();

        const RHI::BufferDesc& GetDesc() const override;

        void Update(const void* data, size_t size, size_t offset = 0) override;

        void* Map() override;
        void Unmap() override;

        ID3D11Buffer* GetNative() const { return m_buffer.Get(); }

        RHI::ResourceType GetType() const override { return RHI::ResourceType::Buffer; }

        RHI::ResourceAccessState GetState() const override { return m_state; }
        void SetState(RHI::ResourceAccessState state) override { m_state = state; }

        void* GetNativeHandle() const override { return m_buffer.Get(); }

        uint32_t GetSubresourceCount() const override { return 1; }


    private:
        RHI::BufferDesc m_desc;

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;

        ID3D11DeviceContext* m_context = nullptr;

        RHI::ResourceAccessState m_state = RHI::ResourceAccessState::Undefined;
    };

    class DX11BufferView : public RHI::IBufferView
    {
    public:
        DX11BufferView(
            ID3D11Device* device,
            DX11Buffer* buffer,
            const RHI::BufferViewDesc& desc);

        RHI::IBuffer* GetBuffer() const override { return m_buffer; }
        const RHI::BufferViewDesc& GetDesc() const override { return m_desc; }

        void* GetSRV() const override
        {
            return m_srv.Get();
        }

        void* GetUAV() const override
        {
            return m_uav.Get();
        }

    private:
        RHI::BufferViewDesc m_desc;
        DX11Buffer* m_buffer = nullptr;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;
        Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_uav;
    };
}