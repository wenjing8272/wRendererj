#pragma once
#include<glm/glm.hpp>
#include"RHI/Types.h"

#include<vector>
#include<iostream>

namespace wrj {
	struct Vertex {
		glm::vec4 position{};
		glm::vec4 color{};

		static VertexBinding vertexBinding()
		{
			return { .binding = 0,.stride = sizeof(Vertex) };
		}
		static std::vector<VertexAttribute> vertexAttribute()
		{
			return { {{.location = 0,.binding = 0,.format = Format::R32G32B32A32_SFloat,.offset = offsetof(Vertex,position)},
					  {.location = 1,.binding = 0,.format = Format::R32G32B32A32_SFloat,.offset = offsetof(Vertex,color)}
				} };
		}
	};
}
