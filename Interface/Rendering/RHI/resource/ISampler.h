#pragma once
#include "RHIEnum.h"
#include "IResource.h"

namespace RHI {
    inline SamplerFlags operator|(SamplerFlags a, SamplerFlags b)
    {
        return (SamplerFlags)((uint32_t)a | (uint32_t)b);
    }

    struct SamplerDesc
    {
        Filter filter = Filter::MinMagMipLinear;

        SamplerAddressMode addressU = SamplerAddressMode::Wrap;
        SamplerAddressMode addressV = SamplerAddressMode::Wrap;
        SamplerAddressMode addressW = SamplerAddressMode::Wrap;

        float mipLODBias = 0.0f;

        uint32_t maxAnisotropy = 1;

        ComparisonFunc comparisonFunc = ComparisonFunc::Always;

        float minLOD = 0.0f;
        float maxLOD = 32.0f;

        BorderColor borderColor = BorderColor::FloatTransparentBlack;

        SamplerReductionMode reduction = SamplerReductionMode::Standard;

        SamplerFlags flags = SamplerFlags::None;
    };

    class ISampler : public IResource
    {
    public:
        virtual ~ISampler() = default;

        virtual const SamplerDesc& GetDesc() const = 0;
    };

    struct StaticSamplerDesc : public SamplerDesc
    {
        uint32_t shaderRegister = 0;
        uint32_t registerSpace = 0;
    };
}