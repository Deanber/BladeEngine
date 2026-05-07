#pragma once
#include <memory>

class ICommandBuffer;

class ICommandPool
{
public:
    virtual ~ICommandPool() = default;

    // 分配
    virtual std::shared_ptr<ICommandBuffer> Allocate() = 0;

    // 重置整个池（高性能关键）
    virtual void Reset() = 0;
};