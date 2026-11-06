#pragma once
#include"Types.h"
#include<string>
#include<vector>

namespace wrj {
	class RenderContext {
	public:
		RenderContext() = default;
		~RenderContext();
		RenderContext(const RenderContext& otherRenderContext) = delete;
		RenderContext& operator=(const RenderContext& otherRenderContext) = delete;
		RenderContext(RenderContext&& otherRenderContext) = delete;
		RenderContext& operator=(RenderContext&& otherRenderContext) = delete;
		void initRenderContext(const NativeResources& native,const std::vector<std::string>& instanceExtensions);
	};
}