#pragma once

#include<cstdint>
#include<vector>

namespace wrj {
	struct NativeResources
	{
		void* display = nullptr;
		void* window = nullptr;
		std::uint32_t frameBufferWidth = 0;
		std::uint32_t frameBufferHeight = 0;
	};
}