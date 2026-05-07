#pragma once
#include <d3d11.h>
#include "Viewport.h"

namespace DX11 {
    class DX11ViewportUtils {
    public:
        // 将 RHI 通用 Viewport 转换为 D3D11 原生 Viewport
        static D3D11_VIEWPORT ToDirectX(const RHI::Viewport& vp) {
            D3D11_VIEWPORT dxVp;
            dxVp.TopLeftX = vp.x;
            dxVp.TopLeftY = vp.y;
            dxVp.Width = vp.width;
            dxVp.Height = vp.height;
            dxVp.MinDepth = vp.minDepth;
            dxVp.MaxDepth = vp.maxDepth;
            return dxVp;
        }

        // 将 RHI 通用 Rect 转换为 D3D11 原生 RECT (注意：DX11 的 Scissor 使用的是 Left/Top/Right/Bottom)
        static D3D11_RECT ToDirectX(const RHI::Rect& rect) {
            D3D11_RECT dxRect;
            dxRect.left = rect.left;
            dxRect.top = rect.top;
            dxRect.right = rect.right;
            dxRect.bottom = rect.bottom;
            return dxRect;
        }
    };

    class DX11Viewport : public RHI::IViewport {
    public:
        DX11Viewport(const RHI::Viewport& vp) : m_desc(vp) {
            m_dxViewport.TopLeftX = vp.x;
            m_dxViewport.TopLeftY = vp.y;
            m_dxViewport.Width = vp.width;
            m_dxViewport.Height = vp.height;
            m_dxViewport.MinDepth = vp.minDepth;
            m_dxViewport.MaxDepth = vp.maxDepth;
        }

        const RHI::Viewport& GetDesc() const override { return m_desc; }
        const D3D11_VIEWPORT& GetNative() const { return m_dxViewport; }

    private:
        RHI::Viewport m_desc;
        D3D11_VIEWPORT m_dxViewport;
    };

    class DX11ScissorRect : public RHI::IScissorRect {
    public:
        DX11ScissorRect(const RHI::Rect& rect) : m_desc(rect) {
            m_dxRect.left = rect.left;
            m_dxRect.top = rect.top;
            m_dxRect.right = rect.right;
            m_dxRect.bottom = rect.bottom;
        }

        const RHI::Rect& GetDesc() const override { return m_desc; }
        const D3D11_RECT& GetNative() const { return m_dxRect; }

    private:
        RHI::Rect m_desc;
        D3D11_RECT m_dxRect;
    };
}