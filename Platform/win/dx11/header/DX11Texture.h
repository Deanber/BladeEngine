#pragma once

#include "ITexture.h"
#include <wrl/client.h>

struct ID3D11Texture2D;
struct ID3D11Device;
struct ID3D11DeviceContext;

namespace DX11
{

    class DX11Texture : public RHI::ITexture
    {
    public:
        DX11Texture(
            ID3D11Device* device,
            ID3D11DeviceContext* context,
            const RHI::TextureDesc& desc,
            const void* initialData = nullptr);

        ~DX11Texture();

        const RHI::TextureDesc& GetDesc() const override { return m_desc; }

        void Update(const RHI::TextureUpdateDesc& desc) override;

        ID3D11Texture2D* GetNative() const { return m_texture.Get(); }

        RHI::ResourceType GetType() const override { return RHI::ResourceType::Texture; }

        RHI::ResourceAccessState GetState() const override { return m_state; }
        void SetState(RHI::ResourceAccessState state) override { m_state = state; }

        // 对于 Texture，NativeHandle 通常是 ID3D11Resource* 或 ID3D11Texture2D*
        void* GetNativeHandle() const override { return m_texture.Get(); }

        // Texture 的子资源数量通常是 MipLevels * ArraySize
        uint32_t GetSubresourceCount() const override {
            return m_desc.mipLevels * m_desc.arraySize;
        }

    private:
        RHI::TextureDesc m_desc;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;

        ID3D11DeviceContext* m_context = nullptr;
        RHI::ResourceAccessState m_state = RHI::ResourceAccessState::Undefined;
    };

}