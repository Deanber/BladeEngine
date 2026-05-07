#pragma once
#include <vector>
#include "ITexture.h"

namespace RHI {
    class IRenderTexture : public ITexture
    {
    public:
        virtual ~IRenderTexture() = default;

        virtual TextureViewDesc GetRTVDesc() const = 0;
        virtual TextureViewDesc GetDSVDesc() const = 0;
        virtual TextureViewDesc GetSRVDesc() const = 0;
    };
}