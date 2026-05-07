#pragma once

#include <vector>
#include <cstdint>
#include "RHIEnum.h"

namespace RHI
{
    enum class InputRate : uint8_t
    {
        Vertex = 0,
        Instance = 1
    };

    struct VertexAttribute
    {
        uint32_t location = 0;
        VertexFormat format;
        uint32_t offset = 0;
        uint32_t bufferSlot = 0;
    };

    struct VertexBufferLayout
    {
        uint32_t slot = 0;
        uint32_t stride = 0;
        InputRate rate = InputRate::Vertex;
        uint32_t stepRate = 0;
    };

    // Vertex Input State
    struct VertexInputState
    {
        std::vector<VertexAttribute> attributes;
        std::vector<VertexBufferLayout> layouts;

        bool operator==(const VertexInputState& o) const
        {
            if (attributes.size() != o.attributes.size()) return false;
            if (layouts.size() != o.layouts.size()) return false;

            for (size_t i = 0; i < attributes.size(); i++)
            {
                const auto& a = attributes[i];
                const auto& b = o.attributes[i];

                if (a.location != b.location ||
                    a.format != b.format ||
                    a.offset != b.offset ||
                    a.bufferSlot != b.bufferSlot)
                    return false;
            }

            for (size_t i = 0; i < layouts.size(); i++)
            {
                const auto& a = layouts[i];
                const auto& b = o.layouts[i];

                if (a.slot != b.slot ||
                    a.stride != b.stride ||
                    a.rate != b.rate ||
                    a.stepRate != b.stepRate)
                    return false;
            }

            return true;
        }
    };

    // helper（只在 backend 用）
    inline const char* MapLocationToSemantic(uint32_t loc)
    {
        // 重新排列，让常用的 0=POS, 1=COLOR, 2=TEXCOORD
        switch (loc) {
        case 0: return "POSITION";
        case 1: return "COLOR";
        case 2: return "TEXCOORD";
        case 3: return "NORMAL";
        case 4: return "TANGENT";
        default: return "TEXCOORD";
        }
    }
}