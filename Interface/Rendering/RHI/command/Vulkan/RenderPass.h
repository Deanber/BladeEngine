#pragma once
#include <cstdint>

namespace RHI {
    class ITexture;

    struct RenderTargetDesc
    {
        float clearColor[4];
    };

    struct RenderPassDesc
    {
        // RenderTarget
        ITexture* colorAttachments[8];

        // Clear
        float clearColor[4];
        float clearDepth;
        uint32_t clearStencil;

        bool clearColorEnabled;
        bool clearDepthEnabled;
    };
}