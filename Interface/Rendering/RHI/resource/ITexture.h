#pragma once
#include <cstdint>
#include "RHIEnum.h"
#include "IResource.h"

namespace RHI 
{
    struct TextureDesc
    {
        TextureDimension type = TextureDimension::Texture2D;

        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;

        uint32_t mipLevels = 1;
        uint32_t arraySize = 1;

        uint32_t sampleCount = 1;

        TextureFormat format = TextureFormat::RGBA8_UNorm;

        MemoryType memory = MemoryType::GPUOnly;
        uint32_t bindFlags = 0;

        bool isCube = false;

        struct ClearValue
        {
            float color[4] = { 0,0,0,0 };
            float depth = 1.0f;
            uint32_t stencil = 0;
        } clearValue;
    };

    struct TextureUpdateDesc
    {
        const void* data;

        uint32_t mipLevel = 0;
        uint32_t arraySlice = 0;

        uint32_t width;
        uint32_t height;
        uint32_t depth;

        uint32_t rowPitch;
        uint32_t slicePitch;
    };

    class ITexture : public IResource
    {
    public:
        virtual ~ITexture() = default;

        virtual const TextureDesc& GetDesc() const = 0;

        virtual void Update(const TextureUpdateDesc& desc) = 0;
    };

    struct TextureViewDesc
    {
        TextureViewUsage usage;
        TextureViewDimension dimension;

        TextureFormat format;

        uint32_t baseMip = 0;
        uint32_t mipCount = 1;

        uint32_t baseLayer = 0;
        uint32_t layerCount = 1;
    };

    class ITextureView : public IDeviceObject
    {
    public:
        virtual ~ITextureView() = default;

        virtual ITexture* GetTexture() const = 0;

        virtual const TextureViewDesc& GetDesc() const = 0;
        virtual void* GetSRV() const = 0;
        virtual void* GetUAV() const = 0;
        virtual void* GetNativeRTV() const = 0;
        virtual void* GetNativeDSV() const = 0;
    };
}