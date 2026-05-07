#pragma once
#include <cstdint>

namespace RHI {
    class IFence {
    public:
        virtual ~IFence() = default;

        /**
         * @brief 获取当前 GPU 已经完成到的数值
         */
        virtual uint64_t GetCompletedValue() = 0;

        /**
         * @brief 在 CPU 端阻塞，直到 Fence 的值达到或超过预期值
         * @param value 目标数值
         * @param timeoutMs 超时时间（毫秒），UINT64_MAX 表示无限等待
         */
        virtual void Wait(uint64_t value, uint64_t timeoutMs = 0xFFFFFFFFFFFFFFFF) = 0;

        /**
         * @brief 手动在 CPU 端更新 Fence 的值（通常用于外部同步）
         */
        virtual void Signal(uint64_t value) = 0;
    };
}