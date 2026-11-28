#pragma once
#include<Types.h>
#include<glm/glm.hpp>

#include<vector>
#include<string>

namespace Scene {
	struct Vertex 
	{
		glm::vec3 position{ 0.0f,0.0f,0.0f };
		glm::vec3 normal{ 0.0f,0.0f,1.0f };
		glm::vec2 uv{ 0.0f,0.0f };
        
        static RHI::VertexBinding getVertexBinding()
        {
            return {.binding = 0,.stride = sizeof(Vertex)};
        }
        static std::vector<RHI::VertexAttribute> getVertexAttribute()
        {
            return {{
                {.location = 0,.binding = 0,.format = RHI::Format::R32G32B32_SFloat,.offset = offsetof(Vertex,position)},
                {.location = 1,.binding = 0,.format = RHI::Format::R32G32B32_SFloat,.offset = offsetof(Vertex,normal)},
                {.location = 2,.binding = 0,.format = RHI::Format::R32G32_SFloat,.offset = offsetof(Vertex,uv)}
            }};
        }
	};

	struct Submesh {
		std::uint32_t firstIndex;
		std::uint32_t indexCount;
		std::uint32_t materialIndex;
	};

	struct MeshData {
		std::vector<Vertex> vertices;
		std::vector<std::uint32_t> indices;
		RHI::IndexType indexType = RHI::IndexType::U32;

		std::vector<Submesh> submeshes;
	};

	class SceneManager {
	public:
		SceneManager();
		~SceneManager();
		SceneManager(const SceneManager& otherManager) = delete;
		SceneManager& operator=(const SceneManager& otherManager) = delete;
		SceneManager(SceneManager&& otherManager) = delete;
		SceneManager& operator=(SceneManager&& otherManager) = delete;

		const RHI::VertexBinding getVertexBinding()const;
		const std::vector<RHI::VertexAttribute> getVertexAttribute()const;

		bool loadglTF(const std::string& path);

		std::uint64_t getVertexSize();
		void* getVertexData();

        std::uint64_t getIndexSize();
        void* getIndexData();

		const std::vector<MeshData>& getMesh()const { return meshes; }
	private:

		std::vector<std::uint32_t> m_indices;
		std::vector<MeshData> meshes;
	};
}
