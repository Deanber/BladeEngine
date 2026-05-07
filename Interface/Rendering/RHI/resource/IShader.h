#pragma once
#include <vector>
#include <string>
#include "RHIEnum.h"
#include "IResource.h"

namespace RHI {
    struct ShaderMacro
    {
        std::string name;
        std::string value;
    };

    struct ShaderDesc
    {
        ShaderStage stage;

        std::string source;
        const void* bytecode = nullptr;
        size_t bytecodeSize = 0;

        std::string entryPoint = "main";
        std::string profile;

        std::vector<ShaderMacro> macros;

        std::string debugName;
    };

    struct ShaderResourceBinding
    {
        std::string name;
        ShaderResourceType type;

        uint32_t binding;
        uint32_t set;
        uint32_t space;

        uint32_t arraySize = 1;
        uint32_t sizeInBytes = 0;
    };

    class IShaderReflection
    {
    public:
        virtual ~IShaderReflection() = default;

        virtual const std::vector<RHI::ShaderResourceBinding>& GetBindings() const = 0;
    };

    class IShader : public IResource
    {
    public:
        virtual ~IShader() = default;

        virtual ShaderStage GetStage() const = 0;

        virtual const RHI::ShaderDesc& GetDesc() const = 0;

        virtual const void* GetByteCode() const = 0;
        virtual size_t GetByteCodeSize() const = 0;

        virtual RHI::IShaderReflection* GetReflection() const = 0; // ¿ÉÎª nullptr
    };
}