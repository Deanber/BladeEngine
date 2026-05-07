#pragma once
#include <memory>
#include "IPipelineState.h"
#include "IBindGroup.h"
#include "RenderPass.h"
#include "PushConstantRange.h"
#include "RHIEnum.h"

namespace RHI {
    class IBuffer;
    class IShader;

    enum class CommandBufferState
    {
        Idle,
        Recording,
        Executable,
        Submitted
    };

    class ICommandBuffer
    {
    public:
        virtual ~ICommandBuffer() = default;
        virtual void PushConstants(ShaderStage stage, uint32_t offset, uint32_t size, const void* data) = 0;
        // ·ÇË÷Òý
        virtual void DrawIndirect(std::shared_ptr<IBuffer> buffer, uint32_t offset = 0, uint32_t drawCount = 1) = 0;
        // Ë÷Òý
        virtual void DrawIndexedIndirect(std::shared_ptr<IBuffer> buffer, uint32_t offset = 0, uint32_t drawCount = 1) = 0;
        virtual void SetVertexBuffer(uint32_t slot, std::shared_ptr<IBuffer> buffer, size_t offset = 0) = 0;
        virtual void SetIndexBuffer(std::shared_ptr<IBuffer> buffer, size_t offset = 0, IndexFormat type = IndexFormat::Uint32) = 0;
        virtual void CopyBuffer(std::shared_ptr<IBuffer> src, std::shared_ptr<IBuffer> dst) = 0;
        virtual void CopyBufferToTexture(std::shared_ptr<IBuffer> src, std::shared_ptr<ITexture> dst) = 0;

        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual void Reset() = 0;
        virtual void SetPipelineState(std::shared_ptr<IPipelineState> pso) = 0;
        virtual void SetBindGroup(uint32_t index, std::shared_ptr<IBindGroup> group) = 0;
        virtual void BeginRenderPass(const RenderPassDesc& desc) = 0;
        virtual void EndRenderPass() = 0;
        virtual void PipelineBarrier(const std::vector<BarrierDesc>& barriers) = 0;
        virtual void Draw(uint32_t vertexCount, uint32_t startVertex = 0) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t startIndex = 0) = 0;
        virtual void Dispatch(uint32_t x, uint32_t y, uint32_t z) = 0;
    };
}