#pragma once
#include "RHIEnum.h"
namespace RHI 
{
    class IDeviceObject {
    public:
        virtual ~IDeviceObject() = default;
        virtual void* GetNativeHandle() const = 0;
        virtual uint64_t GetId() const = 0;
    protected:
        uint64_t m_id = 0;
    };

    class IResource : public IDeviceObject
    {
    public:
        virtual ~IResource() = default;
        virtual ResourceType GetType() const = 0;
        virtual ResourceAccessState GetState() const = 0;
        virtual void SetState(ResourceAccessState state) = 0;
        virtual uint32_t GetSubresourceCount() const { return 1; }
    };
}