#include"JGraphicResourceWatcher.h"
#include"../../JEditorAttribute.h"
#include"../../../../Graphic/JGraphic.h"
#include"../../../../Graphic/GraphicResource/JGraphicResourceManager.h" 
#include"../../../../Graphic/GraphicResource/JGraphicResourceHandle.h" 
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

				Core::JEnumInfo* rInfo = Core::JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_RESOURCE_TYPE).name());
				Core::JEnumInfo* bInfo = Core::JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_BIND_TYPE).name());
				auto rNameVec = rInfo->GetEnumNameVec();
				auto bNameVec = bInfo->GetEnumNameVec();
				 
				for (uint i = 0; i < (uint)Graphic::J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				{
					Graphic::JGraphicResourceManager::ResourceTypeDesc& desc = grManager->typeDesc[i];
					ImGui::Text(rNameVec[i].c_str());
					ImGui::Text(("Resource Count:" + std::to_string(desc.count)).c_str());
					for (uint j = 0; j < (uint)Graphic::J_GRAPHIC_BIND_TYPE::COUNT; ++j)
					{
						ImGui::Text(bNameVec[i].c_str());
						ImGui::Text(("View Count:" + std::to_string(desc.viewInfo[j].count)).c_str());
						ImGui::Text(("View Capacity:" + std::to_string(desc.viewInfo[j].capacity)).c_str());
						ImGui::Text(("View Offset:" + std::to_string(desc.viewInfo[j].offset)).c_str());
					}
					ImGui::Separator();
				}
			} 
		}
	}
}