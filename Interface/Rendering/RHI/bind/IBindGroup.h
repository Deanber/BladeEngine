#pragma once
#include "IResource.h"
#include <vector>
#include <memory>
#include <variant>
#include <cassert>
#include <string>
#include <type_traits>

namespace RHI {

    struct BufferBinding {
        std::shared_ptr<IBuffer> buffer;
        uint64_t offset = 0;
        uint64_t size = 0;
    };
    struct BufferViewBinding
    {
        std::shared_ptr<IBufferView> view;
    };

    struct TextureBinding {
        std::shared_ptr<ITextureView> view;
    };

    struct SamplerBinding {
        std::shared_ptr<ISampler> sampler;
    };

    struct CombinedImageSamplerBinding
    {
        std::shared_ptr<ITextureView> view;
        std::shared_ptr<ISampler> sampler;
    };

    struct AccelerationStructureBinding
    {
        std::shared_ptr<IResource> accelerationStructure;
    };

    struct InputAttachmentBinding
    {
        std::shared_ptr<ITextureView> view;
    };
    struct EmptyBinding {};
    using BindingResource = std::variant<
        EmptyBinding,
        BufferBinding,
        BufferViewBinding,
        TextureBinding,
        SamplerBinding,
        CombinedImageSamplerBinding,
        AccelerationStructureBinding,
        InputAttachmentBinding
    >;

    inline bool Match(DescriptorType type, const BindingResource& res)
    {
        switch (type)
        {
        case DescriptorType::UniformBuffer:
        case DescriptorType::StorageBuffer:
            return std::holds_alternative<BufferBinding>(res);

        case DescriptorType::StructuredBuffer:
        case DescriptorType::RWStructuredBuffer:
        case DescriptorType::ByteAddressBuffer:
        case DescriptorType::RWByteAddressBuffer:
            return std::holds_alternative<BufferViewBinding>(res);

        case DescriptorType::SampledTexture:
        case DescriptorType::StorageTexture:
            return std::holds_alternative<TextureBinding>(res);

        case DescriptorType::Sampler:
            return std::holds_alternative<SamplerBinding>(res);

        case DescriptorType::CombinedImageSampler:
            return std::holds_alternative<CombinedImageSamplerBinding>(res);

        case DescriptorType::AccelerationStructure:
            return std::holds_alternative<AccelerationStructureBinding>(res);

        case DescriptorType::InputAttachment:
            return std::holds_alternative<InputAttachmentBinding>(res);

        default:
            return false;
        }
    }

    struct BindGroupLayoutBinding
    {
        uint32_t binding = 0;
        uint32_t set = 0;

        DescriptorType type;

        uint32_t count = 1;

        ShaderStage visibility = ShaderStage::All;

        const char* name = nullptr;

        // ===== 工业增强 =====
        bool dynamicOffset = false;
        bool bindless = false;
    };

    class IBindGroupLayout : public IResource {
    public:
        virtual ~IBindGroupLayout() = default;

        virtual const std::vector<BindGroupLayoutBinding>& GetBindings() const = 0;
        void* GetNativeHandle() const override { return nullptr; }
    };

    template<typename T, size_t N = 4>
    class SmallVector
    {
    public:
        SmallVector() : m_size(0) {}
        SmallVector(const SmallVector& other) {
            m_size = other.m_size;
            for (size_t i = 0; i < m_size && i < N; ++i)
                m_stack[i] = other.m_stack[i];
            m_heap = other.m_heap;
        }

        void push_back(const T& v)
        {
            if (m_heap.empty() && m_size < N)
            {
                m_stack[m_size++] = v;
            }
            else
            {
                if (m_heap.empty())
                {
                    m_heap.reserve(N + 4);
                    for (size_t i = 0; i < m_size; ++i)
                        m_heap.push_back(std::move(m_stack[i]));
                }

                m_heap.push_back(v);
            }
        }

        const T& operator[](size_t i) const
        {
            return (i < N) ? m_stack[i] : m_heap[i - N];
        }

        size_t size() const
        {
            return m_heap.empty() ? m_size : m_heap.size();
        }

        SmallVector(const SmallVector&) = default;
        SmallVector& operator=(const SmallVector&) = default;

        SmallVector(SmallVector&& other) noexcept
        {
            m_size = other.m_size;
            for (size_t i = 0; i < m_size && i < N; ++i)
                m_stack[i] = std::move(other.m_stack[i]);

            m_heap = std::move(other.m_heap);
        }

        SmallVector& operator=(SmallVector&& other) noexcept
        {
            if (this == &other) return *this;

            m_size = other.m_size;

            for (size_t i = 0; i < m_size && i < N; ++i)
                m_stack[i] = std::move(other.m_stack[i]);

            m_heap = std::move(other.m_heap);

            return *this;
        }

        const T* data() const
        {
            return m_heap.empty() ? m_stack : m_heap.data();
        }

        T* data()
        {
            return (m_size <= N) ? m_stack : m_heap.data();
        }

        auto begin() { return data(); }
        auto end() { return data() + size(); }

        auto begin() const { return data(); }
        auto end()   const { return data() + size(); }

    private:
        T m_stack[N];
        size_t m_size = 0;
        std::vector<T> m_heap;
    };

    struct BindingData
    {
        SmallVector<BindingResource, 4> resources;
    };

    struct BindGroupEntry
    {
        uint32_t binding = 0;

        BindingData data;
    };

    class BindGroupBuilder
    {
    public:
        BindGroupBuilder& Add(uint32_t binding, BindingResource res) {
            auto& entry = m_entries.emplace_back();
            entry.binding = binding;
            entry.data.resources.push_back(std::move(res));
            return *this;
        }

        BindGroupBuilder& AddArray(uint32_t binding, std::vector<BindingResource> res)
        {
            auto& e = m_entries.emplace_back();
            e.binding = binding;
            for (auto& r : res)
            {
                e.data.resources.push_back(std::move(r));
            }
            return *this;
        }

        BindGroupBuilder& AddArray(uint32_t binding, std::initializer_list<BindingResource> list)
        {
            auto& e = m_entries.emplace_back();
            e.binding = binding;
            for (auto& r : list)
            {
                e.data.resources.push_back(r);
            }
            return *this;
        }

        std::vector<BindGroupEntry> Build()
        {
            return std::move(m_entries);
        }

    private:
        std::vector<BindGroupEntry> m_entries;
    };

    inline size_t HashCombine(size_t a, size_t b)
    {
        return a ^ (b + 0x9e3779b97f4a7c15ull + (a << 6) + (a >> 2));
    }

    inline size_t HashResource(const BindingResource& res)
    {
        return std::visit([](auto&& r) -> size_t
            {
                using T = std::decay_t<decltype(r)>;

                if constexpr (std::is_same_v<T, BufferBinding>)
                {
                    return HashCombine(
                        r.buffer->GetId(),
                        r.offset ^ r.size
                    );
                }
                else if constexpr (std::is_same_v<T, TextureBinding>)
                {
                    return r.view->GetId();
                }
                else if constexpr (std::is_same_v<T, SamplerBinding>)
                {
                    return r.sampler->GetId();
                }
                else if constexpr (std::is_same_v<T, BufferViewBinding>)
                {
                    return r.view->GetId();
                }
                else if constexpr (std::is_same_v<T, CombinedImageSamplerBinding>)
                {
                    return HashCombine(
                        r.view->GetId(),
                        r.sampler->GetId()
                    );
                }
                else if constexpr (std::is_same_v<T, AccelerationStructureBinding>)
                {
                    return r.accelerationStructure->GetId();
                }
                else if constexpr (std::is_same_v<T, InputAttachmentBinding>)
                {
                    return r.view->GetId();
                }
                else
                {
                    assert(false && "Unhandled BindingResource type");
                    return 0;
                }
            }, res);
    }

    

    class IBindGroup : public IResource {
    public:
        virtual ~IBindGroup() = default;

        virtual IBindGroupLayout* GetLayout()  const = 0;
        virtual const std::vector<BindGroupEntry>& GetEntries() const = 0;
        void* GetNativeHandle() const override { return nullptr; }
    };

    struct BindGroupDesc
    {
        std::shared_ptr<IBindGroupLayout> layout;
        std::vector<BindGroupEntry> entries;
    };

    inline size_t HashBindGroupDesc(const BindGroupDesc& desc)
    {
        size_t h = desc.layout->GetId();

        for (auto& e : desc.entries)
        {
            size_t eh = e.binding;

            for (auto& r : e.data.resources)
                eh = HashCombine(eh, HashResource(r));

            h = HashCombine(h, eh);
        }

        return h;
    }

    struct ValidationResult
    {
        bool ok = true;
        const char* message = nullptr;
    };

    ValidationResult ValidateEntry(
        const BindGroupLayoutBinding& layout,
        const BindGroupEntry& entry)
    {
        if (layout.binding != entry.binding)
            return { false, "Binding mismatch" };

        const auto& res = entry.data.resources;

        if (res.size() == 0)
            return { false, "Empty binding" };

        if (!layout.bindless && res.size() != layout.count)
            return { false, "Count mismatch" };

        for (auto& r : res)
        {
            if (!Match(layout.type, r))
                return { false, "Type mismatch" };
        }

        return { true, nullptr };
    }

    inline bool ValidateGroup(
        const IBindGroupLayout& layout,
        const std::vector<BindGroupEntry>& entries,
        std::string& error)
    {
        auto& bindings = layout.GetBindings();

        for (auto& e : entries)
        {
            auto it = std::find_if(bindings.begin(), bindings.end(),
                [&](auto& b) { return b.binding == e.binding; });

            if (it == bindings.end())
            {
                error = "Binding not found in layout";
                return false;
            }

            auto r = ValidateEntry(*it, e);
            if (!r.ok)
            {
                error = r.message;
                return false;
            }
        }

        return true;
    }

}