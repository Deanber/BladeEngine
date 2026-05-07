#pragma once
#include <vector>
#include "IResource.h"

namespace RHI {
    class IBindGroupLayout;
    class PushConstantRange;

    struct PipelineLayoutDesc
    {
        std::vector<IBindGroupLayout*> bindGroupLayouts;

        //std::vector<PushConstantRange> pushConstants;
    };

    class IPipelineLayout : public IResource
    {
    public:
        virtual ~IPipelineLayout() = default;

        virtual const PipelineLayoutDesc& GetDesc() const = 0;
    };
}