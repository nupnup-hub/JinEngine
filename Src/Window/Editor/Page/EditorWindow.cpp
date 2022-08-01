#include"EditorWindow.h"
#include"EditorAttribute.h"
#include"../ImGuiEx/ImGuiManager.h"
#include"../Event/EditorEventStruct.h"
#include"../Utility/EditorUtility.h"
#include"../../../Utility/JCommonUtility.h" 
#include<fstream>
 
namespace JinEngine
{
	EditorWindow::EditorWindow(std::unique_ptr<EditorAttribute> attribute, const size_t ownerPageGuid)
		:Editor(std::move(attribute)), ownerPageGuid(ownerPageGuid)
	{}
	EditorWindow::~EditorWindow() {}
	void EditorWindow::EnterWindow(EditorUtility* editorUtility)
	{
		std::string windowName = GetName();
		ImGui::Begin(windowName.c_str(), 0,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

		UpdateDocking(editorUtility);
	}
	void EditorWindow::UpdateWindow(EditorUtility* editorUtility)
	{
		std::string windowName = GetName();
		editorUtility->leftMouseClick = false;
		editorUtility->rightMouseClick = false;

		if (IsFocus() && IsActivated() && ImGui::IsMouseInWindow(ImGui::GetWindowPos(), ImGui::GetWindowSize()))
		{
			if (ImGui::IsMouseClicked(0))
			{ 
				editorUtility->leftMouseClick = true;
				std::unique_ptr<EditorMouseClickEvStruct> lclickEvStruct = std::make_unique<EditorMouseClickEvStruct>(windowName, 0);			
				//editorUtility->eventManager.NotifyEvent(GetGuid(), EDITOR_EVENT::MOUSE_CLICK, lclickEvStruct.get());
			}
			else if (ImGui::IsMouseClicked(1))
			{ 
				editorUtility->rightMouseClick = true;
				std::unique_ptr<EditorMouseClickEvStruct> rclickEvStruct = std::make_unique< EditorMouseClickEvStruct>(windowName, 1);
				//editorUtility->eventManager.NotifyEvent(GetGuid(), EDITOR_EVENT::MOUSE_CLICK, rclickEvStruct.get());
			}
		} 
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
		{ 
			if (!IsFocus())
			{
				std::unique_ptr<EditorFocusWindowEvStruct> focusEvStruct = std::make_unique<EditorFocusWindowEvStruct>(this, editorUtility);
				//editorUtility->eventManager.NotifyEvent(GetGuid(), EDITOR_EVENT::FOCUS_WINDOW, focusEvStruct.get());
			}
		}
	}
	void EditorWindow::UpdateDocking(EditorUtility* editorUtility)
	{
		ImGuiDockNode* dockNode = ImGui::GetWindowDockNode();
		if (dockNode != nullptr)
		{
			ImGuiID windowID = ImGui::GetCurrentWindow()->ID;
			ImGuiID selectedTabId = dockNode->SelectedTabId;
			if (IsActivated() && windowID != selectedTabId)
			{
				std::unique_ptr<EditorDeActWindowEvStruct> deActEvStruct = std::make_unique< EditorDeActWindowEvStruct>(GetName());
				//editorUtility->eventManager.NotifyEvent(GetGuid(), EDITOR_EVENT::DEACTIVATE_WINDOW, deActEvStruct.get());
			}
			else if (!IsActivated() && windowID == selectedTabId)
			{
				bool isFocus = false;
				if (dockNode->IsFocused)
					isFocus = true;
				std::unique_ptr<EditorActWindowEvStruct> actEvStruct = std::make_unique<EditorActWindowEvStruct>(GetName(), isFocus);
				//editorUtility->eventManager.NotifyEvent(GetGuid(), EDITOR_EVENT::ACTIVATE_WINDOW, actEvStruct.get());
			}	
			//forDebug
			/*ImGui::Text(("State: " + std::to_string(dockNode->State)).c_str());
			ImGui::Text(("IsFloatingNode: " + std::to_string(dockNode->IsFloatingNode())).c_str());
			if (dockNode->ParentNode != nullptr)
				ImGui::Text(("Parent: " + std::to_string(dockNode->ParentNode->ID)).c_str());*/
		}
	}
	void EditorWindow::CloseWindow()
	{
		ImGui::End();
	}
	void EditorWindow::StoreEditorWindow(std::wofstream& stream)
	{
		stream << L"Open: " << IsOpen() << '\n';
		stream << L"Front: " << IsFront() << '\n';
		stream << L"Activate: " << IsActivated() << '\n';
	}
	void EditorWindow::LoadEditorWindow(std::wifstream& stream)
	{
		std::wstring guide;
		bool isOpen;
		bool isFront;
		bool activated;

		stream >> guide; stream >> isOpen;
		stream >> guide; stream >> isFront;
		stream >> guide; stream >> activated;

		if (isOpen)
			SetOpen();
		else
			OffOpen();

		if (isFront)
			SetFront();
		else
			OffFront();
	}
	size_t EditorWindow::GetOwnerPageGuid()const noexcept
	{
		return ownerPageGuid;
	}
	void EditorWindow::SetButtonSelectColor()noexcept
	{ 
		ImGuiManager::SetColorToDeep(ImGuiCol_Button, 0.15f);
		ImGuiManager::SetColorToDeep(ImGuiCol_ButtonHovered, 0.15f);
		ImGuiManager::SetColorToDeep(ImGuiCol_ButtonActive, 0.15f);
	}
	void EditorWindow::SetButtonDefaultColor()noexcept
	{
		ImGuiManager::ReturnDefaultColor(ImGuiCol_Button);
		ImGuiManager::ReturnDefaultColor(ImGuiCol_ButtonHovered);
		ImGuiManager::ReturnDefaultColor(ImGuiCol_ButtonActive);
	}
	void EditorWindow::SetTreeNodeSelectColor()noexcept
	{
		ImGuiManager::SetColorToDeep(ImGuiCol_Header, 0.15f);
		ImGuiManager::SetColorToDeep(ImGuiCol_HeaderHovered, 0.15f);
		ImGuiManager::SetColorToDeep(ImGuiCol_HeaderActive, 0.15f);
	}
	void EditorWindow::SetTreeNodeDefaultColor()noexcept
	{
		ImGuiManager::ReturnDefaultColor(ImGuiCol_Header);
		ImGuiManager::ReturnDefaultColor(ImGuiCol_HeaderHovered);
		ImGuiManager::ReturnDefaultColor(ImGuiCol_HeaderActive);
	}
}