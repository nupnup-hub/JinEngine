#include"JGraphicResourceWatcher.h"
#include"../../JEditorAttribute.h"
#include"../../../../Graphic/JGraphic.h"
#include"../../../../Graphic/JGraphicResourceManager.h" 
#include"../../../../Graphic/JGraphicTextureHandle.h" 
#include"../../../../../Lib/imgui/imgui.h"
#include"../../../../Core/Identity/JIdentifier.h"

namespace JinEngine
{
	namespace Editor
	{
		JGraphicResourceWatcher::JGraphicResourceWatcher(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE pageType)
			:JEditorWindow(name, std::move(attribute), pageType)
		{}
		JGraphicResourceWatcher::~JGraphicResourceWatcher() {}

		J_EDITOR_WINDOW_TYPE JGraphicResourceWatcher::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::GRAPHIC_WATCHER;
		}
		void JGraphicResourceWatcher::UpdateWindow()
		{ 
			if (IsActivated())
			{
				Graphic::JGraphicResourceManager* grManager = JGraphic::Instance().graphicResource.get();
				std::string user2DTextreCount = "User2DTextureCount: " + std::to_string(grManager->user2DTextureCount);
				std::string userCubeTextreCount = "userCubeTextreCount: " + std::to_string(grManager->userCubeMapCount);
				std::string rsCount = "renderResultCount: " + std::to_string(grManager->renderResultCount);
				std::string shadowMapCount = "shadowMapCount: " + std::to_string(grManager->shadowMapCount);

				ImGui::Text(user2DTextreCount.c_str());
				ImGui::Text(userCubeTextreCount.c_str());
				ImGui::Text(rsCount.c_str());
				ImGui::Text(shadowMapCount.c_str());

				if (grManager->shadowMapCount > 0)
				{
					ImVec2 windowSize = ImGui::GetWindowSize();
					//ImGui::Image((ImTextureID)JGraphic::Instance().GetGpuSrvDescriptorHandle(grManager->shadowMapHandle[0]->GetSrvHeapIndex()).ptr,
					//	ImVec2(windowSize.x, windowSize.y));
				}
			} 
		}
	}
}