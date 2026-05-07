#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <unordered_map>
#include <vector>
#include <mutex>

#include "VertexInputState.h"

namespace DX11
{
    using Microsoft::WRL::ComPtr;

    struct InputLayoutKey
    {
        uint64_t vsBytecodeHash = 0;
        uint64_t vertexInputHash = 0;

        bool operator==(const InputLayoutKey& other) const
        {
            return vsBytecodeHash == other.vsBytecodeHash
                && vertexInputHash == other.vertexInputHash;
        }
    };

    struct InputLayoutKeyHash
    {
        uint64_t operator()(const InputLayoutKey& k) const noexcept
        {
            uint64_t h = 1469598103934665603ull;

            auto mix = [&](uint64_t v)
                {
                    h ^= v;
                    h *= 1099511628211ull;
                };

            mix(k.vsBytecodeHash);
            mix(k.vertexInputHash);

            return h;
        }
    };

    class DX11InputLayoutCache
    {
    public:
        explicit DX11InputLayoutCache(ID3D11Device* device) : m_device(device) {}
        ID3D11InputLayout* GetOrCreate(
            const void* vsBytecode,
            uint64_t bytecodeSize,
            const RHI::VertexInputState& inputState);

    private:
        ID3D11Device* m_device = nullptr;
        std::unordered_map<InputLayoutKey, ComPtr<ID3D11InputLayout>, InputLayoutKeyHash> m_cache;

        std::mutex m_mutex;

    private:
        uint64_t HashBytecode(const void* data, size_t size);
        uint64_t HashVertexInput(const RHI::VertexInputState& state);
    };
}