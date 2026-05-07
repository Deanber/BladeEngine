#pragma once
#include "ISampler.h"
#include <d3d11.h>

namespace DX11
{
    class DX11Sampler : public RHI::ISampler
    {
    public:
        DX11Sampler(ID3D11Device* device, const RHI::SamplerDesc& desc);
        virtual ~DX11Sampler();

        virtual const RHI::SamplerDesc& GetDesc() const override;
        ID3D11SamplerState* GetDXSampler() const;

    private:
        RHI::SamplerDesc m_desc;
        ID3D11SamplerState* m_sampler = nullptr;
    };
}