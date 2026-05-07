#pragma once
#include <memory>
#include <vector>
#include "IFence.h"
#include "IShader.h"

namespace RHI {
    class IBuffer;
    struct BufferDesc;
    class IShader;
    class ICommandList;
    class ITextureView;
    class ITexture;
    class TextureDesc;
    class TextureViewDesc;
    class IPipelineLayout;
    class PipelineLayoutDesc;
    class IPipelineState;
    class PipelineStateDesc;
    class IBindGroup;
    class IBindGroupLayout;

    struct RenderPassDesc {
        float clearColor[4] = { 0, 0, 0, 1 };
    };

    class IRenderer {
    public:
        virtual ~IRenderer() = default;

        virtual void Initialize(void* windowHandle) = 0;
        virtual void Shutdown() = 0;

        // Resource
        virtual std::shared_ptr<IBuffer> CreateBuffer(const BufferDesc& desc) = 0;
        virtual std::shared_ptr<ITexture> CreateTexture(const TextureDesc& desc, const void* initialData = nullptr) = 0;
        virtual std::shared_ptr<ITextureView> CreateTextureView(std::shared_ptr<ITexture> texture, const TextureViewDesc& desc) = 0;
        virtual std::shared_ptr<IShader> CreateShader(const ShaderDesc& desc) = 0;

        //Pipelineline
        virtual std::shared_ptr<IPipelineLayout> CreatePipelineLayout(const PipelineLayoutDesc& desc) = 0;
        virtual std::shared_ptr<IPipelineState> CreatePipelineState(const PipelineStateDesc& desc) = 0;
        virtual std::shared_ptr<IBindGroupLayout> CreateBindGroupLayout(const BindGroupLayoutDesc& desc) = 0;
        virtual std::shared_ptr<IBindGroup> CreateBindGroup(const BindGroupDesc& desc) = 0;

        //RenderPass
        virtual std::shared_ptr<IRenderPass> CreateRenderPass(const RenderPassDesc& desc) = 0;
        virtual std::shared_ptr<IFramebuffer> CreateFramebuffer(const FramebufferDesc& desc) = 0;
        
        // Command
        virtual std::shared_ptr<ICommandList> CreateCommandList() = 0;

        // Queue
        virtual std::shared_ptr<ICommandQueue> GetQueue(QueueType type = QueueType::Graphics) = 0;

        // Swapchain
        virtual std::shared_ptr<ISwapchain> CreateSwapchain(const SwapchainDesc& desc) = 0;

        // Sync
        virtual std::shared_ptr<IFence> CreateFence(uint64_t initialValue = 0) = 0;

        // Frame
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void WaitIdle() = 0;

        virtual ITextureView* GetCurrentBackBufferRTV() = 0;
        virtual ITextureView* GetDepthStencil() = 0;

        virtual void QueueSignal(IFence* fence, uint64_t value) = 0;
        virtual void QueueWait(IFence* fence, uint64_t value) = 0;
        virtual void Submit(ICommandList* cmd) = 0;
    };
}