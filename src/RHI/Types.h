#pragma once

#include<cstdint>
#include<vector>

namespace wrj {

	struct Offset2D
	{
		std::int32_t x = 0;
		std::int32_t y = 0;
	};
	struct Extent2D
	{
		std::uint32_t width = 0;
		std::uint32_t height = 0;
	};
	struct Area2D 
	{
		Offset2D offset;
		Extent2D extent;
	};
	struct NativeResources
	{
		void* display = nullptr;
		void* window = nullptr;
		std::uint32_t frameBufferWidth = 0;
		std::uint32_t frameBufferHeight = 0;
	};
}