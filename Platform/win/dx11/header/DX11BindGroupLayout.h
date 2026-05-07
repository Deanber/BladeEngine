#pragma once
#include <d3d11.h>
#include <vector>
#include <map>
#include "IBindGroup.h"
#include <unordered_map>

namespace DX11 {
    struct StageLayoutInfo {
        UINT minSlot = 0;
        UINT maxSlot = 0;
        UINT count = 0;
        bool exists = false;
    };

    class DX11BindGroupLayout final : public RHI::IBindGroupLayout {
    public:
        explicit DX11BindGroupLayout(const std::vector<RHI::BindGroupLayoutBinding>& bindings);
        ~DX11BindGroupLayout() override = default;

        const std::vector<RHI::BindGroupLayoutBinding>& GetBindings() const override { return m_bindings; }

        const StageLayoutInfo& GetStageInfo(RHI::ShaderStage stage, RHI::ShaderResourceType type) const;

        bool ValidateBinding(const RHI::BindGroupEntry& entry) const;
        std::unordered_map<uint32_t, RHI::BindGroupLayoutBinding> m_bindingMap;

    private:
        void AnalyzeLayout();

    private:
        std::vector<RHI::BindGroupLayoutBinding> m_bindings;

        std::map<RHI::ShaderStage, std::map<RHI::ShaderResourceType, StageLayoutInfo>> m_infoCache;
    };
}