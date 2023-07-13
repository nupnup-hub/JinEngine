#include"JWindowStateViewer.h"
#include"../JEditorWindow.h"
#include"../JEditorPageShareData.h"
#include"../../GuiLibEx/ImGuiEx/JImGuiImpl.h"

namespace JinEngine
{
	namespace Editor
	{
		void JWindowStateViewer::Initialize(const std::vector<JEditorWindow*>& newWndVec)
		{
			wndVec = newWndVec;
		}
		void JWindowStateViewer::Update()
		{
			JImGuiImpl::BeginWindow("WindowStateViewer##GraphicOptionSetting", GetOpenPtr(), ImGuiWindowFlags_NoDocking);
			for (const auto& data : wndVec)
			{
				JImGuiImpl::Text(data->GetName());
				ImGuiWindow* wnd = JImGuiImpl::GetGuiWindow(data->GetName());
				if (wnd == nullptr)
				{
					JImGuiImpl::Text("Closed");
					continue;
				}

				ImGuiDockNode* dock = wnd->DockNode;
				JImGuiImpl::Text("FocusOrder: " + std::to_string(wnd->FocusOrder));
				JImGuiImpl::Text("LastFrameJustFocused: " + std::to_string(wnd->LastFrameJustFocused));
				if (dock != nullptr)
					JImGuiImpl::Text("DockOrder: " + std::to_string(wnd->DockOrder));
				ImGui::Separator();
			}

			JImGuiImpl::EndWindow(); 
		}
	}
}