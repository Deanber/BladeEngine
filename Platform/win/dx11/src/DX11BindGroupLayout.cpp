#include "DX11BindGroupLayout.h"
#include <cassert>
#include <algorithm>

namespace DX11 {

    DX11BindGroupLayout::DX11BindGroupLayout(const std::vector<RHI::BindGroupLayoutBinding>& bindings)
        : m_bindings(bindings)
    {
        for (const auto& b : bindings) {
            m_bindingMap[b.binding] = b; // 确保 Map 被填充
        }
        AnalyzeLayout();
    }

    void DX11BindGroupLayout::AnalyzeLayout()
    {
        m_infoCache.clear();

        for (const auto& binding : m_bindings)
        {
            auto stages = binding.visibility;

            // 遍历所有 stage
            for (RHI::ShaderStage stage : {
                RHI::ShaderStage::Vertex,
                    RHI::ShaderStage::Fragment,
                    RHI::ShaderStage::Compute
            })
            {
                if ((stages & stage) == RHI::ShaderStage::None)
                    continue;

                auto& typeMap = m_infoCache[stage];
                auto& info = typeMap[binding.type];

                if (!info.exists)
                {
                    info.minSlot = binding.binding;
                    info.maxSlot = binding.binding;
                    info.count = 1;
                    info.exists = true;
                }
                else
                {
                    info.minSlot = (std::min)(info.minSlot, binding.binding);
                    info.maxSlot = (std::max)(info.maxSlot, binding.binding);
                    info.count++;
                }
            }
        }
    }

    const StageLayoutInfo& DX11BindGroupLayout::GetStageInfo(RHI::ShaderStage stage, RHI::ShaderResourceType type) const
    {
        static StageLayoutInfo empty{};

        auto itStage = m_infoCache.find(stage);
        if (itStage == m_infoCache.end())
            return empty;

        auto itType = itStage->second.find(type);
        if (itType == itStage->second.end())
            return empty;

        return itType->second;
    }

    bool DX11BindGroupLayout::ValidateBinding(const RHI::BindGroupEntry& entry) const
    {
        auto it = m_bindingMap.find(entry.binding);
        if (it == m_bindingMap.end())
            return false;

        const auto& layoutBinding = it->second;

        // 类型必须匹配
        if (layoutBinding.type != entry.type)
            return false;

        // 资源合法性
        switch (entry.type)
        {
        case RHI::ShaderResourceType::ConstantBuffer:
            return entry.buffer != nullptr;

        case RHI::ShaderResourceType::Texture:
        case RHI::ShaderResourceType::StructuredBuffer:
        case RHI::ShaderResourceType::ByteAddressBuffer:
        case RHI::ShaderResourceType::RWTexture:
        case RHI::ShaderResourceType::RWStructuredBuffer:
        case RHI::ShaderResourceType::RWByteAddressBuffer:
            return entry.textureView != nullptr;

        case RHI::ShaderResourceType::Sampler:
            return entry.sampler != nullptr;

        default:
            return true;
        }
    }
}