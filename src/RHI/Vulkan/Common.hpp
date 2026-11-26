#pragma once
#include<vulkan/vulkan.h>
#include"../Types.h"
#include"../Log.h"

#include<vector>

namespace RHI {

	//Transfer user to vulkan 
	VkBufferUsageFlags userToVK(BufferUsage bufferUsage);
	VkMemoryPropertyFlags userToVK(MemoryProperty memoryUsage);
	VkFormat userToVK(Format format);
	VkPrimitiveTopology userToVK(PrimitiveTopology topology);
	VkDynamicState userToVK(DynamicState dynamic);
	VkCullModeFlags userToVK(CullMode cullMode);
	VkFrontFace userToVK(FrontFace frontFace);
	VkSampleCountFlagBits userToVK(SampleCount sampleCount);
	VkBool32 userBoolToVK(bool user);

	//Configure debug name tool class
	class DebugName {
	public:
		DebugName() = delete;
		DebugName(const DebugName& otherSetDebugName) = delete;
		DebugName& operator=(const DebugName& otherSetDebugName) = delete;

		//template instantiation to automatic deduction VkObjectType
		template<class T>struct VkObjectStairts;
		template<>struct VkObjectStairts<VkPipeline> {
			static constexpr VkObjectType Value = VK_OBJECT_TYPE_PIPELINE;
		};
		template<>struct VkObjectStairts<VkPipelineLayout> {
			static constexpr VkObjectType Value = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
		};
		template<>struct VkObjectStairts<VkSemaphore>{
			static constexpr VkObjectType Value = VK_OBJECT_TYPE_SEMAPHORE;
		};
		template<>struct VkObjectStairts<VkCommandBuffer>
		{
			static constexpr VkObjectType Value = VK_OBJECT_TYPE_COMMAND_BUFFER;
		};

		//Init tool class 
		static void init(VkDevice device)
		{
#ifndef NDEBUG
			m_device = device;
			m_vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
#else
			void(device);
#endif // !NDEBUG

		}

		//Deduce the object type from the given Vulkan handle.
		//Early-out in debug builds if the debugName,handle,or m_vkSetDebugUtilsObjectNameEXT function pointer is null.
		//No-op in non-debug builds
		template<class vkHandle>
		static void set(vkHandle handle, const char* debugName)
		{
#ifndef NDEBUG
			if (m_vkSetDebugUtilsObjectNameEXT == nullptr || handle == VK_NULL_HANDLE || !debugName)
				return;

			VkDebugUtilsObjectNameInfoEXT info{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
			info.objectHandle = reinterpret_cast<std::uint64_t>(handle);
			info.objectType = VkObjectStairts<vkHandle>::Value;
			info.pObjectName = debugName;

			m_vkSetDebugUtilsObjectNameEXT(m_device, &info);
#else
			void(handle);
			void(debugName);
#endif // !1
		}
	private:
#ifndef NDEBUG
		inline static VkDevice m_device = VK_NULL_HANDLE;
		inline static PFN_vkSetDebugUtilsObjectNameEXT m_vkSetDebugUtilsObjectNameEXT = nullptr;
#endif // !NDEBUG
	};

	//Generate hash key via FNV-1a,combine hashed using Boost's hash_combine
	inline std::uint64_t hashBytes(const void* data, std::size_t size, std::uint64_t seed = 14695981039346656037ull)
	{
		const auto* dataBytes = static_cast<const std::uint8_t*>(data);
		std::uint64_t hash = seed;
		for (std::size_t i = 0;i < size;++i)
		{
			hash ^= dataBytes[i];
			hash *= 1099511628211ull;
		}
		return hash;
	}
	inline void hashCombine(std::uint64_t& seed, std::uint64_t value)
	{
		seed ^= value + 0x9e3779b97f4a7c15 + (seed << 6) + (seed >> 2);
	}

	//Mapppine index
	template<class Record>
	inline std::uint32_t acquireIndex(std::vector<Record>& buffers)
	{
		for (std::uint32_t i = 0;i < buffers.size();++i)
		{
			if (buffers[i].alive)continue;
			auto& buffer = buffers[i];
			buffer.alive = true;
			buffer.generation += 1;
			return i;
		}
		//Since Cpp17,emplace_back direcrly returns a reference to the inserted element.
		//Prior to Cpp17
		/*
			buffers.emplace_back();
			auto& buffer = buffers.back;
		*/
		auto& buffer = buffers.emplace_back();
		buffer.alive = true;					
		buffer.generation = 1;
		return static_cast<std::uint32_t>(buffers.size() - 1);
	}
}