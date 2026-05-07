#pragma once
#include <vector>
#include <memory>

class ICommandBuffer;
class IRenderer;

struct DrawCommand
{
    int vertexCount;
};

class Renderer
{
public:
    void Initialize(IRenderer* rhi)
    {
        renderer = rhi;
    }

    void Submit(const DrawCommand& cmd)
    {
        commands.push_back(cmd);
    }

    void Flush()
    {
        auto cmdBuffer = renderer->CreateCommandBuffer();

        cmdBuffer->Begin();

        for (auto& cmd : commands)
        {
            cmdBuffer->Draw(cmd.vertexCount);
        }

        cmdBuffer->End();

        commands.clear();
    }

private:
    IRenderer* renderer = nullptr;
    std::vector<DrawCommand> commands;
};