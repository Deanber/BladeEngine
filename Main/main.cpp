#include <windows.h>
#include <memory>
#include <vector>
#include <fstream>
#include <cassert>

#include "IBuffer.h"
#include "IRenderer.h"
#include "IShader.h"
#include "DX11Shader.h"
#include "PipelineStateDesc.h"
#include "ICommandList.h"
#include "DX11Renderer.h"
#include "filesystem"
#define _ITERATOR_DEBUG_LEVEL 2

using namespace RHI;

struct Vertex
{
    float pos[3];
    float uv[2]; // 使用 UV 坐标作为复数平面的映射
};

std::vector<char> LoadFile(const char* path)
{
    std::ifstream file(path, std::ios::binary);
    return std::vector<char>((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
    const wchar_t* CLASS_NAME = L"RHIWindow";

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;

    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"RHI Triangle", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, nullptr, nullptr, hInst, nullptr);

    ShowWindow(hwnd, SW_SHOW);

    std::shared_ptr<RHI::IRenderer> renderer = std::make_shared<DX11::DX11Renderer>();
    renderer->Initialize(hwnd);

    std::vector<Vertex> vertices =
    {
        { { -1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f } },
        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f } },
        { { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },

        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f } },
        { {  1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } },
        { { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },
    };

    RHI::BufferDesc desc{};
    desc.size = vertices.size() * sizeof(Vertex);
    desc.type = RHI::BufferType::Vertex;
    desc.bindFlags = RHI::BindFlags::Bind_VertexBuffer;
    desc.memory = RHI::MemoryType::GPUOnly;
    auto vb = renderer->CreateBuffer(desc);
    vb->Update(vertices.data(), desc.size, 0);

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string currentDir = std::filesystem::path(exePath).parent_path().string();

    char cwd[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, cwd);

    OutputDebugStringA((std::string("CWD: ") + cwd + "\n").c_str());
    OutputDebugStringA((std::string("EXE: ") + exePath + "\n").c_str());

    auto vsData = LoadFile("vertex.cso");
    auto psData = LoadFile("pixel.cso");

    RHI::ShaderDesc vsDesc{};
    vsDesc.stage = ShaderStage::Vertex;
    vsDesc.bytecode = vsData.data();
    vsDesc.bytecodeSize = vsData.size();
    vsDesc.entryPoint = "VSMain";

    RHI::ShaderDesc psDesc{};
    psDesc.stage = ShaderStage::Fragment;
    psDesc.bytecode = psData.data();
    psDesc.bytecodeSize = psData.size();
    psDesc.entryPoint = "PSMain";

    auto vs = renderer->CreateShader(vsDesc);
    auto ps = renderer->CreateShader(psDesc);

    RHI::PipelineStateDesc psoDesc{};
    psoDesc.vs = vs.get();
    psoDesc.ps = ps.get();

    psoDesc.inputLayout.attributes.clear();
    psoDesc.inputLayout.attributes.push_back(
        { 0, RHI::VertexFormat::Float3, 0, 0 });
    psoDesc.inputLayout.attributes.push_back(
        { 2, RHI::VertexFormat::Float2, 12, 0 });

    // layouts（这里才是 stride）
    psoDesc.inputLayout.layouts.clear();
    psoDesc.inputLayout.layouts.push_back({ 0, sizeof(Vertex), RHI::InputRate::Vertex, 0 });
    assert(sizeof(Vertex) == 20);

    // render target
    psoDesc.renderTargets.colorFormats.resize(1);
    psoDesc.renderTargets.colorFormats[0] = RHI::TextureFormat::RGBA8_UNorm;
    psoDesc.renderTargets.sampleCount = 1;

    OutputDebugStringA("Before CreatePipelineState\n");

    auto pso = renderer->CreatePipelineState(psoDesc);

    OutputDebugStringA("After CreatePipelineState\n");

    bool running = true;
    MSG msg{};

    while (running)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                running = false;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        renderer->BeginFrame();

        auto cmd = renderer->CreateCommandList();
        cmd->Begin();

        auto rtv = renderer->GetCurrentBackBufferRTV();
        auto dsv = renderer->GetDepthStencil();
        cmd->SetRenderTargets(&rtv, 1, dsv);

        cmd->SetPipelineState(pso.get());

        cmd->SetVertexBuffer(vb.get());

        RHI::Viewport vp{};
        vp.x = 0;
        vp.y = 0;
        vp.width = 800;
        vp.height = 600;
        vp.minDepth = 0;
        vp.maxDepth = 1;

        cmd->SetViewport(vp);

        cmd->Draw(6);

        cmd->End();

        renderer->Submit(cmd.get());
        renderer->EndFrame();
    }

    renderer->Shutdown();
    return 0;
}