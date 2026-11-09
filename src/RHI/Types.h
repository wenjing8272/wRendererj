#pragma once

#include<cstdint>
#include<vector>

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
		D32_SFloat,
	};
	enum class BufferUsage:std::uint32_t
	{
		None = 0,
		VertexBuffer = 1u << 0,
		IndexBuffer = 1u << 1,
		UniformBuffer = 1u << 2,
		StorageBuffer = 1u << 3,
		TransferSrc = 1u << 4,
		TransferDst = 1u << 5,
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
}