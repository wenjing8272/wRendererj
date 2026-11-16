#pragma once
#include"Scene.h"
#include<vector>

namespace wrj {

	inline const std::vector<Vertex> getVertices()
	{
		Vertex v1
		{
			.position{0.0f,-0.5f,0.0f,1.0f},
			.color{1.0f,0.0f,0.0f,1.0f}
		};

		Vertex v2
		{
			.position{ -0.5f,0.5f,0.0f,1.0f },
			.color{ 0.0f,1.0f,0.0f,1.0f }
		};


		Vertex v3
		{
			.position{ 0.5f,0.5f,0.0f,1.0f },
			.color{ 0.0f,0.0f,1.0f,1.0f }
		};
		return { v1,v2,v3 };
	}
}