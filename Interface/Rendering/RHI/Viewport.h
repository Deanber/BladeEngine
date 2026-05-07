#pragma once
#include <cstdint>

namespace RHI {

    // 视口定义
    struct Viewport {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        float minDepth = 0.0f;
        float maxDepth = 1.0f;
    };

    // 裁剪矩形定义 (Scissor Rect)
    struct Rect {
        int32_t left = 0;
        int32_t top = 0;
        int32_t right = 0;
        int32_t bottom = 0;
    };

    class IViewport {
    public:
        virtual ~IViewport() = default;
        virtual const Viewport& GetDesc() const = 0;
    };

    class IScissorRect {
    public:
        virtual ~IScissorRect() = default;
        virtual const Rect& GetDesc() const = 0;
    };
}