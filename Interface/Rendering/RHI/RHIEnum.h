#pragma once
#include <cstdint>

namespace RHI {

    enum class MemoryType
    {
        GPUOnly,    // Default (DX11) / DEFAULT (DX12) / DEVICE_LOCAL (VK)
        CPUUpload,  // Dynamic / UPLOAD / HOST_VISIBLE
        CPUReadback // Staging / READBACK
    };

    enum class ResourceBindState : uint32_t
    {
        Undefined,
        GPURead,
        GPUWrite,
        CopySrc,
        CopyDst
    };

    enum class BufferType
    {
        Vertex,        // 顶点缓冲
        Index,         // 索引缓冲
        Constant,      // 常量缓冲（Uniform）
        Structured,    // 结构化缓冲（StructuredBuffer）
        Raw,           // 原始缓冲（ByteAddressBuffer）
        Indirect,      // 间接绘制参数
        Staging        // CPU上传用（中转）
    };

    enum BindFlags : uint32_t
    {
        Bind_None = 0,
        Bind_VertexBuffer = 1 << 0,
        Bind_IndexBuffer = 1 << 1,
        Bind_ConstantBuffer = 1 << 2,
        Bind_ShaderResource = 1 << 3,
        Bind_UnorderedAccess = 1 << 4,
        Bind_RenderTarget = 1 << 5,
        Bind_DepthStencil = 1 << 6,
        Bind_IndirectBuffer = 1 << 7,
    };

    enum class BufferUsage
    {
        Default,    // GPU读写（常规使用）
        Immutable,  // 只读（创建后不再改变）
        Dynamic,    // CPU频繁更新（每帧Map/Update）
        Staging     // CPU <-> GPU 数据拷贝
    };

    enum class ResourceAccessState
    {
        Undefined,

        VertexBuffer,
        IndexBuffer,
        ConstantBuffer,

        ShaderResource,
        UnorderedAccess,

        IndirectArgument,

        CopySrc,
        CopyDst,

        RenderTarget,
        DepthWrite,
        DepthRead,

        Present,
    };

    enum class BarrierType
    {
        Buffer,
        Texture
    };

    class IResource;

    struct BarrierDesc
    {
        IResource* pResource;
        ResourceAccessState before;
        ResourceAccessState after;

        uint32_t baseMipLevel = 0;
        uint32_t levelCount = 1;

        uint32_t baseArrayLayer = 0;
        uint32_t layerCount = 1;
    };

    enum class ResourceType : uint32_t
    {
        Unknown = 0,
        Buffer,
        Texture,
        Sampler,
        Shader,
        PipelineLayout,
    };

    enum class TextureDimension
    {
        Texture1D,
        Texture2D,
        Texture3D,
    };

    enum class TextureFormat
    {
        Unknown,

        // --- 8-bit ---
        R8_UNorm,
        RGBA8_UNorm,
        RGBA8_UNorm_SRGB,
        BGRA8_UNorm,

        // --- 10/11-bit HDR ---
        R11G11B10_Float,
        RGB10A2_UNorm,

        // --- 16-bit ---
        R16_Float,
        RG16_Float,
        RGBA16_Float,
        R16_UNorm,

        // --- 32-bit ---
        R32_Float,
        RGBA32_Float,

        // --- Depth / Stencil ---
        D24_S8_UNorm,
        D32_Float,
        D32_Float_S8_UInt,

        // --- Block Compression (PC) ---
        BC1_UNorm, // DXT1
        BC3_UNorm, // DXT5
        BC4_UNorm, // 单通道
        BC5_UNorm, // 双通道 (Normal Map)
        BC7_UNorm, // 高质量颜色

        // --- ASTC (Mobile/Universal) ---
        ASTC_4x4_UNorm,
        ASTC_6x6_UNorm,
    };

    inline bool IsDepthFormat(TextureFormat format) {
        switch (format) {
        case TextureFormat::D24_S8_UNorm:
        case TextureFormat::D32_Float:
        case TextureFormat::D32_Float_S8_UInt:
            return true;
        default:
            return false;
        }
    }

    inline bool IsStencilFormat(TextureFormat format) {
        switch (format) {
        case TextureFormat::D24_S8_UNorm:
        case TextureFormat::D32_Float_S8_UInt:
            return true;
        default:
            return false;
        }
    }

    inline bool IsCompressedFormat(TextureFormat format) {
        return (int)format >= (int)TextureFormat::BC1_UNorm;
    }

    inline bool IsSRGBFormat(TextureFormat format) {
        return format == TextureFormat::RGBA8_UNorm_SRGB;
    }

    enum class TextureViewUsage
    {
        SRV,
        RTV,
        DSV,
        UAV
    };

    enum class TextureViewDimension
    {
        Texture1D,
        Texture2D,
        Texture2DArray,
        Texture3D,
        Cube,
        CubeArray
    };

    enum class DescriptorType
    {
        UniformBuffer,
        StorageBuffer,

        SampledTexture,
        StorageTexture,

        Sampler,

        CombinedImageSampler,

        AccelerationStructure,
        StructuredBuffer,
        RWStructuredBuffer,
        ByteAddressBuffer,
        RWByteAddressBuffer,
        InputAttachment,
    };

    enum class Filter
    {
        // --- 标准采样 ---
        MinMagMipPoint,                          // Point
        MinMagPointMipLinear,
        MinPointMagLinearMipPoint,
        MinPointMagMipLinear,
        MinLinearMagMipPoint,
        MinLinearMagPointMipLinear,
        MinMagLinearMipPoint,
        MinMagMipLinear,                         // Linear (最常用)
        Anisotropic,                             // 各向异性

        // --- 比较采样 (Comparison) - 用于 Shadow Map ---
        ComparisonMinMagMipPoint,
        ComparisonMinMagPointMipLinear,
        ComparisonMinPointMagLinearMipPoint,
        ComparisonMinPointMagMipLinear,
        ComparisonMinLinearMagMipPoint,
        ComparisonMinLinearMagPointMipLinear,
        ComparisonMinMagLinearMipPoint,
        ComparisonMinMagMipLinear,
        ComparisonAnisotropic,

        // --- 最值采样 (Minimum / Maximum) - 用于特定优化算法 ---
        MinimumMinMagMipPoint,
        MinimumMinMagMipLinear,
        MaximumMinMagMipPoint,
        MaximumMinMagMipLinear,
    };

    enum class SamplerAddressMode
    {
        Wrap,          // repeat
        Mirror,        // mirror repeat
        Clamp,         // clamp to edge
        Border,        // border color
        MirrorOnce     // D3D特有
    };

    enum class ComparisonFunc
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
    };

    enum class BorderColor
    {
        FloatTransparentBlack,
        FloatOpaqueBlack,
        FloatOpaqueWhite,

        // 可选扩展（Vulkan支持）
        IntTransparentBlack,
        IntOpaqueBlack,
        IntOpaqueWhite
    };

    enum class SamplerReductionMode
    {
        Standard,
        Comparison,
        Minimum,
        Maximum
    };

    enum class SamplerFlags : uint32_t
    {
        None = 0,
        Subsampled = 1 << 0,
        Immutable = 1 << 1,
    };

    enum class ShaderStage : uint32_t // 显式指定底层类型为 uint32_t
    {
        None = 0,
        Vertex = 1 << 0,  // 1 (000001)
        Fragment = 1 << 1,  // 2 (000010)
        Compute = 1 << 2,  // 4 (000100)

        // 预留阶段
        Geometry = 1 << 3,  // 8
        Hull = 1 << 4,  // 16
        Domain = 1 << 5,  // 32

        // 常用组合（商讨开发时的便利性）
        AllGraphics = Vertex | Fragment | Geometry | Hull | Domain,
        All = 0xFFFFFFFF
    };

    inline ShaderStage operator&(ShaderStage a, ShaderStage b) {
        return static_cast<ShaderStage>(
            static_cast<uint32_t>(a) & static_cast<uint32_t>(b)
            );
    }

    // 重载 | 运算符 (用于组合 Stage)
    inline ShaderStage operator|(ShaderStage a, ShaderStage b) {
        return static_cast<ShaderStage>(
            static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
            );
    }

    enum class ShaderResourceType
    {
        Unknown = 0,

        // --- 常量数据 ---
        ConstantBuffer,       // DX12: CBV | Vulkan: Uniform Buffer
        PushConstant,         // Vulkan 特有 / DX12: Root Constants (极快的小数据)

        // --- 只读资源 (SRV) ---
        Texture,              // 纹理本身 (ITexture)
        Sampler,              // 采样器状态 (ISampler)
        StructuredBuffer,     
        ByteAddressBuffer,
        AccelerationStructure, // 光线追踪加速结构 (TLAS)

        // --- 可读写资源 (UAV) ---
        RWTexture,            // Compute Shader 写入贴图
        RWStructuredBuffer,   
        RWByteAddressBuffer,

        // --- 输入附件 (Vulkan 特有) ---
        InputAttachment       // 用于移动端延迟渲染的 Subpass 读取
    };

    enum class IndexFormat : uint8_t {
        Uint16, // 对应 DXGI_FORMAT_R16_UINT
        Uint32  // 对应 DXGI_FORMAT_R32_UINT
    };

    enum class Format
    {
        Unknown,

        // --- float ---
        R32_FLOAT,
        RG32_FLOAT,
        RGB32_FLOAT,
        RGBA32_FLOAT,

        // --- half ---
        R16_FLOAT,
        RG16_FLOAT,
        RGBA16_FLOAT,

        // --- uint ---
        R32_UINT,
        RG32_UINT,
        RGB32_UINT,
        RGBA32_UINT,

        // --- normalized ---
        RGBA8_UNORM,
        RGBA8_SNORM,

        // --- packed ---
        R10G10B10A2_UNORM,
    };

    enum class VertexFormat {
        Float,
        Float2,
        Float3,
        Float4,
        UByte4,
        UByte4N,
        Int,
        Half,
        UInt,
        Packed,
        UByte4Norm,
    };

    enum class CullMode
    {
        None,
        Back,
        Front
    };

    enum class CompareFunc
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
    };

    enum class StencilOp
    {
        Keep,
        Zero,
        Replace,
        Incr,
        IncrSat,
        Decr,
        DecrSat,
        Invert,
    };

    enum class BlendFactor
    {
        Zero,
        One,
        SrcAlpha,
        InvSrcAlpha,
        DestAlpha,
        InvDestAlpha,
        SrcColor,
        InvSrcColor,
        DestColor,
        InvDestColor,
        SrcAlphaSat,
        BlendConst,
        InvBlendConst,
    };

    enum class BlendOp
    {
        Add,
        Subtract,
        RevSubtract,
        Min,
        Max,
    };

    enum class PrimitiveTopology
    {
        TriangleList,
        TriangleStrip,
        LineList,
        LineStrip,

        PatchList
    };
}
