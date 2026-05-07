#pragma once
#include <vector>
#include <cstdint>
#include <unordered_map>
#include "IPipelineLayout.h"
#include "DX11BindGroupLayout.h"

namespace DX11
{
    struct SlotAllocator
    {
        uint32_t nextCBV = 0;
        uint32_t nextSRV = 0;
        uint32_t nextUAV = 0;
        uint32_t nextSampler = 0;
    };

    struct SetBase
    {
        uint32_t cbvBase = 0;
        uint32_t srvBase = 0;
        uint32_t uavBase = 0;
        uint32_t samplerBase = 0;

        //记录范围（用于校验）
        uint32_t cbvCount = 0;
        uint32_t srvCount = 0;
        uint32_t uavCount = 0;
        uint32_t samplerCount = 0;
    };

    struct SetMapping
    {
        std::unordered_map<uint32_t, uint32_t> cbvSlots;
        std::unordered_map<uint32_t, uint32_t> srvSlots;
        std::unordered_map<uint32_t, uint32_t> uavSlots;
        std::unordered_map<uint32_t, uint32_t> samplerSlots;
    };

    class DX11PipelineLayout final : public RHI::IPipelineLayout
    {
    public:
        explicit DX11PipelineLayout(const RHI::PipelineLayoutDesc& desc);
        ~DX11PipelineLayout() override = default;

        std::vector<SetBase> m_setBases;

        const RHI::PipelineLayoutDesc& GetDesc() const override { return m_desc; }

        // 查询 slot
        uint32_t GetSlot(uint32_t set, uint32_t binding, RHI::ShaderResourceType type) const;

        RHI::ResourceType GetType() const override { return RHI::ResourceType::PipelineLayout; }

        // Layout 是无状态的逻辑对象
        RHI::ResourceAccessState GetState() const override { return RHI::ResourceAccessState::Undefined; }
        void SetState(RHI::ResourceAccessState state) override { /* 无需实现 */ }

        // 它不对应原生 DX11 对象，返回 nullptr 即可
        void* GetNativeHandle() const override { return nullptr; }

        uint32_t GetSubresourceCount() const override { return 1; }


    private:
        void BuildMapping();

    private:
        RHI::PipelineLayoutDesc m_desc;

        // set index → mapping
        std::vector<SetMapping> m_setMappings;
    };
}