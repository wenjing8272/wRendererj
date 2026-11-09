#pragma once

#include<cstdint>
#include<vector>
#include<limits>

namespace wrj {

	struct Offset2D
	{
		std::int32_t x{ 0 };
		std::int32_t y{ 0 };
	};
	struct Extent2D
	{
		std::uint32_t width{ 0 };
		std::uint32_t height{ 0 };
	};
	struct Area2D 
	{
		Offset2D offset;
		Extent2D extent;
	};
	struct NativeResources
	{
		void* display{ nullptr };
		void* window{ nullptr };
		std::uint32_t frameBufferWidth{ 0 };
		std::uint32_t frameBufferHeight{ 0 };
	};
	struct Viewport
	{
		float x{ 0.0f };
		float y{ 0.0f };
		float width{ 0.0f };
		float height{ 0.0f };
		float minDepth{ 0.0f };
		float maxDepth{ 1.0f };
	};
	enum class Format 
	{
		Undefined,
		R8G8B8A8_UNorm,
		R8G8B8A8_SRGB,
		B8G8R8A8_UNorm,
		B8G8R8A8_SRGB,
		R16G16B16A16_SFloat,
		R8G8_UNorm,
		D32_SFloat
	};
	enum class BufferUsage:std::uint32_t
	{
		None = 0,
		VertexBuffer = 1u << 0,
		IndexBuffer = 1u << 1,
		UniformBuffer = 1u << 2,
		StorageBuffer = 1u << 3,
		TransferSrc = 1u << 4,
		TransferDst = 1u << 5
	};
	constexpr BufferUsage operator|(BufferUsage a, BufferUsage b)
	{
		return static_cast<BufferUsage>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
	}
	constexpr BufferUsage operator&(BufferUsage a, BufferUsage b)
	{
		return static_cast<BufferUsage>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
	}
	constexpr bool isFlag(BufferUsage value, BufferUsage flag)
	{
		return (value & flag) != BufferUsage::None;
	}
	enum class TextureUsage :std::uint32_t
	{
		None = 0,
		Sampled = 1u << 0,
		ColorAttachment = 1u << 1,
		DepthAttachment = 1u << 2,
		Storage = 1u << 3,
		TransferSrc = 1u << 4,
		TransferDst = 1u << 5
	};
	constexpr TextureUsage operator|(TextureUsage a, TextureUsage b)
	{
		return static_cast<TextureUsage>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
	}
	constexpr TextureUsage operator&(TextureUsage a, TextureUsage b)
	{
		return static_cast<TextureUsage>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
	}
	constexpr bool isFlag(TextureUsage value, TextureUsage flag)
	{
		return (value & flag) != TextureUsage::None;
	}

	enum class ShaderStage :std::uint32_t
	{
		None = 0,
		Vertex = 1u << 0,
		Fragment = 1u << 1,
		Compute = 1u << 2
	};
	constexpr ShaderStage operator|(ShaderStage a, ShaderStage b)
	{
		return static_cast<ShaderStage>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
	}
	constexpr ShaderStage operator&(ShaderStage a, ShaderStage b)
	{
		return static_cast<ShaderStage>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
	}
	constexpr bool isFlag(ShaderStage value, ShaderStage flag)
	{
		return (value & flag) != ShaderStage::None;
	}
	enum class VertexFormat {
		Float,
		Float2,
		Float3,
		Float4,
		UInt32
	};

	enum class PrimitiveTopology {
		TriangleList,
		TriangleStrip
	};

	enum class CullMode {
		None,
		Front,
		Back
	};

	enum class FrontFace {
		CounterClockwise,
		Clockwise
	};

	enum class CompareOp {
		Never,
		Less,
		LessOrEqual,
		Always
	};

	enum class IndexType {
		UInt16,
		UInt32
	};

	constexpr std::uint32_t vaildValue = UINT32_MAX;
	struct VertexBinding
	{
		std::uint32_t binding{ UINT32_MAX };
		std::uint32_t stride{ UINT32_MAX };
		bool isPerInstance{ false };
		
		constexpr bool isVaild()const
		{
			return binding != vaildValue && stride != vaildValue && stride == 0;
		}
	};

	struct VertexAttribute
	{
		std::uint32_t location{ UINT32_MAX };
		std::uint32_t binding{ UINT32_MAX };
		Format format{ Format::Undefined };
		std::uint32_t offset{ UINT32_MAX };

		constexpr bool isVaild()const
		{
			return location != vaildValue && binding != vaildValue && format != Format::Undefined && offset != vaildValue;
		}
	};

	struct PushConstantRange
	{
		ShaderStage stage = ShaderStage::Vertex | ShaderStage::Fragment;
		std::uint32_t offset{ UINT32_MAX };
		std::uint32_t size{ UINT32_MAX };

		constexpr bool isVaild()const
		{
			return stage != ShaderStage::None && offset != vaildValue && size != vaildValue;
		}
	};

	struct GraphicsPipeline
	{

	};
}