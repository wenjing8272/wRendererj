#pragma once

#include<cstdint>
#include<vector>
#include<limits>
#include<span>

namespace wrj {
	struct NativeResources
	{
		void* display{ nullptr };
		void* window{ nullptr };
	};

	struct InitResource
	{
		NativeResources native{};
		std::uint32_t frameBufferWidth{ 0 };
		std::uint32_t frameBufferHeight{ 0 };
		const char* appName = "wRenderj";
	};

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
		R32G32B32A32_SFloat,
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

	enum class MemoryProperty 
	{
		HostVisible,
		DeviceLocal
	};

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

	constexpr std::uint32_t vaildValue = UINT32_MAX;

	template<class Tag>
	struct Handle
	{
		std::uint32_t index{ UINT32_MAX };
		std::uint32_t generation{ UINT32_MAX };

		constexpr bool isVaild()
		{
			return index != vaildValue;
		}
	};

	using BufferHandle = Handle<struct BufferTag>;
	using TextureHandle = Handle<struct TextureTag>;
	using ViewHandle = Handle<struct ViewTag>;
	using ShaderHandle = Handle<struct ShaderTag>;
	using PipelineHandle = Handle<struct PipelineTag>;

	struct VertexBinding
	{
		std::uint32_t binding{ UINT32_MAX };
		std::uint32_t stride{ UINT32_MAX };
		bool isPerInstance{ false };
		
		constexpr bool isVaild()const
		{
			return binding != vaildValue && stride != vaildValue && stride != 0;
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

	enum class PrimitiveTopology
	{
		LineList,
		TriangleList,
		TriangleStrip
	};

	enum class PolygonMode
	{
		Fill,
		Line,
		Point
	};

	enum class CullMode
	{
		Front,
		Back
	};

	enum class FrontFace
	{
		Clockwise,
		CounterClockwise
	};

	enum class SampleCount
	{
		e1,
		e4,
		e16
	};

	enum class DynamicState
	{
		Viewport,
		Scissor
	};

	enum class RenderTarget
	{
		Swapchain,
		GeneralImage
	};

	struct GraphicsPipeline
	{
		ShaderHandle vertexShader{};
		ShaderHandle fragmentShader{};
		const char* vertexName = "vertMain";
		const char* fragmentName = "fragMain";
		VertexBinding vertexBinding;
		std::span<VertexAttribute> vertexAttribute;
		PushConstantRange pushConstantRange{};
		PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleList };
		PolygonMode polygonMode{ PolygonMode::Fill };
		CullMode cullMode{ CullMode::Back };
		FrontFace frontFace{ FrontFace::Clockwise };
		bool depthBaisEnable{ false };
		SampleCount sampleCount{ SampleCount::e1 };
		bool blendEnable{ false };
		std::span<DynamicState> dynamicStates;
	};
}