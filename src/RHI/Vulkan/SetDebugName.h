#pragma once
#include<vulkan/vulkan.h>

namespace RHI {

	class DebugName {
	public:
		DebugName() = delete;
		DebugName(const DebugName& otherSetDebugName) = delete;
		DebugName& operator=(const DebugName& otherSetDebugName) = delete;

		template<class T>struct VkObjectStairts;
		template<>struct VkObjectStairts<VkPipeline> {
			static constexpr VkObjectType Value = VK_OBJECT_TYPE_PIPELINE;
		};
		template<>struct VkObjectStairts<VkPipelineLayout> {
			static constexpr VkObjectType Value = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
		};

		static void init(VkDevice device)
		{
#ifndef NDEBUG
			m_device = device;
			m_vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
#else
			void(device);
#endif // !NDEBUG

		}
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
}