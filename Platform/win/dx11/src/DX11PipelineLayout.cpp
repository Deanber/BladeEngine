#include "DX11PipelineLayout.h"
#include <cassert>

namespace DX11 {

    DX11PipelineLayout::DX11PipelineLayout(const RHI::PipelineLayoutDesc& desc)
        : m_desc(desc)
    {
        BuildMapping();
    }



    void DX11PipelineLayout::BuildMapping()
    {
        m_setBases.resize(m_desc.bindGroupLayouts.size());

        uint32_t globalCBV = 0;
        uint32_t globalSRV = 0;
        uint32_t globalUAV = 0;
        uint32_t globalSampler = 0;

        for (size_t setIndex = 0; setIndex < m_desc.bindGroupLayouts.size(); ++setIndex)
        {
            auto* layout = static_cast<DX11BindGroupLayout*>(m_desc.bindGroupLayouts[setIndex]);
            assert(layout);

            auto& base = m_setBases[setIndex];

            base.cbvBase = globalCBV;
            base.srvBase = globalSRV;
            base.uavBase = globalUAV;
            base.samplerBase = globalSampler;

            const auto& bindings = layout->GetBindings();

            uint32_t maxCBV = 0;
            uint32_t maxSRV = 0;
            uint32_t maxUAV = 0;
            uint32_t maxSampler = 0;

            for (const auto& b : bindings)
            {
                switch (b.type)
                {
                case RHI::ShaderResourceType::ConstantBuffer:
                    maxCBV = (std::max)(maxCBV, b.binding + 1);
                    break;

                case RHI::ShaderResourceType::Texture:
                case RHI::ShaderResourceType::StructuredBuffer:
                case RHI::ShaderResourceType::ByteAddressBuffer:
                    maxSRV = (std::max)(maxSRV, b.binding + 1);
                    break;

                case RHI::ShaderResourceType::RWTexture:
                case RHI::ShaderResourceType::RWStructuredBuffer:
                case RHI::ShaderResourceType::RWByteAddressBuffer:
                    maxUAV = (std::max)(maxUAV, b.binding + 1);
                    break;

                case RHI::ShaderResourceType::Sampler:
                    maxSampler = (std::max)(maxSampler, b.binding + 1);
                    break;

                default:
                    break;
                }
            }

            // 记录 count（用于校验）
            base.cbvCount = maxCBV;
            base.srvCount = maxSRV;
            base.uavCount = maxUAV;
            base.samplerCount = maxSampler;

            globalCBV += maxCBV;
            globalSRV += maxSRV;
            globalUAV += maxUAV;
            globalSampler += maxSampler;
        }

        // DX11 上限检查（非常重要）
        assert(globalCBV <= D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
        assert(globalSRV <= D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
        assert(globalSampler <= D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);
        assert(globalUAV <= D3D11_PS_CS_UAV_REGISTER_COUNT);
    }

    uint32_t DX11PipelineLayout::GetSlot(uint32_t set, uint32_t binding, RHI::ShaderResourceType type) const
    {
        assert(set < m_setBases.size());
        const auto& base = m_setBases[set];

        switch (type)
        {
        case RHI::ShaderResourceType::ConstantBuffer:
            assert(binding < base.cbvCount);
            return base.cbvBase + binding;

        case RHI::ShaderResourceType::Texture:
        case RHI::ShaderResourceType::StructuredBuffer:
        case RHI::ShaderResourceType::ByteAddressBuffer:
            assert(binding < base.srvCount);
            return base.srvBase + binding;

        case RHI::ShaderResourceType::RWTexture:
        case RHI::ShaderResourceType::RWStructuredBuffer:
        case RHI::ShaderResourceType::RWByteAddressBuffer:
            assert(binding < base.uavCount);
            return base.uavBase + binding;

        case RHI::ShaderResourceType::Sampler:
            assert(binding < base.samplerCount);
            return base.samplerBase + binding;

        default:
            break;
        }

        assert(false && "Unsupported resource type");
        return 0;
    }
}