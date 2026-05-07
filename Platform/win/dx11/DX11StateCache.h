#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <unordered_map>
#include <vector>
#include <functional>
#include <cstdint>
#include <type_traits>
#include <array>

#include "PipelineStateDesc.h"

namespace DX11 {

    // Raster
    struct RasterKey {
        bool wireframe;
        RHI::CullMode cull;
        bool frontCCW;

        int depthBias;
        float slopeScaledDepthBias;

        bool depthClip;
        bool scissor;

        bool multisample;
        bool antialiasedLine;

        //RasterKey() = default;

        bool operator==(const RasterKey& o) const {
            return wireframe == o.wireframe &&
                cull == o.cull &&
                frontCCW == o.frontCCW &&
                depthBias == o.depthBias &&
                slopeScaledDepthBias == o.slopeScaledDepthBias &&
                depthClip == o.depthClip &&
                scissor == o.scissor &&
                multisample == o.multisample &&
                antialiasedLine == o.antialiasedLine;
        }
    };

    // Stencil
    struct StencilKey {
        RHI::CompareFunc func;
        RHI::StencilOp failOp;
        RHI::StencilOp depthFailOp;
        RHI::StencilOp passOp;
        uint8_t readMask;
        uint8_t writeMask;

        StencilKey()
            : func(RHI::CompareFunc::Always),
            failOp(RHI::StencilOp::Keep),
            depthFailOp(RHI::StencilOp::Keep),
            passOp(RHI::StencilOp::Keep),
            readMask(0xff),
            writeMask(0xff)
        {
        }

        StencilKey(RHI::CompareFunc f,
            RHI::StencilOp fail,
            RHI::StencilOp dfail,
            RHI::StencilOp pass,
            uint8_t rm,
            uint8_t wm)
            : func(f),
            failOp(fail),
            depthFailOp(dfail),
            passOp(pass),
            readMask(rm),
            writeMask(wm)
        {
        }

        bool operator==(const StencilKey& o) const {
            return func == o.func &&
                failOp == o.failOp &&
                depthFailOp == o.depthFailOp &&
                passOp == o.passOp &&
                readMask == o.readMask &&
                writeMask == o.writeMask;
        }
    };

    // Depth
    struct DepthKey {
        bool enable = false;
        bool write = false;
        RHI::CompareFunc func = RHI::CompareFunc::Always;
        bool stencilEnable = false;

        StencilKey front{};
        StencilKey back{};

        DepthKey() = default;

        DepthKey(bool e, bool w, RHI::CompareFunc f,
            bool sEnable, StencilKey fKey, StencilKey bKey)
            : enable(e),
            write(w),
            func(f),
            stencilEnable(sEnable),
            front(fKey),
            back(bKey)
        {
        }

        bool operator==(const DepthKey& o) const {
            return enable == o.enable &&
                write == o.write &&
                func == o.func &&
                stencilEnable == o.stencilEnable &&
                front == o.front &&
                back == o.back;
        }
    };

    struct BlendKey {
        std::array<RHI::RenderTargetBlend, 8> targets;
        uint32_t targetCount = 0;
        bool independentBlend = false;

        bool operator==(const BlendKey& o) const {
            if (targetCount != o.targetCount || independentBlend != o.independentBlend)
                return false;

            for (uint32_t i = 0; i < targetCount; ++i) {
                if (!(targets[i] == o.targets[i])) return false;
            }
            return true;
        }
    };

}

// std hash specializations
namespace std {

    inline void HashCombine(size_t& seed, size_t v)
    {
        seed ^= v + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    // Raster
    template<>
    struct hash<DX11::RasterKey>
    {
        size_t operator()(const DX11::RasterKey& k) const noexcept
        {
            size_t h = 0;
            HashCombine(h, k.wireframe);
            HashCombine(h, (int)k.cull);
            HashCombine(h, k.frontCCW);
            HashCombine(h, k.depthBias);
            HashCombine(h, std::hash<float>{}(k.slopeScaledDepthBias));
            HashCombine(h, k.depthClip);
            HashCombine(h, k.scissor);
            HashCombine(h, k.multisample);
            HashCombine(h, k.antialiasedLine);
            return h;
        }
    };

    // Stencil
    template<>
    struct hash<DX11::StencilKey>
    {
        size_t operator()(const DX11::StencilKey& k) const noexcept
        {
            size_t h = 0;
            HashCombine(h, (int)k.func);
            HashCombine(h, (int)k.failOp);
            HashCombine(h, (int)k.depthFailOp);
            HashCombine(h, (int)k.passOp);
            HashCombine(h, k.readMask);
            HashCombine(h, k.writeMask);
            return h;
        }
    };

    // Depth
    template<>
    struct hash<DX11::DepthKey>
    {
        size_t operator()(const DX11::DepthKey& k) const noexcept
        {
            size_t h = 0;
            HashCombine(h, k.enable);
            HashCombine(h, k.write);
            HashCombine(h, (int)k.func);
            HashCombine(h, k.stencilEnable);

            HashCombine(h, hash<DX11::StencilKey>{}(k.front));
            HashCombine(h, hash<DX11::StencilKey>{}(k.back));

            return h;
        }
    };

    // Blend
    template<>
    struct hash<DX11::BlendKey>
    {
        size_t operator()(const DX11::BlendKey& k) const noexcept
        {
            size_t h = 0;

            HashCombine(h, k.independentBlend);
            HashCombine(h, k.targetCount);

            for (uint32_t i = 0; i < k.targetCount; ++i)
            {
                const auto& t = k.targets[i];
                HashCombine(h, t.enable);
                HashCombine(h, t.writeMask);
                HashCombine(h, (int)t.srcColor);
                HashCombine(h, (int)t.dstColor);
                HashCombine(h, (int)t.colorOp);
                HashCombine(h, (int)t.srcAlpha);
                HashCombine(h, (int)t.dstAlpha);
                HashCombine(h, (int)t.alphaOp);
            }

            return h;
        }
    };
}

namespace DX11 {
    // Cache
    class DX11StateCache {
    public:
        DX11StateCache() = default;
        explicit DX11StateCache(ID3D11Device* device) : m_device(device) {}
        using RasterPtr = Microsoft::WRL::ComPtr<ID3D11RasterizerState>;
        using DepthPtr = Microsoft::WRL::ComPtr<ID3D11DepthStencilState>;
        using BlendPtr = Microsoft::WRL::ComPtr<ID3D11BlendState>;

        RasterPtr GetRaster(ID3D11Device* device, const RHI::RasterState& rs);
        DepthPtr  GetDepth(ID3D11Device* device, const RHI::DepthState& ds);
        BlendPtr  GetBlend(ID3D11Device* device, const RHI::BlendState& bs);

    private:
        ID3D11Device* m_device = nullptr;
        std::unordered_map<RasterKey, RasterPtr> m_rasterCache;
        std::unordered_map<DepthKey, DepthPtr>    m_depthCache;
        std::unordered_map<BlendKey, BlendPtr>    m_blendCache;
    };
}