#include "DX11BindGroup.h"
#include "RHIEnum.h"
#include <cassert>
#include <algorithm>
#include "DX11Buffer.h"
#include "DX11TextureView.h"
#include "DX11Sampler.h"

namespace DX11 {

    DX11BindGroup::DX11BindGroup(RHI::IBindGroupLayout* layout, const std::vector<RHI::BindGroupEntry>& entries)
        : m_layout(layout)
    {
        assert(layout != nullptr);
        PrecomputeBindings(entries);
    }

    void DX11BindGroup::PrecomputeBindings(const std::vector<RHI::BindGroupEntry>& entries)
    {
        // 排序，保证 batching 正确
        std::vector<RHI::BindGroupEntry> sorted = entries;
        std::sort(sorted.begin(), sorted.end(),
            [](const RHI::BindGroupEntry& a, const RHI::BindGroupEntry& b)
            {
                return a.binding < b.binding;
            });

        for (const auto& entry : sorted)
        {
            // TODO: 后续可以从 layout 获取
            RHI::ShaderStage visibility = RHI::ShaderStage::All;
            SortEntryToStages(entry, visibility);
        }
    }

    void DX11BindGroup::SortEntryToStages(const RHI::BindGroupEntry& entry, RHI::ShaderStage visibility)
    {
        auto AddToBatch = [](auto& batches, UINT slot, auto* ptr)
            {
                if (!ptr) return;

                if (batches.empty() ||
                    batches.back().startSlot + batches.back().views.size() != slot)
                {
                    batches.push_back({});
                    batches.back().startSlot = slot;
                }

                batches.back().views.push_back(ptr);
            };

        switch (entry.type)
        {
        case RHI::ShaderResourceType::ConstantBuffer:
            if (entry.buffer)
            {
                auto* buf = (ID3D11Buffer*)entry.buffer->GetNativeHandle();

                if ((visibility & RHI::ShaderStage::Vertex) != RHI::ShaderStage::None)
                    AddToBatch(m_vsResources.constantBuffers, entry.binding, buf);

                if ((visibility & RHI::ShaderStage::Fragment) != RHI::ShaderStage::None)
                    AddToBatch(m_psResources.constantBuffers, entry.binding, buf);

                if ((visibility & RHI::ShaderStage::Compute) != RHI::ShaderStage::None)
                    AddToBatch(m_csResources.constantBuffers, entry.binding, buf);
            }
            break;

        case RHI::ShaderResourceType::Texture:
        case RHI::ShaderResourceType::StructuredBuffer:
        case RHI::ShaderResourceType::ByteAddressBuffer:
            if (entry.textureView)
            {
                auto* srv = (ID3D11ShaderResourceView*)entry.textureView->GetSRV();

                if ((visibility & RHI::ShaderStage::Vertex) != RHI::ShaderStage::None)
                    AddToBatch(m_vsResources.srvs, entry.binding, srv);

                if ((visibility & RHI::ShaderStage::Fragment) != RHI::ShaderStage::None)
                    AddToBatch(m_psResources.srvs, entry.binding, srv);

                if ((visibility & RHI::ShaderStage::Compute) != RHI::ShaderStage::None)
                    AddToBatch(m_csResources.srvs, entry.binding, srv);
            }
            break;

        case RHI::ShaderResourceType::RWTexture:
        case RHI::ShaderResourceType::RWStructuredBuffer:
        case RHI::ShaderResourceType::RWByteAddressBuffer:
            if (entry.textureView)
            {
                auto* uav = (ID3D11UnorderedAccessView*)entry.textureView->GetUAV();

                // PS UAV 不在 BindGroup Apply 阶段绑定（由 OMSet 控制）
                if ((visibility & RHI::ShaderStage::Fragment) != RHI::ShaderStage::None)
                {
                    m_psResources.uavs.push_back({ entry.binding, {uav} });
                }

                if ((visibility & RHI::ShaderStage::Compute) != RHI::ShaderStage::None)
                {
                    AddToBatch(m_csResources.uavs, entry.binding, uav);
                }
            }
            break;

        case RHI::ShaderResourceType::Sampler:
            if (entry.sampler)
            {
                auto* sampler = (ID3D11SamplerState*)entry.sampler->GetNativeHandle();

                if ((visibility & RHI::ShaderStage::Vertex) != RHI::ShaderStage::None)
                    AddToBatch(m_vsResources.samplers, entry.binding, sampler);

                if ((visibility & RHI::ShaderStage::Fragment) != RHI::ShaderStage::None)
                    AddToBatch(m_psResources.samplers, entry.binding, sampler);

                if ((visibility & RHI::ShaderStage::Compute) != RHI::ShaderStage::None)
                    AddToBatch(m_csResources.samplers, entry.binding, sampler);
            }
            break;

        case RHI::ShaderResourceType::PushConstant:
        case RHI::ShaderResourceType::AccelerationStructure:
        case RHI::ShaderResourceType::InputAttachment:
            break;

        default:
            assert(false && "Unsupported ShaderResourceType in DX11BindGroup");
            break;
        }
    }

    void DX11BindGroup::Apply(ID3D11DeviceContext* context)
    {
        // VS
        for (const auto& batch : m_vsResources.constantBuffers)
            context->VSSetConstantBuffers(batch.startSlot, (UINT)batch.views.size(), batch.views.data());
        for (const auto& batch : m_vsResources.srvs)
            context->VSSetShaderResources(batch.startSlot, (UINT)batch.views.size(), batch.views.data());
        for (const auto& batch : m_vsResources.samplers)
            context->VSSetSamplers(batch.startSlot, (UINT)batch.views.size(), batch.views.data());

        // PS
        for (const auto& batch : m_psResources.constantBuffers)
            context->PSSetConstantBuffers(batch.startSlot, (UINT)batch.views.size(), batch.views.data());
        for (const auto& batch : m_psResources.srvs)
            context->PSSetShaderResources(batch.startSlot, (UINT)batch.views.size(), batch.views.data());
        for (const auto& batch : m_psResources.samplers)
            context->PSSetSamplers(batch.startSlot, (UINT)batch.views.size(), batch.views.data());

        // PS UAV 不在这里绑定

        // CS
        for (const auto& batch : m_csResources.constantBuffers)
            context->CSSetConstantBuffers(batch.startSlot, (UINT)batch.views.size(), batch.views.data());
        for (const auto& batch : m_csResources.srvs)
            context->CSSetShaderResources(batch.startSlot, (UINT)batch.views.size(), batch.views.data());
        for (const auto& batch : m_csResources.samplers)
            context->CSSetSamplers(batch.startSlot, (UINT)batch.views.size(), batch.views.data());
        for (const auto& batch : m_csResources.uavs)
            context->CSSetUnorderedAccessViews(batch.startSlot, (UINT)batch.views.size(), batch.views.data(), nullptr);
    }
}