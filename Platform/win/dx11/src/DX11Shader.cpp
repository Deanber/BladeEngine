#include "DX11Shader.h"
#include <d3dcompiler.h>
#include <cstring>
#include <wrl/client.h>
#include <d3d11shader.h>
#include <memory>
#include <unordered_map>
#include <filesystem>
#include <mutex>
#include <algorithm>
#include <cstdio>
#pragma comment(lib, "d3dcompiler.lib")
#include <filesystem>
namespace fs = std::filesystem;

namespace DX11 {

    const std::vector<RHI::ShaderResourceBinding>& DX11ShaderReflection::GetBindings() const
    {
        return m_bindings;
    }

    enum DX11ShaderType
    {
        DX11_VS,
        DX11_PS,
        DX11_CS,
        DX11_GS,
        DX11_HS,
        DX11_DS
    };

    static DX11ShaderType MapShaderStageToDX11(RHI::ShaderStage stage)
    {
        switch (stage)
        {
        case RHI::ShaderStage::Vertex:   return DX11_VS;
        case RHI::ShaderStage::Fragment: return DX11_PS;  // Fragment → Pixel Shader
        case RHI::ShaderStage::Compute:  return DX11_CS;
        case RHI::ShaderStage::Geometry: return DX11_GS;
        case RHI::ShaderStage::Hull:     return DX11_HS;
        case RHI::ShaderStage::Domain:   return DX11_DS;
        default:
            throw std::runtime_error("Unsupported ShaderStage");
        }
    }

    static std::string GetShaderTarget(RHI::ShaderStage stage, const std::string& version = "5_0")
    {
        switch (stage)
        {
        case RHI::ShaderStage::Vertex:   return "vs_" + version;
        case RHI::ShaderStage::Fragment: return "ps_" + version;
        case RHI::ShaderStage::Compute:  return "cs_" + version;
        case RHI::ShaderStage::Geometry: return "gs_" + version;
        case RHI::ShaderStage::Hull:     return "hs_" + version;
        case RHI::ShaderStage::Domain:   return "ds_" + version;
        default:
            throw std::runtime_error("Unsupported ShaderStage");
        }
    }

    static std::vector<D3D_SHADER_MACRO> PrepareMacros(const std::vector<RHI::ShaderMacro>& macros)
    {
        std::vector<D3D_SHADER_MACRO> dxMacros;
        dxMacros.reserve(macros.size() + 1);

        for (const auto& m : macros)
            dxMacros.push_back({ m.name.c_str(), m.value.c_str() });

        dxMacros.push_back({ nullptr, nullptr });
        return dxMacros;
    }

    /// <summary>
    /// 创建Shader对象
    /// </summary>
    /// <param name="device"></param>
    void DX11Shader::CreateNativeShader(ID3D11Device* device)
    {
        if (m_byteCode.empty()) {
            throw std::runtime_error("DX11Shader: Bytecode is empty before creating native shader.");
        }

        HRESULT hr = S_OK;
        switch (MapShaderStageToDX11(m_desc.stage))
        {
        case DX11_VS:
        {
            ID3D11VertexShader* vs = nullptr;
            hr = device->CreateVertexShader(m_byteCode.data(), m_byteCode.size(), nullptr, &vs);
            m_shader = vs; // ComPtr 会自动接管引用计数
            break;
        }
        case DX11_PS:
        {
            ID3D11PixelShader* ps = nullptr;
            hr = device->CreatePixelShader(m_byteCode.data(), m_byteCode.size(), nullptr, &ps);
            m_shader = ps;
            break;
        }
        case DX11_CS:
        {
            ID3D11ComputeShader* cs = nullptr;
            hr = device->CreateComputeShader(m_byteCode.data(), m_byteCode.size(), nullptr, &cs);
            m_shader = cs;
            break;
        }
        case DX11_GS:
        {
            ID3D11GeometryShader* gs = nullptr;
            hr = device->CreateGeometryShader(m_byteCode.data(), m_byteCode.size(), nullptr, &gs);
            m_shader = gs;
            break;
        }
        case DX11_HS:
        {
            ID3D11HullShader* hs = nullptr;
            hr = device->CreateHullShader(m_byteCode.data(), m_byteCode.size(), nullptr, &hs);
            m_shader = hs;
            break;
        }
        case DX11_DS:
        {
            ID3D11DomainShader* ds = nullptr;
            hr = device->CreateDomainShader(m_byteCode.data(), m_byteCode.size(), nullptr, &ds);
            m_shader = ds;
            break;
        }
        default:
            throw std::runtime_error("Unsupported DX11 shader type");
        }

        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create native DX11 shader object. HRESULT: " + std::to_string(hr));
        }
    }

    DX11Shader::DX11Shader(ID3D11Device* device, const RHI::ShaderDesc& desc)
        : m_desc(desc)
    {
        if (!device)
            throw std::runtime_error("DX11Shader: device is null");

        if (desc.bytecode && desc.bytecodeSize > 0)
        {
            m_byteCode.assign((uint8_t*)desc.bytecode, (uint8_t*)desc.bytecode + desc.bytecodeSize);
        }
        else if(!desc.source.empty())
        {
            CompileShader(device);
        }
        else {
            throw std::runtime_error("DX11Shader: No bytecode or source provided");
        }

        CreateNativeShader(device);

        Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflector;
        HRESULT hr = D3DReflect(m_byteCode.data(), m_byteCode.size(), IID_PPV_ARGS(&reflector));
        if (FAILED(hr))
        {
            m_reflection = std::make_unique<DX11ShaderReflection>(std::vector<RHI::ShaderResourceBinding>{});
            return;
        }

        D3D11_SHADER_DESC shaderDesc;
        reflector->GetDesc(&shaderDesc);

        std::vector<RHI::ShaderResourceBinding> bindings;

        for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
        {
            D3D11_SHADER_INPUT_BIND_DESC bindDesc;
            reflector->GetResourceBindingDesc(i, &bindDesc);

            RHI::ShaderResourceBinding binding;
            binding.name = bindDesc.Name;
            binding.binding = bindDesc.BindPoint;
            binding.set = 0;
            binding.space = 0;
            binding.arraySize = bindDesc.BindCount;

            switch (bindDesc.Type)
            {
            case D3D_SIT_CBUFFER: binding.type = RHI::ShaderResourceType::ConstantBuffer; break;
            case D3D_SIT_TBUFFER: binding.type = RHI::ShaderResourceType::StructuredBuffer; break;
            case D3D_SIT_TEXTURE: binding.type = RHI::ShaderResourceType::Texture; break;
            case D3D_SIT_SAMPLER: binding.type = RHI::ShaderResourceType::Sampler; break;
            default: binding.type = RHI::ShaderResourceType::Unknown; break;
            }

            if (bindDesc.Type == D3D_SIT_CBUFFER || bindDesc.Type == D3D_SIT_TBUFFER)
            {
                auto cb = reflector->GetConstantBufferByName(bindDesc.Name);
                D3D11_SHADER_BUFFER_DESC cbDesc;
                cb->GetDesc(&cbDesc);
                binding.sizeInBytes = cbDesc.Size;

                // 可以扩展保存成员信息，示例：
                // for (UINT v = 0; v < cbDesc.Variables; ++v)
                // {
                //     D3D11_SHADER_VARIABLE_DESC varDesc;
                //     cb->GetVariableByIndex(v)->GetDesc(&varDesc);
                //     // 保存 varDesc.Name / varDesc.StartOffset / varDesc.Size
                // }
            }

            bindings.push_back(binding);
        }

        m_reflection = std::make_unique<DX11ShaderReflection>(bindings);
    }

    //DX11Shader::~DX11Shader() = default;

    const RHI::ShaderDesc& DX11Shader::GetDesc() const { return m_desc; }
    RHI::ShaderStage DX11Shader::GetStage() const { return m_desc.stage; }
    const void* DX11Shader::GetByteCode() const { return m_byteCode.data(); }
    size_t DX11Shader::GetByteCodeSize() const { return m_byteCode.size(); }
    RHI::IShaderReflection* DX11Shader::GetReflection() const { return m_reflection.get(); }

    ID3D11DeviceChild* DX11Shader::GetDXShader() const { return m_shader.Get(); }

    class InlineInclude : public ID3DInclude
    {
    public:
        // 内存文件: filename -> content
        std::unordered_map<std::string, std::string> memoryFiles;

        // 根搜索路径
        std::vector<std::string> searchPaths;

        InlineInclude() = default;
        ~InlineInclude()
        {
            // 自动释放所有临时 buffer
            std::lock_guard<std::mutex> lock(mutex);
            for (auto& buf : tempBuffers)
                delete[] buf.data;
            tempBuffers.clear();
        }

        STDMETHOD(Open)(D3D_INCLUDE_TYPE, LPCSTR pFileName, LPCVOID, LPCVOID* ppData, UINT* pBytes) override
        {
            std::string fileName = pFileName;

            // 内存文件优先
            auto memIt = memoryFiles.find(fileName);
            if (memIt != memoryFiles.end())
            {
                *ppData = memIt->second.data();
                *pBytes = static_cast<UINT>(memIt->second.size());
                return S_OK;
            }

            // 磁盘搜索路径
            for (const auto& root : searchPaths)
            {
                fs::path fullPath = fs::path(root) / fileName;
                if (!fs::exists(fullPath)) continue;

                FILE* f = fopen(fullPath.string().c_str(), "rb");
                if (!f) continue;

                fseek(f, 0, SEEK_END);
                size_t size = ftell(f);
                fseek(f, 0, SEEK_SET);

                char* buffer = new char[size];
                fread(buffer, 1, size, f);
                fclose(f);

                {
                    std::lock_guard<std::mutex> lock(mutex);
                    tempBuffers.push_back({ buffer, size });
                }

                *ppData = buffer;
                *pBytes = static_cast<UINT>(size);
                return S_OK;
            }

            return E_FAIL; // 找不到
        }

        STDMETHOD(Close)(LPCVOID pData) override
        {
            std::lock_guard<std::mutex> lock(mutex);
            auto it = std::find_if(tempBuffers.begin(), tempBuffers.end(),
                [pData](const TempBuffer& b) { return b.data == pData; });
            if (it != tempBuffers.end())
            {
                delete[] it->data;
                tempBuffers.erase(it);
            }
            return S_OK;
        }

    private:
        struct TempBuffer
        {
            char* data = nullptr;
            size_t size = 0;
        };

        std::vector<TempBuffer> tempBuffers;
        std::mutex mutex; // 多线程保护
    };

    void DX11Shader::CompileShader(ID3D11Device* device)
    {
        UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        InlineInclude includeHandler;

        includeHandler.memoryFiles["Common.hlsl"] = "float4 SomeFunc() { return float4(1,1,1,1); }";

        includeHandler.searchPaths.push_back("shaders/includes");
        includeHandler.searchPaths.push_back("engine/shaders/common");
        auto dxMacros = PrepareMacros(m_desc.macros);
        OutputDebugStringA("\n===== VS SOURCE BEGIN =====\n");
        OutputDebugStringA(m_desc.source.c_str());
        OutputDebugStringA("\n===== VS SOURCE END =====\n");
        HRESULT hr = D3DCompile(
            m_desc.source.c_str(),
            m_desc.source.size(),
            nullptr,
            dxMacros.data(),
            &includeHandler,
            m_desc.entryPoint.c_str(),
            GetShaderTarget(m_desc.stage).c_str(),
            compileFlags,
            0,
            &shaderBlob,
            &errorBlob
        );

        if (FAILED(hr))
        {
            if (errorBlob)
            {
                const char* compileErrors = (const char*)errorBlob->GetBufferPointer();
                OutputDebugStringA("\n[HLSL Compile Error]:\n");
                OutputDebugStringA(compileErrors);
                OutputDebugStringA("\n");

                MessageBoxA(NULL, compileErrors, "HLSL 语法错误", MB_OK);
            }
            throw std::runtime_error("Shader 编译失败，详情请看输出窗口或弹窗。");
        }

        m_byteCode.resize(shaderBlob->GetBufferSize());
        memcpy(m_byteCode.data(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());
    }
}


