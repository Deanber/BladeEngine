#pragma once
#include "IBindGroup.h"
#include <d3d11.h>
#include <wrl/client.h>

namespace DX11 {

    using Microsoft::WRL::ComPtr;

    template<typename TNativeView>
    struct StageBindingBatch {
        UINT startSlot = 0;
        std::vector<TNativeView*> views;
    };

    struct ShaderStageResources {
        std::vector<StageBindingBatch<ID3D11Buffer>>              constantBuffers;
        std::vector<StageBindingBatch<ID3D11ShaderResourceView>>  srvs;
        std::vector<StageBindingBatch<ID3D11SamplerState>>        samplers;
        std::vector<StageBindingBatch<ID3D11UnorderedAccessView>> uavs;
    };

    class DX11BindGroup final : public RHI::IBindGroup {
    public:
        DX11BindGroup(RHI::IBindGroupLayout* layout, const std::vector<RHI::BindGroupEntry>& entries);
        ~DX11BindGroup() override = default;

        RHI::IBindGroupLayout* GetLayout() const override { return m_layout; }

        void Apply(ID3D11DeviceContext* context);
        const auto& GetPSUAVs() const
        {
            return m_psResources.uavs;
        }

    private:
        void PrecomputeBindings(const std::vector<RHI::BindGroupEntry>& entries);

        void SortEntryToStages(const RHI::BindGroupEntry& entry, RHI::ShaderStage visibility);

    private:
        RHI::IBindGroupLayout* m_layout = nullptr;
        ShaderStageResources m_vsResources;
        ShaderStageResources m_psResources;
        ShaderStageResources m_csResources;
    };
}