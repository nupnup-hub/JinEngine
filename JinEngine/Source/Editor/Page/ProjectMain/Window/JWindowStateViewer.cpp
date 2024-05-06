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


#include"JWindowStateViewer.h" 
#include"../../JEditorAttribute.h"
#include"../../JEditorPageShareData.h"
#include"../../../Gui/JGui.h"

namespace JinEngine
{
	namespace Editor
	{
		JWindowStateViewer::JWindowStateViewer(const std::string name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), ownerPageType, windowFlag)
		{}

		J_EDITOR_WINDOW_TYPE JWindowStateViewer::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::GRAPHIC_OPTION;
		}
		void JWindowStateViewer::Initialize(const std::vector<JEditorWindow*>& newWndVec)
		{
			wndVec = newWndVec;
			/*
			* if (containStateViewer)
			{ 
				bool(*equalPtr)(JEditorWindow*, size_t) = [](JEditorWindow* w, size_t guid) {return w->GetGuid() == guid; };
				const int index = JCUtil::GetIndex(wndVec, equalPtr, GetGuid());			 
				if(index == invalidIndex)
					wndVec.push_back(this);
			}
			*/
		}
		void JWindowStateViewer::UpdateWindow()
		{
			EnterWindow(J_GUI_WINDOW_FLAG_NO_COLLAPSE);
			if (IsActivated())
			{
				UpdateMouseClick();
				for (const auto& data : wndVec)
				{
					JGui::Text(data->GetName());
					JGuiWindowInfo wInfo;
					if (!JGui::GetWindowInfo(data->GetName(), wInfo))
					{
						JGui::Text("Closed");
						continue;
					}
					JGui::Text("FocusOrder: " + std::to_string(wInfo.focusOrder));
					JGui::Text("LastFrameJustFocused: " + std::to_string(wInfo.isLastFrameFocused));
					if (wInfo.hasDockNode)
						JGui::Text("DockOrder: " + std::to_string(wInfo.dockOrder));
					JGui::Separator();
				}
			}
			CloseWindow();
		}
	}
}