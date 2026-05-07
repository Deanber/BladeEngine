#pragma once
#include <cstdint>
#include "IResource.h"
#include <Viewport.h>

namespace RHI {

    class IPipelineLayout;
    class IBindGroup;
    class IPipelineState;
    class IBuffer;
    class ITextureView;

    class ICommandList
    {
    public:
        virtual ~ICommandList() = default;

        // Frame
        virtual void Begin() = 0;
        virtual void End() = 0;

        // Resource barrier£¨DX11¿ÕÊµÏÖ£¬Vulkan¹Ø¼ü£©
        virtual void Transition(
            RHI::IResource* resource,
            ResourceAccessState before,
            ResourceAccessState after
        ) = 0;

        // Pipeline
        virtual void SetPipelineState(RHI::IPipelineState*) = 0;
        virtual void SetPipelineLayout(RHI::IPipelineLayout*) = 0;

        virtual void SetBindGroup(uint32_t index, IBindGroup*) = 0;

        // Geometry
        virtual void SetVertexBuffer(IBuffer*) = 0;
        virtual void SetIndexBuffer(IBuffer*, IndexFormat, uint32_t offset) = 0;

        virtual void SetRenderTargets(
            ITextureView** colorRTs,
            uint32_t count,
            ITextureView* depthRT
        ) = 0;

        virtual void ClearRenderTarget(ITextureView*, const float color[4]) = 0;

        // Draw
        virtual void Draw(uint32_t vertexCount, uint32_t firstVertex = 0) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0) = 0;

        // Constants
        virtual void SetPushConstants(uint32_t offset, const void* data, size_t size) = 0;

        // Viewport
        virtual void SetViewport(const Viewport& viewport) = 0;

        // Compute
        virtual void Dispatch(uint32_t x, uint32_t y, uint32_t z) = 0;
    };

}