#pragma once
#include <cstddef>
#include <cstdint>
#include "RHIEnum.h"
#include "IResource.h"
namespace RHI {
    struct BufferDesc
    {
        size_t size = 0;
        BufferType type = BufferType::Vertex;
        MemoryType memory = MemoryType::GPUOnly;
        uint32_t bindFlags = Bind_None;
        size_t stride = 0;
    };

    struct BufferViewDesc
    {
        size_t offset = 0;
        size_t size = 0;
        enum class ViewType { Default, Structured, Raw } type = ViewType::Default;
        size_t stride = 0;
    };

    class IBuffer : public IResource
    {
    public:
        virtual ~IBuffer() = default;
        virtual const BufferDesc& GetDesc() const = 0;

        virtual void Update(const void* data, size_t size, size_t offset = 0) = 0;

        virtual void* Map() = 0;
        virtual void Unmap() = 0;
    };

    class IBufferView : public IDeviceObject
    {
    public:
        virtual ~IBufferView() = default;
        virtual IBuffer* GetBuffer() const = 0;
        virtual const BufferViewDesc& GetDesc() const = 0;

        virtual void* GetSRV() const = 0;
        virtual void* GetUAV() const = 0;
    };
}

