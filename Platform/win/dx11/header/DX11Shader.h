#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#include <memory> 
#include "IShader.h"
#include "IResource.h"
#include "RHIEnum.h"

namespace DX11
{
    class DX11ShaderReflection : public RHI::IShaderReflection
    {
    public:
        explicit DX11ShaderReflection(const std::vector<RHI::ShaderResourceBinding>& bindings)
            : m_bindings(bindings) {
        }

        const std::vector<RHI::ShaderResourceBinding>& GetBindings() const override;

    private:
        std::vector<RHI::ShaderResourceBinding> m_bindings;
    };

    class DX11Shader : public RHI::IShader
    {
    public:
        DX11Shader(ID3D11Device* device, const RHI::ShaderDesc& desc);
        virtual ~DX11Shader() = default;

        virtual RHI::ShaderStage GetStage() const override;
        virtual const RHI::ShaderDesc& GetDesc() const override;

        virtual const void* GetByteCode() const override;
        virtual size_t GetByteCodeSize() const override;

        virtual RHI::IShaderReflection* GetReflection() const;

        // 获取原生 DX11 shader 对象
        ID3D11DeviceChild* GetDXShader() const;

        RHI::ResourceType GetType() const override { return RHI::ResourceType::Shader; }

        // Shader 通常不需要状态转换，直接返回 Undefined 或 ShaderResource 即可
        RHI::ResourceAccessState GetState() const override { return RHI::ResourceAccessState::Undefined; }
        void SetState(RHI::ResourceAccessState state) override { /* Shader 状态通常是静态的，空实现即可 */ }

        // 返回底层的 ID3D11DeviceChild* (即各种着色器对象)
        void* GetNativeHandle() const override { return m_shader.Get(); }

        // Shader 没有子资源概念，返回 1
        uint32_t GetSubresourceCount() const override { return 1; }

    private:
        RHI::ShaderDesc m_desc;
        Microsoft::WRL::ComPtr<ID3D11DeviceChild> m_shader;
        std::vector<uint8_t> m_byteCode;

        std::unique_ptr<DX11ShaderReflection> m_reflection;

        // 内部函数
        void CompileShader(ID3D11Device* device);
        void CreateNativeShader(ID3D11Device* device);
    };
}