#include"JGraphicResourceWatcher.h"
#include"../../JEditorAttribute.h"
#include"../../../../Graphic/JGraphicPrivate.h"
#include"../../../../Graphic/GraphicResource/JGraphicResourceManager.h" 
#include"../../../../Graphic/GraphicResource/JGraphicResourceInfo.h" 
#include"../../../../../ThirdParty/imgui/imgui.h" 

namespace JinEngine
{
	namespace Editor
	{
		JGraphicResourceWatcher::JGraphicResourceWatcher(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute, 
			const J_EDITOR_PAGE_TYPE pageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), pageType, windowFlag)
		{}
		JGraphicResourceWatcher::~JGraphicResourceWatcher() {}

		J_EDITOR_WINDOW_TYPE JGraphicResourceWatcher::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::GRAPHIC_WATCHER;
		}
		void JGraphicResourceWatcher::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated())
			{
				using GraphicDebugInterface = Graphic::JGraphicPrivate::DebugInterface;
				Graphic::JGraphicResourceManager* grManager = GraphicDebugInterface::GetGraphicResourceManager();

				Core::JEnumInfo* rInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_RESOURCE_TYPE).name());
				Core::JEnumInfo* bInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_BIND_TYPE).name());
				auto rNameVec = rInfo->GetEnumNameVec();
				auto bNameVec = bInfo->GetEnumNameVec();
				 
				for (uint i = 0; i < (uint)Graphic::J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				{
					Graphic::JGraphicResourceManager::ResourceTypeDesc& desc = grManager->typeDesc[i];
					ImGui::Text(rNameVec[i].c_str());
					ImGui::Text(("Resource Count:" + std::to_string(desc.count)).c_str());
					for (uint j = 0; j < (uint)Graphic::J_GRAPHIC_BIND_TYPE::COUNT; ++j)
					{
						ImGui::Text(bNameVec[j].c_str());
						ImGui::Text(("View Count:" + std::to_string(desc.viewInfo[j].count)).c_str());
						ImGui::Text(("View Capacity:" + std::to_string(desc.viewInfo[j].capacity)).c_str());
						ImGui::Text(("View Offset:" + std::to_string(desc.viewInfo[j].offset)).c_str());
					}
					ImGui::Separator();
				}
			} 
			CloseWindow();
		}
	}
}