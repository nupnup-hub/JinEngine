#include"JGraphicResourceWatcher.h"
#include"../../JEditorAttribute.h"
#include"../../../Gui/JGui.h"
#include"../../../../Graphic/JGraphicPrivate.h"
#include"../../../../Graphic/GraphicResource/JGraphicResourceManager.h" 
#include"../../../../Graphic/GraphicResource/JGraphicResourceInfo.h"  

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
			EnterWindow(J_GUI_WINDOW_FLAG_NO_SCROLL_BAR);
			UpdateDocking();
			if (IsActivated())
			{
				UpdateMouseClick();
				using GraphicDebugInterface = Graphic::JGraphicPrivate::DebugInterface;
				Graphic::JGraphicResourceManager* grManager = GraphicDebugInterface::GetGraphicResourceManager();

				Core::JEnumInfo* rInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_RESOURCE_TYPE).name());
				Core::JEnumInfo* bInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_BIND_TYPE).name());
				auto rNameVec = rInfo->GetEnumNameVec();
				auto bNameVec = bInfo->GetEnumNameVec();
				 
				for (uint i = 0; i < (uint)Graphic::J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				{ 
					const Graphic::J_GRAPHIC_RESOURCE_TYPE rType = (Graphic::J_GRAPHIC_RESOURCE_TYPE)i;
					JGui::Text(rNameVec[i].c_str());
					JGui::Text(("Resource Count:" + std::to_string(grManager->GetResourceCount(rType))));
					for (uint j = 0; j < (uint)Graphic::J_GRAPHIC_BIND_TYPE::COUNT; ++j)
					{
						const Graphic::J_GRAPHIC_BIND_TYPE bType = (Graphic::J_GRAPHIC_BIND_TYPE)j;
						JGui::Text(bNameVec[j].c_str());
						JGui::Text(("View Count:" + std::to_string(grManager->GetViewCount(rType, bType))));
						JGui::Text(("View Capacity:" + std::to_string(grManager->GetViewCapacity(rType, bType))));
						JGui::Text(("View Offset:" + std::to_string(grManager->GetViewOffset(rType, bType))));
					}
					JGui::Separator();
				}
			} 
			CloseWindow();
		}
	}
}