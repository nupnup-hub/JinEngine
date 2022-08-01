#include"EditorWindowPage.h"
#include"EditorWindow.h"
#include"EditorAttribute.h"
#include"../Utility/EditorUtility.h"
#include"../Menubar/EditorMenuBar.h"
#include"../Event/EditorEventStruct.h"
#include"../../../Object/Resource/JResourceManager.h"
#include"../../../Utility/JCommonUtility.h"
#include"../ImGuiEx/ImGuiManager.h" 
#include<fstream>

namespace JinEngine
{
	EditorWindowPage::EditorWindowPage(std::unique_ptr<EditorAttribute> attribute)
		: Editor(std::move(attribute))
	{
		dockSpaceName = GetName() + "DockSpace";
	}
	EditorWindowPage::~EditorWindowPage() {}
	bool EditorWindowPage::Activate(EditorUtility* editorUtility)
	{
		if (Editor::Activate(editorUtility))
		{
			this->AddEventListener(*editorUtility->EvInterface(), GetGuid(), EDITOR_EVENT::FOCUS_WINDOW);

			const uint opendWindowCount = (uint)opendWindow.size();
			for (uint i = 0; i < opendWindowCount; ++i)
				opendWindow[i]->Activate(editorUtility);
			pageOpen = true;
			return true;
		}
		else
			return false;
	}
	bool EditorWindowPage::DeActivate(EditorUtility* editorUtility)
	{
		if (Editor::DeActivate(editorUtility))
		{
			this->EraseListener(*editorUtility->EvInterface(), GetGuid());

			const uint opendWindowCount = (uint)opendWindow.size();
			for (uint i = 0; i < opendWindowCount; ++i)
				opendWindow[i]->DeActivate(editorUtility);
			pageOpen = false;
			return true;
		}
		else
			return false;
	}
	void EditorWindowPage::StorePage(std::wofstream& stream)
	{
		const uint editorWindowCount = (uint)windows.size();
		const uint opendWindowCount = (uint)opendWindow.size();

		stream << L"PageName: " << GetWName() << '\n';
		stream << L"PageGuid: " << GetGuid() << '\n';
		stream << L"Open: " << IsOpen() << '\n';
		stream << L"Front: " << IsFront() << '\n';
		stream << L"Activate: " << IsActivated() << '\n';
		stream << L"WindowCount: " << editorWindowCount << '\n';
		stream << L"OpendWindowCount: " << opendWindowCount << '\n';

		for (uint i = 0; i < editorWindowCount; ++i)
		{
			stream << L"WindowName: " << windows[i]->GetWName() << '\n';
			stream << L"WindowGuid: " << windows[i]->GetGuid() << '\n';
		}
		for (uint i = 0; i < editorWindowCount; ++i)
			windows[i]->StoreEditorWindow(stream);
	}
	void EditorWindowPage::LoadPage(std::wifstream& stream,
		std::vector<EditorWindow*>& allEditorWindows,
		std::vector<EditorWindowPage*>& opendEditorPage,
		EditorUtility* editorUtility)
	{
		std::wstring guide;
		std::wstring name;
		size_t pageGuid;
		bool isOpen;
		bool isFront;
		bool active;
		int windowCount;
		int opendWindowCount;

		stream >> guide; stream >> name;
		stream >> guide; stream >> pageGuid;
		stream >> guide; stream >> isOpen;
		stream >> guide; stream >> isFront;
		stream >> guide; stream >> active;
		stream >> guide; stream >> windowCount;
		stream >> guide; stream >> opendWindowCount;

		if (isOpen)
			SetOpen();
		else
			OffOpen();
		if (isFront)
			SetFront();
		else
			OffFront();

		//windows.clear();
		//windows.reserve(windowCount);
		opendWindow.clear();
		opendWindow.reserve(opendWindowCount);

		std::vector<size_t> storedWindowGuid;
		for (int i = 0; i < windowCount; ++i)
		{
			size_t windowGuid;
			stream >> guide; stream >> name;
			stream >> guide; stream >> windowGuid;
			storedWindowGuid.push_back(windowGuid);
		}

		const uint sotreWindowCount = (uint)storedWindowGuid.size();
		const uint allWindowCount = (uint)allEditorWindows.size();
		for (uint i = 0; i < sotreWindowCount; ++i)
		{ 
			for (uint j = 0; j < allWindowCount; ++j)
			{
				if (allEditorWindows[j]->GetGuid() == storedWindowGuid[i])
				{ 
					allEditorWindows[j]->LoadEditorWindow(stream);
					if (allEditorWindows[j]->IsOpen())
						opendWindow.push_back(allEditorWindows[j]);
					break;
				}
			}
		}

		if (IsOpen() && IsFront())
		{
			opendEditorPage.push_back(this);
			if (active)
				Activate(editorUtility);
			else
				OffActivattion();
		}
	}
	void EditorWindowPage::EnterPage(const int windowFlag, const int dockspaceFlag, bool isMainPage)noexcept
	{
		if (ImGuiManager::IsFullScreen())
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		}
		if (!ImGuiManager::IsWindowPadding())
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		 
		if (isMainPage)
			ImGui::Begin(GetName().c_str(), 0, (ImGuiWindowFlags)windowFlag);
		else
			ImGui::Begin(GetName().c_str(), &pageOpen, (ImGuiWindowFlags)windowFlag);

		ImGuiManager::PushFont(2);

		if (!ImGuiManager::IsWindowPadding())
			ImGui::PopStyleVar();
		if (ImGuiManager::IsFullScreen())
			ImGui::PopStyleVar(2);

		ImGuiID dockspace_id = ImGui::GetID((GetName() + "DockSpace").c_str());
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), (ImGuiDockNodeFlags)dockspaceFlag);
	}
	void EditorWindowPage::ClosePage(EditorUtility* editorUtility)noexcept
	{
		ImGuiManager::PopFont();
		ImGui::End();
		if (!pageOpen)
		{
			std::unique_ptr<EditorClosePageEvStruct> closePageEvStruct =
				std::make_unique<EditorClosePageEvStruct>(GetName());

			this->NotifyEvent(*editorUtility->EvInterface(), GetGuid(), EDITOR_EVENT::CLOSE_PAGE, closePageEvStruct.get());
		}
		//if(pageClose)
	}
	void EditorWindowPage::OpenWindow(EditorWindow* window, EditorUtility* editorUtility)noexcept
	{
		if (window == nullptr && window->IsOpen())
			return;

		bool hasWindow = IsWindowInVector(window, windows.cbegin(), (uint)windows.size());
		if (!hasWindow)
			return;

		bool opendOverlap = IsWindowInVector(window, opendWindow.cbegin(), (uint)opendWindow.size());
		if (opendOverlap)
			return;

		opendWindow.push_back(window);
		window->Open(editorUtility);
		window->SetFront(editorUtility);
		window->Activate(editorUtility);
	}
	void EditorWindowPage::CloseWindow(EditorWindow* window, EditorUtility* editorUtility)noexcept
	{
		if (window == nullptr && !window->IsOpen())
			return;

		bool hasWindow = IsWindowInVector(window, windows.cbegin(), (uint)windows.size());
		if (!hasWindow)
			return;

		int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
		if (windowIndex == -1)
			return;

		if (window->IsActivated())
		{
			window->DeActivate(editorUtility);
			window->OffFront(editorUtility);
			window->Close(editorUtility);
		}
		else
			window->Close(editorUtility);

		opendWindow.erase(opendWindow.begin() + windowIndex);
	}
	uint EditorWindowPage::GetMemeberCount()const noexcept
	{
		return (uint)windows.size();
	}
	void EditorWindowPage::UpdateWindowMenuBar(EditorUtility* editorUtility)
	{
		if (editorMenuBar != nullptr && editorMenuBar->UpdateMenuBar(editorUtility))
		{
			MenuNode* selectedNode = editorMenuBar->GetSelectedNode();
			EditorWindow* selectedWindow = FindEditorWindow(selectedNode->GetWindowName());
			if (!selectedWindow->IsOpen())
			{
				std::unique_ptr<EditorOpenWindowEvStruct> openWindowEvStruct =
					std::make_unique<EditorOpenWindowEvStruct>(selectedWindow->GetName(), true);
				this->NotifyEvent(*editorUtility->EvInterface(), GetGuid(), EDITOR_EVENT::OPEN_WINDOW, openWindowEvStruct.get());
			}
			else
			{
				std::unique_ptr<EditorCloseWindowEvStruct> closeWindowEvStruct =
					std::make_unique<EditorCloseWindowEvStruct>(selectedWindow->GetName());
				this->NotifyEvent(*editorUtility->EvInterface(), GetGuid(), EDITOR_EVENT::CLOSE_PAGE, closeWindowEvStruct.get());
			}
		}
	}
	void EditorWindowPage::PrintOpenWindowState()
	{
		const uint openWindowCount = (uint)opendWindow.size();
		ImGui::SetCursorPos(ImVec2(50, 50));
		ImGui::Begin("PrintOpenWindowState");
		for (uint i = 0; i < openWindowCount; ++i)
		{ 
			ImGui::Text(opendWindow[i]->GetName().c_str());
			if (opendWindow[i]->IsOpen())
				ImGui::Text("Open");
			else
				ImGui::Text("Close");

			if (opendWindow[i]->IsFront())
				ImGui::Text("Front");
			else
				ImGui::Text("Back");

			if (opendWindow[i]->IsActivated())
				ImGui::Text("Act");
			else
				ImGui::Text("DeAct");

			if (opendWindow[i]->IsFocus())
				ImGui::Text("Focus On");
			else
				ImGui::Text("Focus Off");		 
		}
		ImGui::End();
	}
	EditorWindow* EditorWindowPage::FindEditorWindow(const std::string& windowName)const noexcept
	{
		const uint windowCount = (uint)windows.size();
		for (uint i = 0; i < windowCount; ++i)
		{
			if (windows[i]->GetName() == windowName)
				return windows[i];
		}
		return nullptr;
	}
	bool EditorWindowPage::IsWindowInVector(EditorWindow* window,
		std::vector<EditorWindow*>::const_iterator st,
		const uint count)const noexcept
	{
		const size_t tarGuid = window->GetGuid();
		for (uint i = 0; i < count; ++i)
		{
			if ((*(st + i))->GetGuid() == tarGuid)
				return true;
		}
		return false;
	}
	int EditorWindowPage::FindWindowIndex(EditorWindow* window,
		std::vector<EditorWindow*>::const_iterator st,
		const uint count)const noexcept
	{
		int tarIndex = -1;
		const size_t tarGuid = window->GetGuid();
		for (uint i = 0; i < count; ++i)
		{
			if ((*(st + i))->GetGuid() == tarGuid)
			{
				tarIndex = (int)i;
				return tarIndex;
			}
		}
		return tarIndex;
	}
	void EditorWindowPage::OnEvent(const size_t& senderGuid, const EDITOR_EVENT& eventType, EditorEventStruct* eventStruct)
	{
		if (senderGuid == GetGuid())
			return;

		if (eventType == EDITOR_EVENT::FOCUS_WINDOW)
		{
			EditorFocusWindowEvStruct* focusEvStruct = dynamic_cast<EditorFocusWindowEvStruct*>(eventStruct);
			if (focusEvStruct->PassDefectInspection())
			{
				if (focusWindow != nullptr)
					focusWindow->OffFocus(focusEvStruct->editorUtility);
				focusWindow = focusEvStruct->focusWindow;
				focusWindow->OnFocus(focusEvStruct->editorUtility);
			}
		}
	}
}