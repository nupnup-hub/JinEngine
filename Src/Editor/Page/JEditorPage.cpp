#include"JEditorPage.h"
#include"JEditorWindow.h"
#include"JEditorAttribute.h" 
#include"../Menubar/JEditorMenuBar.h"
#include"../Event/JEditorEventStruct.h"
#include"../../Core/Reflection/JReflectionInfo.h"
#include"../../Object/Resource/Scene/JSceneManager.h"
#include"../../Object/Component/JComponent.h"
#include"../../Object/Resource/JResourceObject.h"
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/Scene/JScene.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Utility/JCommonUtility.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h" 
#include<fstream>

namespace JinEngine
{
	namespace Editor
	{
		void JEditorPage::EnterPage(const int windowFlag, const int dockspaceFlag, bool isMainPage)noexcept
		{
			if (JImGuiImpl::IsFullScreen())
			{
				const ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos(viewport->WorkPos);
				ImGui::SetNextWindowSize(viewport->WorkSize);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			}
			if (!JImGuiImpl::IsWindowPadding())
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			if (isMainPage)
				ImGui::Begin(GetName().c_str(), 0, (ImGuiWindowFlags)windowFlag);
			else
				ImGui::Begin(GetName().c_str(), &pageOpen, (ImGuiWindowFlags)windowFlag);

			JImGuiImpl::PushFont();

			if (!JImGuiImpl::IsWindowPadding())
				ImGui::PopStyleVar();
			if (JImGuiImpl::IsFullScreen())
				ImGui::PopStyleVar(2);

			ImGuiID dockspace_id = ImGui::GetID((GetName() + "DockSpace").c_str());
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), (ImGuiDockNodeFlags)dockspaceFlag);
		}
		void JEditorPage::ClosePage()noexcept
		{
			JImGuiImpl::PopFont();
			ImGui::End();
		}
		void JEditorPage::DoSetOpen()noexcept
		{
			JEditor::DoSetOpen();
		}
		void JEditorPage::DoOffOpen()noexcept
		{
			JEditor::DoOffOpen();
			JImGuiImpl::ClearPageData(GetPageType());
		}
		void JEditorPage::OpenWindow(const std::string& windowname)noexcept
		{
			OpenWindow(FindEditorWindow(windowname));
		}
		void JEditorPage::OpenWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;

			bool hasWindow = IsWindowInVector(window, windows.cbegin(), (uint)windows.size());
			if (!hasWindow)
				return;

			opendWindow.push_back(window);
			window->SetOpen();
		}
		void JEditorPage::CloseWindow(const std::string& windowname)noexcept
		{
			CloseWindow(FindEditorWindow(windowname));
		}
		void JEditorPage::CloseWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;

			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			window->DeActivate();
			window->OffFront();
			window->OffOpen(); 
			opendWindow.erase(opendWindow.begin() + windowIndex);
		}
		void JEditorPage::FrontWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;

			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			window->SetFront();
		}
		void JEditorPage::BackWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;

			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			window->DeActivate();
			window->OffFront();
		}
		void JEditorPage::ActivateWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;

			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			if (!window->IsFront())
				window->SetFront();
			window->Activate();
		}
		void JEditorPage::DeActivateWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;

			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			window->DeActivate();
		}
		void JEditorPage::FocusWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;

			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			if (!window->IsFront())
				window->SetFront();
			if (!window->IsActivated())
				window->Activate();

			window->SetFocus();
		}
		void JEditorPage::UnFocusWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;

			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			window->OffFocus();
		}
		void JEditorPage::UpdateWindowMenuBar()
		{
			if (editorMenuBar != nullptr && editorMenuBar->UpdateMenuBar())
			{
				JMenuNode* selectedNode = editorMenuBar->GetSelectedNode();
				JEditorWindow* selectedWindow = FindEditorWindow(selectedNode->GetWindowName());
				if (!selectedWindow->IsOpen())
				{
					JEditorOpenWindowEvStruct openWindowEvStruct{ selectedWindow->GetName(), GetPageType()};
					NotifyEvent(*JImGuiImpl::EvInterface(), GetGuid(), J_EDITOR_EVENT::ACTIVATE_WINDOW, &openWindowEvStruct);					
				}
				else
				{
					JEditorCloseWindowEvStruct closeWindowEvStruct{ selectedWindow->GetName(),  GetPageType()};
					NotifyEvent(*JImGuiImpl::EvInterface(), GetGuid(), J_EDITOR_EVENT::CLOSE_WINDOW, &closeWindowEvStruct);
				}
			}
		}
		void JEditorPage::PrintOpenWindowState()
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
					ImGui::Text("Activate");
				else
					ImGui::Text("DeActivate");

				if (opendWindow[i]->IsFocus())
					ImGui::Text("Focus On");
				else
					ImGui::Text("Focus Off");
			}
			ImGui::End();
		}
		void JEditorPage::DoActivate()
		{ 
			const uint opendWindowCount = (uint)opendWindow.size();
			for (uint i = 0; i < opendWindowCount; ++i)
			{
				if (opendWindow[i]->IsFront())
					opendWindow[i]->Activate();
			}
			pageOpen = true;
		}
		void JEditorPage::DoDeActivate()
		{ 
			const uint opendWindowCount = (uint)opendWindow.size();
			for (uint i = 0; i < opendWindowCount; ++i)
			{
				if (opendWindow[i]->IsFront())
					opendWindow[i]->DeActivate();
			}
			pageOpen = false; 
		}
		JEditorWindow* JEditorPage::FindEditorWindow(const std::string& windowName)const noexcept
		{
			const uint windowCount = (uint)windows.size();
			for (uint i = 0; i < windowCount; ++i)
			{
				if (windows[i]->GetName() == windowName)
					return windows[i];
			}
			return nullptr;
		}
		bool JEditorPage::IsWindowInVector(JEditorWindow* window, std::vector<JEditorWindow*>::const_iterator st, const uint count)const noexcept
		{
			const size_t tarGuid = window->GetGuid();
			for (uint i = 0; i < count; ++i)
			{
				if ((*(st + i))->GetGuid() == tarGuid)
					return true;
			}
			return false;
		}
		int JEditorPage::FindWindowIndex(JEditorWindow* window, std::vector<JEditorWindow*>::const_iterator st, const uint count)const noexcept
		{
			const size_t tarGuid = window->GetGuid();
			for (uint i = 0; i < count; ++i)
			{
				if ((*(st + i))->GetGuid() == tarGuid)
					return i;
			}
			return -1;
		}
		void JEditorPage::StorePage(std::wofstream& stream)
		{
			stream << L"PageName: " << GetWName() << '\n';
			stream << L"PageGuid: " << GetGuid() << '\n';
			stream << L"Open: " << IsOpen() << '\n';
			stream << L"Front: " << IsFront() << '\n';
			stream << L"Activate: " << IsActivated() << '\n';
			stream << L"Focus: " << IsFocus() << '\n';

			JObject* sObj = JImGuiImpl::GetSelectedObj(GetPageType());
			if (sObj != nullptr)
			{
				const J_OBJECT_TYPE objType = sObj->GetObjectType();
				stream << L"HasSelectedObj: " << true << '\n';
				stream << L"Guid: " << sObj->GetGuid() << '\n';
				stream << L"Type: " << (int)objType << '\n';

				if (objType == J_OBJECT_TYPE::RESOURCE_OBJECT)
					stream << L"SubType: " << (int)static_cast<JResourceObject*>(sObj)->GetResourceType() << '\n';
				else
					stream << L"SubType: " << 0 << '\n';
			}
			else
			{
				stream << L"HasSelectedObj: " << false << '\n';
				stream << L"Guid: " << 0 << '\n';
				stream << L"Type: " << 0 << '\n';
				stream << L"SubType: " << 0 << '\n';
			}

			const uint editorWindowCount = (uint)windows.size();
			for (uint i = 0; i < editorWindowCount; ++i)
				windows[i]->StoreEditorWindow(stream);
		}
		void JEditorPage::LoadPage(std::wifstream& stream)
		{
			std::wstring guide;
			std::wstring name;
			size_t pageGuid;
			bool isOpen;
			bool isFront;
			bool active;
			bool isFocus;

			bool hasSelectedObj;
			size_t sObjGuid; 
			int sObjType;
			int sObjSubType; 

			stream >> guide; stream >> name;
			stream >> guide; stream >> pageGuid;
			stream >> guide; stream >> isOpen;
			stream >> guide; stream >> isFront;
			stream >> guide; stream >> active;
			stream >> guide; stream >> isFocus;

			stream >> guide; stream >> hasSelectedObj;
			stream >> guide; stream >> sObjGuid;
			stream >> guide; stream >> sObjType;
			stream >> guide; stream >> sObjSubType;

			JObject* selectedObj = nullptr;
			if (hasSelectedObj)
			{
				if (sObjType == (int)J_OBJECT_TYPE::RESOURCE_OBJECT)
					selectedObj = JResourceManager::Instance().GetResource((J_RESOURCE_TYPE)sObjSubType, sObjGuid);
				if (sObjType == (int)J_OBJECT_TYPE::GAME_OBJECT)
					selectedObj = Core::JReflectionInfo::Instance().GetTypeInfo(JGameObject::TypeName())->GetInstance(sObjGuid);
			}
			if (isOpen)
			{
				JEditorOpenPageEvStruct evStruct{selectedObj->TypeName(), selectedObj->GetGuid(), GetPageType() };
				AddEventNotification(*JImGuiImpl::EvInterface(), GetGuid(), J_EDITOR_EVENT::OPEN_PAGE, &evStruct);
			}
			if (isFront)
			{
				JEditorFrontPageEvStruct evStruct{ this };
				AddEventNotification(*JImGuiImpl::EvInterface(), GetGuid(), J_EDITOR_EVENT::FRONT_PAGE, &evStruct);
			}
			if (active)
			{
				JEditorActPageEvStruct evStruct{ this };
				AddEventNotification(*JImGuiImpl::EvInterface(), GetGuid(), J_EDITOR_EVENT::ACTIVATE_PAGE, &evStruct);
			}
			if (isFocus)
			{
				JEditorFocusPageEvStruct evStruct{ this };
				AddEventNotification(*JImGuiImpl::EvInterface(), GetGuid(), J_EDITOR_EVENT::FOCUS_PAGE, &evStruct);
			} 
			 
			const uint windowCount = (uint)windows.size();
			for (uint i = 0; i < windowCount; ++i)
				windows[i]->LoadEditorWindow(stream);		
		}
		JEditorPage::JEditorPage(std::unique_ptr<JEditorAttribute> attribute, bool hasOpenInitObjType)
			:JEditor(std::move(attribute))
		{
			dockSpaceName = GetName() + "DockSpace";
			JImGuiImpl::RegisterPage(GetPageType(), hasOpenInitObjType);
		}
		JEditorPage::~JEditorPage()
		{
			JImGuiImpl::UnRegisterPage(GetPageType());
		}
	}
}