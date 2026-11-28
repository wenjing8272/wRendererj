#include"SceneManager.h"
#include<tiny_gltf.h>
#include<ktx.h>

#include<iostream>
#include<filesystem>

namespace Scene {
	SceneManager::SceneManager()
	{
		std::cout<<"scene manager"<<'\n';
	}
	SceneManager::~SceneManager()
	{
		std::cout<<"scene manager"<<'\n';
	}
	std::uint64_t SceneManager::getVertexSize()
	{
		if (meshes.empty())
		{
			std::cout << "not scene data" << '\n';
			return 0;
		}
		std::cout << "vertices bytes: " << meshes[0].vertices.size() * sizeof(Vertex) << '\n';
		return meshes[0].vertices.size() * sizeof(Vertex);
	}
	
	void* SceneManager::getVertexData()
	{
		return meshes[0].vertices.data();
	}

    std::uint64_t SceneManager::getIndexSize()
    {
		std::cout << "indices bytes: " << meshes[0].indices.size() * sizeof(std::uint32_t) << '\n';
        return meshes[0].indices.size() * sizeof(std::uint32_t);
    }
    void* SceneManager::getIndexData()
    {
        return meshes[0].indices.data();
    }
	const RHI::VertexBinding SceneManager::getVertexBinding()const
	{
		return Vertex::getVertexBinding();
	}
	const std::vector<RHI::VertexAttribute> SceneManager::getVertexAttribute()const 
	{
		return Vertex::getVertexAttribute();
	}

	static bool keepRawImage(
		tinygltf::Image* image,
		const int imageIndex,
		std::string* error,
		std::string* warn,
		int width,
		int height,
		const unsigned char* bytes,
		int size,
		void* data)
	{
		image->image.assign(bytes, bytes + size);
		return true;
	}

	static const std::uint8_t* accessorData(const tinygltf::Model& model, const tinygltf::Accessor& accessor, std::size_t& stride)
	{
		const auto& bufferView = model.bufferViews[accessor.bufferView];
		const auto& buffer = model.buffers[bufferView.buffer];
		std::cout << "buffer and view name: " << buffer.name << bufferView.name << '\n';

		stride = static_cast<std::size_t> (accessor.ByteStride(bufferView));

		return buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
	}

	static void readData(const tinygltf::Accessor accessor, const std::uint8_t* base, size_t i,std::size_t stribe,float* out,std::size_t count)
	{
		const std::uint8_t* p = base + i * stribe;
		for (std::size_t j = 0;j < count;++j)
		{
			switch (accessor.componentType)
			{
			case TINYGLTF_COMPONENT_TYPE_FLOAT:
				out[j] = reinterpret_cast<const float*>(p)[j];
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
				out[j] = accessor.normalized ? p[j] / 255.0f : (float)p[j];
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
				uint16_t v = reinterpret_cast<const uint16_t*>(p)[j];
				out[j] = accessor.normalized ? v / 65535.0f : (float)v;
				break;
			}
			default: 
				break;
			}
		}
	}

    static std::uint32_t readIndexData(const tinygltf::Accessor& accessor,const std::uint8_t* indexData,std::size_t i,std::size_t indexStribe)
    {
        const std::uint8_t* p = indexData + i * indexStribe;
        switch(accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return *p;
        case TINYGLTF_COMPONENT_TYPE_SHORT:
            return *reinterpret_cast<const std::uint16_t*>(p);
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            return *reinterpret_cast<const std::uint32_t*>(p);
        default:
            return *reinterpret_cast<const std::uint32_t*>(p);
        }
    }
	bool SceneManager::loadglTF(const std::string& path)
	{
		std::cout << "current directory: " << std::filesystem::current_path() << '\n';

		tinygltf::Model model;
		tinygltf::TinyGLTF loader;

		std::string error;
		std::string warn;
		loader.SetImageLoader(keepRawImage, nullptr);

		const bool loaderResult = path.ends_with(".glb") ?
			loader.LoadBinaryFromFile(&model, &error, &warn, path) :
			loader.LoadASCIIFromFile(&model, &error, &warn, path);

		if (!warn.empty())
			std::cout << "loader gltf warn: " << warn.c_str() << '\n';
		if (!loaderResult)
		{
			std::cout << "loader gltf error" << error.c_str() << '\n';
			return false;
		}

		for (const auto& mesh : model.meshes)
		{
			MeshData meshData{};

			for (const auto& prim : mesh.primitives) {
				std::cout << "primitive mode: " << prim.mode << '\n';
				if (prim.mode != -1 && prim.mode != TINYGLTF_MODE_TRIANGLES)continue;

				const auto posIt = prim.attributes.find("POSITION");
				if (posIt == prim.attributes.end())continue;
				const auto& posAcc = model.accessors[posIt->second];
				std::size_t posStribe{ 0 };
				const std::uint8_t* posData = accessorData(model, posAcc, posStribe);

				const tinygltf::Accessor* normalAcc{ nullptr };
				std::size_t normalStribe{ 0 };
				const std::uint8_t* normalData{ nullptr };
				if (const auto normalIt = prim.attributes.find("NORMAL");normalIt != prim.attributes.end())
				{
					normalAcc = &model.accessors[normalIt->second];
					normalData = accessorData(model, *normalAcc, normalStribe);
				}

				const tinygltf::Accessor* textureUVAcc{ nullptr };
				std::size_t textureUVStribe{ 0 };
				const std::uint8_t* textureUVData{ nullptr };
				if (const auto textureUVIt = prim.attributes.find("TEXCOORD_0");textureUVIt != prim.attributes.end())
				{
					textureUVAcc = &model.accessors[textureUVIt->second];
					textureUVData = accessorData(model, *textureUVAcc, textureUVStribe);
				}

				const std::uint32_t baseVertex = static_cast<std::uint32_t> (meshData.vertices.size());
				meshData.vertices.resize(baseVertex + posAcc.count);
				for (std::size_t i = 0;i < posAcc.count;++i)
				{
					Vertex& v = meshData.vertices[baseVertex + i];

					readData(posAcc, posData, i, posStribe, &v.position.x, 3);
					if (normalAcc != nullptr)readData(*normalAcc, normalData, i, normalStribe, &v.normal.x, 3);
					if (textureUVAcc != nullptr)readData(*textureUVAcc, textureUVData, i, textureUVStribe, &v.uv.x, 2);
				}
                Submesh submesh{};
                submesh.firstIndex = meshData.indices.size();
                submesh.materialIndex = prim.material;
                
                std::cout<<"vertex total: "<<posAcc.count<<'\n';
                if(prim.indices >= 0)
                {
                    std::cout<<"use indices of model"<<'\n';
                    const auto& indexAcc = model.accessors[prim.indices];
                    std::size_t indexStribe{0};
                    const std::uint8_t* indexData = accessorData(model,indexAcc,indexStribe);
                    for(std::size_t i = 0;i < indexAcc.count;++i)
                    {
                        meshData.indices.push_back(baseVertex + readIndexData(indexAcc,indexData,i,indexStribe));
                    }
                }
                else
                {
                    std::cout<<"use vertex of model to construct indices"<<'\n';
                    for(std::size_t i = 0;i < posAcc.count;++i)
                    {
                        meshData.indices.push_back(baseVertex + static_cast<std::uint32_t>(i));
                    }
                }
				submesh.indexCount = meshData.indices.size() - submesh.firstIndex;
                meshData.submeshes.emplace_back(submesh);
			}
            meshes.emplace_back(meshData);
		}
		return true;
	}


}
