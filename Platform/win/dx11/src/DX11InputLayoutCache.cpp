#include "DX11InputLayoutCache.h"
#include <cassert>
#include <mutex>
#include "RHIEnum.h"

using namespace RHI;

namespace DX11 {
    // =========================
    // FNV1a 64-bit hash
    // =========================
    static uint64_t HashBytes(const void* data, size_t size)
    {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);

        uint64_t hash = 1469598103934665603ull;
        for (size_t i = 0; i < size; i++)
        {
            hash ^= bytes[i];
            hash *= 1099511628211ull;
        }
        return hash;
    }

    // =========================
    // VertexInput Hash
    // =========================
    uint64_t DX11InputLayoutCache::HashVertexInput(const VertexInputState& state)
    {
        uint64_t h = 0;

        auto combine = [&](uint64_t v)
            {
                h ^= v + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2);
            };

        combine(state.attributes.size());
        combine(state.layouts.size());

        for (const auto& a : state.attributes)
        {
            combine(a.location);
            combine((uint64_t)a.format);
            combine(a.offset);
            combine(a.bufferSlot);
        }

        for (const auto& b : state.layouts)
        {
            combine(b.slot);
            combine(b.stride);
            combine((uint64_t)b.rate);
            combine(b.stepRate);
        }

        return h;
    }


    // =========================
    // GetOrCreate InputLayout
    // =========================
    ID3D11InputLayout* DX11InputLayoutCache::GetOrCreate(
        const void* vsBytecode,
        uint64_t bytecodeSize,
        const VertexInputState& inputState)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        InputLayoutKey key;
        key.vsBytecodeHash = HashBytes(vsBytecode, bytecodeSize);
        key.vertexInputHash = HashVertexInput(inputState);

        auto it = m_cache.find(key);
        if (it != m_cache.end())
            return it->second.Get();

        std::vector<const char*> semanticNames;
        semanticNames.reserve(inputState.attributes.size());

        for (const auto& attr : inputState.attributes)
        {
            semanticNames.push_back(MapLocationToSemantic(attr.location));
        }

        std::vector<D3D11_INPUT_ELEMENT_DESC> descs;
        descs.reserve(inputState.attributes.size());

        auto ToDXGI = [](RHI::VertexFormat fmt)
            {
                switch (fmt)
                {
                case VertexFormat::Float:      return DXGI_FORMAT_R32_FLOAT;
                case VertexFormat::Float2:     return DXGI_FORMAT_R32G32_FLOAT;
                case VertexFormat::Float3:     return DXGI_FORMAT_R32G32B32_FLOAT;
                case VertexFormat::Float4:     return DXGI_FORMAT_R32G32B32A32_FLOAT;

                case VertexFormat::UInt:       return DXGI_FORMAT_R32_UINT;
                case VertexFormat::Int:        return DXGI_FORMAT_R32_SINT;

                case VertexFormat::UByte4:     return DXGI_FORMAT_R8G8B8A8_UINT;
                case VertexFormat::UByte4Norm: return DXGI_FORMAT_R8G8B8A8_UNORM;

                default:
                    assert(false);
                    return DXGI_FORMAT_R32_FLOAT;
                }
            };

        for (size_t i = 0; i < inputState.attributes.size(); ++i)
        {
            const auto& attr = inputState.attributes[i];

            D3D11_INPUT_ELEMENT_DESC d{};
            d.SemanticName = semanticNames[i];
            d.SemanticIndex = 0;
            d.Format = ToDXGI(attr.format);
            d.InputSlot = attr.bufferSlot;
            d.AlignedByteOffset = attr.offset;

            // layout safety
            if (attr.bufferSlot < inputState.layouts.size())
            {
                const auto& layout = inputState.layouts[attr.bufferSlot];

                d.InputSlotClass =
                    (layout.rate == InputRate::Vertex)
                    ? D3D11_INPUT_PER_VERTEX_DATA
                    : D3D11_INPUT_PER_INSTANCE_DATA;

                d.InstanceDataStepRate = layout.stepRate;
            }
            else
            {
                d.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                d.InstanceDataStepRate = 0;
            }

            descs.push_back(d);
        }

        // =========================
        // Create InputLayout
        // =========================
        ComPtr<ID3D11InputLayout> layout;

        HRESULT hr = m_device->CreateInputLayout(
            descs.data(),
            (UINT)descs.size(),
            vsBytecode,
            (SIZE_T)bytecodeSize,
            layout.GetAddressOf()
        );
        if (FAILED(hr)) {
            char buf[256];
            sprintf_s(buf, "CreateInputLayout Failed! HRESULT: 0x%08X\n", hr);
            OutputDebugStringA(buf);

            if (hr == E_INVALIDARG) {
                OutputDebugStringA("Error: Invalid Argument. Check if SemanticName is correct or bytecode is null.\n");
            }
            // 这里的断点非常关键
            __debugbreak();
        }
        assert(SUCCEEDED(hr));

        m_cache.emplace(key, layout);

        return layout.Get();
    }
}