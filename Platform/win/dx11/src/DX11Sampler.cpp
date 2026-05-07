#include "DX11Sampler.h"
#include <cstring>
#include <stdexcept>

namespace DX11 {

    static D3D11_FILTER ConvertFilter(RHI::Filter filter)
    {
        switch (filter)
        {
        case RHI::Filter::MinMagMipPoint: return D3D11_FILTER_MIN_MAG_MIP_POINT;
        case RHI::Filter::MinMagPointMipLinear: return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        case RHI::Filter::MinPointMagLinearMipPoint: return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        case RHI::Filter::MinPointMagMipLinear: return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        case RHI::Filter::MinLinearMagMipPoint: return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        case RHI::Filter::MinLinearMagPointMipLinear: return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        case RHI::Filter::MinMagLinearMipPoint: return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        case RHI::Filter::MinMagMipLinear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        default: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        }
    }

    // Helper: RHI::SamplerAddressMode -> D3D11_TEXTURE_ADDRESS_MODE
    static D3D11_TEXTURE_ADDRESS_MODE ConvertAddress(RHI::SamplerAddressMode mode)
    {
        switch (mode)
        {
        case RHI::SamplerAddressMode::Wrap: return D3D11_TEXTURE_ADDRESS_WRAP;
        case RHI::SamplerAddressMode::Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
        case RHI::SamplerAddressMode::Clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
        case RHI::SamplerAddressMode::Border: return D3D11_TEXTURE_ADDRESS_BORDER;
        case RHI::SamplerAddressMode::MirrorOnce: return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
        default: return D3D11_TEXTURE_ADDRESS_WRAP;
        }
    }

    // Helper: RHI::ComparisonFunc -> D3D11_COMPARISON_FUNC
    static D3D11_COMPARISON_FUNC ConvertComparison(RHI::ComparisonFunc func)
    {
        switch (func)
        {
        case RHI::ComparisonFunc::Never: return D3D11_COMPARISON_NEVER;
        case RHI::ComparisonFunc::Less: return D3D11_COMPARISON_LESS;
        case RHI::ComparisonFunc::Equal: return D3D11_COMPARISON_EQUAL;
        case RHI::ComparisonFunc::LessEqual: return D3D11_COMPARISON_LESS_EQUAL;
        case RHI::ComparisonFunc::Greater: return D3D11_COMPARISON_GREATER;
        case RHI::ComparisonFunc::NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
        case RHI::ComparisonFunc::GreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
        case RHI::ComparisonFunc::Always: return D3D11_COMPARISON_ALWAYS;
        default: return D3D11_COMPARISON_ALWAYS;
        }
    }

    DX11Sampler::DX11Sampler(ID3D11Device* device, const RHI::SamplerDesc& desc)
        : m_desc(desc)
    {
        if (!device)
            throw std::runtime_error("DX11Sampler: device is null");

        D3D11_SAMPLER_DESC dxDesc = {};
        dxDesc.Filter = ConvertFilter(desc.filter);
        dxDesc.AddressU = ConvertAddress(desc.addressU);
        dxDesc.AddressV = ConvertAddress(desc.addressV);
        dxDesc.AddressW = ConvertAddress(desc.addressW);
        dxDesc.MipLODBias = desc.mipLODBias;
        dxDesc.MaxAnisotropy = desc.maxAnisotropy;
        dxDesc.ComparisonFunc = ConvertComparison(desc.comparisonFunc);
        dxDesc.MinLOD = desc.minLOD;
        dxDesc.MaxLOD = desc.maxLOD;

        float border[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        switch (desc.borderColor)
        {
        case RHI::BorderColor::FloatTransparentBlack: border[0] = border[1] = border[2] = border[3] = 0.0f; break;
        case RHI::BorderColor::FloatOpaqueBlack: border[0] = border[1] = border[2] = 0.0f; border[3] = 1.0f; break;
        case RHI::BorderColor::FloatOpaqueWhite: border[0] = border[1] = border[2] = border[3] = 1.0f; break;
        }
        memcpy(dxDesc.BorderColor, border, sizeof(float) * 4);

        HRESULT hr = device->CreateSamplerState(&dxDesc, &m_sampler);
        if (FAILED(hr))
        {
            m_sampler = nullptr;
            throw std::runtime_error("DX11Sampler: Failed to create sampler state");
        }
    }

    DX11Sampler::~DX11Sampler()
    {
        if (m_sampler)
        {
            m_sampler->Release();
            m_sampler = nullptr;
        }
    }

    const RHI::SamplerDesc& DX11Sampler::GetDesc() const
    {
        return m_desc;
    }

    ID3D11SamplerState* DX11Sampler::GetDXSampler() const
    {
        return m_sampler;
    }
}