#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <cassert>
#include <RHIEnum.h>
#include "VertexInputState.h"

namespace RHI {

    class IPipelineLayout;
    class IShader;
    class InputLayoutDesc;

    inline void HashCombine(size_t& seed, size_t value)
    {
        seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
    }

    struct RasterState
    {
        bool wireframe = false;
        CullMode cull = CullMode::Back;

        bool frontCCW = false;

        int depthBias = 0;
        float slopeScaledDepthBias = 0.0f;

        bool depthClip = true;
        bool scissor = false;

        bool multisample = false;
        bool antialiasedLine = false;

        bool operator==(const RasterState& o) const
        {
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

        inline size_t Hash(const RasterState& r)
        {
            size_t h = 0;

            HashCombine(h, r.wireframe);
            HashCombine(h, static_cast<size_t>(r.cull));
            HashCombine(h, r.frontCCW);

            HashCombine(h, std::hash<int>{}(r.depthBias));
            HashCombine(h, std::hash<float>{}(r.slopeScaledDepthBias));

            HashCombine(h, r.depthClip);
            HashCombine(h, r.scissor);
            HashCombine(h, r.multisample);
            HashCombine(h, r.antialiasedLine);

            return h;
        }
    };

    

    

    struct StencilOpState
    {
        CompareFunc func = CompareFunc::Always;

        StencilOp failOp = StencilOp::Keep;
        StencilOp depthFailOp = StencilOp::Keep;
        StencilOp passOp = StencilOp::Keep;

        uint8_t readMask = 0xff;
        uint8_t writeMask = 0xff;

        bool operator==(const StencilOpState& o) const
        {
            return func == o.func &&
                failOp == o.failOp &&
                depthFailOp == o.depthFailOp &&
                passOp == o.passOp &&
                readMask == o.readMask &&
                writeMask == o.writeMask;
        }
    };

    struct DepthState
    {
        bool enable = true;
        bool write = true;
        CompareFunc func = CompareFunc::Less;

        bool stencilEnable = false;
        StencilOpState front;
        StencilOpState back;

        bool operator==(const DepthState& o) const
        {
            if (enable != o.enable ||
                write != o.write ||
                func != o.func ||
                stencilEnable != o.stencilEnable)
                return false;

            if (stencilEnable)
            {
                if (!(front == o.front) || !(back == o.back))
                    return false;
            }

            return true;
        }
    };

    

    struct RenderTargetBlend
    {
        bool enable = false;
        uint8_t writeMask = 0x0F;

        BlendFactor srcColor = BlendFactor::One;
        BlendFactor dstColor = BlendFactor::Zero;
        BlendOp colorOp = BlendOp::Add;

        BlendFactor srcAlpha = BlendFactor::One;
        BlendFactor dstAlpha = BlendFactor::Zero;
        BlendOp alphaOp = BlendOp::Add;
    };

    inline bool operator==(const RenderTargetBlend& a, const RenderTargetBlend& b)
    {
        return a.enable == b.enable &&
            a.writeMask == b.writeMask &&
            a.srcColor == b.srcColor &&
            a.dstColor == b.dstColor &&
            a.colorOp == b.colorOp &&
            a.srcAlpha == b.srcAlpha &&
            a.dstAlpha == b.dstAlpha &&
            a.alphaOp == b.alphaOp;
    }

    struct BlendState
    {
        RenderTargetBlend targets[8]{};
        bool independentBlend = false;
        uint32_t targetCount = 1;

        float blendFactor[4] = { 1.f, 1.f, 1.f, 1.f };

        void Normalize()
        {
            for (uint32_t i = targetCount; i < 8; ++i)
                targets[i] = {};
        }

        bool operator==(const BlendState& o) const
        {
            if (independentBlend != o.independentBlend ||
                targetCount != o.targetCount)
                return false;

            // 比较 targets
            for (uint32_t i = 0; i < targetCount; ++i)
            {
                if (!(targets[i] == o.targets[i]))
                    return false;
            }

            // 比较 blendFactor
            for (int i = 0; i < 4; ++i)
            {
                if (blendFactor[i] != o.blendFactor[i])
                    return false;
            }

            return true;
        }
    };

    struct VertexElement {
        std::string semantic;
        uint32_t semanticIndex;
        VertexFormat format;
        uint32_t slot;
        uint32_t offset;
    };

    struct RenderTargetState
    {
        std::vector<TextureFormat> colorFormats;
        TextureFormat depthFormat = TextureFormat::Unknown;

        uint32_t sampleCount = 1;

        void Validate() const
        {
            assert(colorFormats.size() <= 8);
        }

        bool operator==(const RenderTargetState& o) const
        {
            if (sampleCount != o.sampleCount)
                return false;

            for (uint32_t i = 0; i < sampleCount; ++i)
            {
                if (colorFormats[i] != o.colorFormats[i])
                    return false;
            }

            return true;
        }
    };

    struct InputLayoutDesc {
        std::vector<VertexElement> elements;
        std::vector<VertexBufferLayout> buffers;
    };

    struct PipelineStateDesc
    {
        IShader* vs = nullptr;
        IShader* ps = nullptr;
        IPipelineLayout* layout = nullptr;

        VertexInputState inputLayout;   // 唯一入口

        RenderTargetState renderTargets;
        RasterState raster;
        DepthState depth;
        BlendState blend;

        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
        uint32_t sampleMask = 0xFFFFFFFF;

        bool operator==(const PipelineStateDesc& o) const
        {
            return
                vs == o.vs &&
                ps == o.ps &&
                layout == o.layout &&
                inputLayout == o.inputLayout &&
                renderTargets == o.renderTargets &&
                raster == o.raster &&
                depth == o.depth &&
                blend == o.blend &&
                topology == o.topology &&
                sampleMask == o.sampleMask;
        }
    };

    
}