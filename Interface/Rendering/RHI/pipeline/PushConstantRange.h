#pragma once
#include <cstdint>
#include "RHIEnum.h"
namespace RHI {
    struct PushConstantRange
    {
        ShaderStage stageFlags;
        uint32_t offset;
        uint32_t size;  //×Ö½Ú
    };
}