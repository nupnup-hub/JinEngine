#include"JGraphicResourceWatcher.h"
#include"../../JEditorAttribute.h"
#include"../../../../Graphic/JGraphic.h"
#include"../../../../Graphic/JGraphicResourceManager.h" 
#include"../../../../Graphic/JGraphicTextureHandle.h" 
#include"../../../../../Lib/imgui/imgui.h"

namespace JinEngine
{
	namespace Editor
	{
		JGraphicResourceWatcher::JGraphicResourceWatcher(std::unique_ptr<JEditorAttribute> attribute, const size_t ownerPageGuid)
			:JEditorWindow(std::move(attribute), ownerPageGuid)
		{}
		JGraphicResourceWatcher::~JGraphicResourceWatcher() {}

		void JGraphicResourceWatcher::UpdateWindow(JEditorUtility* editorUtility)
		{
			JEditorWindow::UpdateWindow(editorUtility);
			Graphic::JGraphicResourceManager* grManager = JGraphic::Instance().graphicResource.get();
			std::string user2DTextreCount = "User2DTextureCount: " + std::to_string(grManager->user2DTextureCount);
			std::string userCubeTextreCount = "userCubeTextreCount: " + std::to_string(grManager->userCubeTextureCount);
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