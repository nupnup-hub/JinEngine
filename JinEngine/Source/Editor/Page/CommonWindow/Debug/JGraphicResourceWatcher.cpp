/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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