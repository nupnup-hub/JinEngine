#include"JEditorPage.h" 
#include"JEditorWindow.h"
#include"JEditorAttribute.h" 
#include"JEditorPageShareData.h"
#include"../Menubar/JEditorMenuBar.h"
#include"../Event/JEditorEventStruct.h"
#include"../Event/JEditorEvent.h"
#include"../../Core/File/JFileIOHelper.h"
#include"../../Core/Reflection/JReflectionInfo.h"
#include"../../Object/Resource/Scene/JSceneManager.h"
#include"../../Object/Component/JComponent.h"
#include"../../Object/Resource/JResourceObject.h"
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/Scene/JScene.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Window/JWindows.h"
#include"../../Utility/JCommonUtility.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h" 
#include<fstream> 

namespace JinEngine
{
	namespace Editor
	{
		JEditorPage::WindowInitInfo::WindowInitInfo(const std::string name,
			float initWidthRate, float initHeightRate,
			float initPosXRate, float initPosYRate,
			bool isOpen, bool isLastAct)
			:name(name),
			initWidthRate(initWidthRate), initHeightRate(initHeightRate),
			initPosXRate(initPosXRate), initPosYRate(initPosYRate),
			isOpen(isOpen), isLastAct(isLastAct)
		{
		}
		std::string JEditorPage::WindowInitInfo::GetName()const noexcept
		{
			return name;
		}
		std::unique_ptr<JEditorAttribute> JEditorPage::WindowInitInfo::MakeAttribute()noexcept
		{
			return std::make_unique<JEditorAttribute>(initWidthRate, initHeightRate, initPosXRate, initPosYRate, isOpen, isLastAct);
		}

		JEditorPage::JEditorPage(const std::string name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_FLAG pageFlag)
			:JEditor(name, std::move(attribute)), pageFlag(pageFlag)
		{
			auto openEditorWindowLam = [](JEditorPage& page, const std::string windowName)
			{
				JEditorWindow* selectedWindow = page.FindEditorWindow(windowName);
				if (!selectedWindow->IsOpen())
				{
					page.AddEventNotification(*JEditorEvent::EvInterface(), page.GetGuid(), J_EDITOR_EVENT::ACTIVATE_WINDOW,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorActWindowEvStruct>(selectedWindow, page.GetPageType())));
				}
				else
				{
					page.AddEventNotification(*JEditorEvent::EvInterface(), page.GetGuid(), J_EDITOR_EVENT::CLOSE_WINDOW,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorCloseWindowEvStruct>(selectedWindow->GetName(), page.GetPageType())));
				}
			};

			auto openSimpleWindowLam = [](bool& isOpen) {isOpen = !isOpen;};
			openEditorWindowFunctor = std::make_unique<OpenEditorWindowF::Functor>(openEditorWindowLam);
			openSimpleWindowFunctor = std::make_unique<OpenSimpleWindowF::Functor>(openSimpleWindowLam);
		}
		JEditorPage::~JEditorPage()
		{}
		J_EDITOR_PAGE_FLAG JEditorPage::GetPageFlag()const noexcept
		{
			return pageFlag;
		}
		void JEditorPage::EnterPage(const int windowFlag)noexcept
		{
			if (Core::HasSQValueEnum(pageFlag, J_EDITOR_PAGE_SUPPORT_WINDOW_CLOSING))
			{
				bool res = ImGui::Begin(GetName().c_str(), &isPageOpen, (ImGuiWindowFlags)windowFlag);
				if (!res)
				{
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::CLOSE_PAGE,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorClosePageEvStruct>(GetPageType())));
				}
			}
			else
				ImGui::Begin(GetName().c_str(), 0, (ImGuiWindowFlags)windowFlag);
		}
		void JEditorPage::ClosePage()noexcept
		{
			ImGui::End();
			SetLastActivated(IsActivated());
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

			if (window->IsActivated())
				DeActivateWindow(window);

			window->SetClose();
			opendWindow.erase(opendWindow.begin() + windowIndex);
		}
		void JEditorPage::ActivateWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;

			if (!window->IsOpen())
				OpenWindow(window);

			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			window->Activate();
		}
		void JEditorPage::DeActivateWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;

			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			if (window->IsFocus())
				UnFocusWindow(window);
			window->DeActivate();
		}
		void JEditorPage::FocusWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;
 
			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			if (!window->IsActivated())
				ActivateWindow(window);
			window->SetFocus();
			if (focusWindow != nullptr)
				UnFocusWindow(focusWindow);
			focusWindow = window;
		}
		void JEditorPage::UnFocusWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;

			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			window->SetUnFocus();
			focusWindow = nullptr;
		}
		JEditorPage::OpenEditorWindowF::Functor* JEditorPage::GetOpEditorWindowFunctorPtr()noexcept
		{
			return openEditorWindowFunctor.get();
		}
		JEditorPage::OpenSimpleWindowF::Functor* JEditorPage::GetOpSimpleWindowFunctorPtr()noexcept
		{
			return openSimpleWindowFunctor.get();
		}
		void JEditorPage::UpdateDockSpace(const int dockspaceFlag)
		{
			if (Core::HasSQValueEnum(pageFlag, J_EDITOR_PAGE_SUPPORT_DOCK))
			{
				ImGuiID dockspace_id = ImGui::GetID(GetDockNodeName().c_str());
				ImGui::DockSpace(dockspace_id, ImGui::GetMainViewport()->WorkSize, dockspaceFlag);
			}
		}
		void JEditorPage::UpdateWindowMenuBar()
		{
			if (editorMenuBar != nullptr && editorMenuBar->UpdateMenuBar())
			{
				JMenuNode* selectedNode = editorMenuBar->GetSelectedNode();
				selectedNode->ExecuteBind();
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
		void JEditorPage::DoSetOpen()noexcept
		{
			JEditor::DoSetOpen();
			isPageOpen = true;
		}
		void JEditorPage::DoSetClose()noexcept
		{
			JEditor::DoSetClose();
			JEditorPageShareData::ClearPageData(GetPageType());
			isPageOpen = false;
		}
		void JEditorPage::DoActivate()noexcept
		{
			JEditor::DoActivate();
			const uint opendWindowCount = (uint)opendWindow.size();
			for (uint i = 0; i < opendWindowCount; ++i)
			{
				if (opendWindow[i]->IsLastActivated())
					opendWindow[i]->Activate();
			}
		}
		void JEditorPage::DoDeActivate()noexcept
		{
			JEditor::DoDeActivate();
			const uint opendWindowCount = (uint)opendWindow.size();
			for (uint i = 0; i < opendWindowCount; ++i)
				opendWindow[i]->DeActivate();
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
			JFileIOHelper::StoreJString(stream, L"PageName:", JCUtil::U8StrToWstr(GetName()));
			JFileIOHelper::StoreAtomicData(stream, L"PageGuid:", GetGuid());
			JFileIOHelper::StoreAtomicData(stream, L"Open:", IsOpen());
			JFileIOHelper::StoreAtomicData(stream, L"Activate:", IsActivated());
			JFileIOHelper::StoreAtomicData(stream, L"Focus:", IsFocus());

			JFileIOHelper::StoreHasObjectIden(stream, JEditorPageShareData::GetOpendPageData(GetPageType()).openSelected.Get());
			JFileIOHelper::StoreHasObjectIden(stream, JEditorPageShareData::GetSelectedObj(GetPageType()).Get());
			
			bool hasFocusWindow = focusWindow != nullptr;
			JFileIOHelper::StoreAtomicData(stream, L"HasFocusWindow:", hasFocusWindow);
			if (hasFocusWindow)
				JFileIOHelper::StoreJString(stream, L"FocusWindowName:", JCUtil::U8StrToWstr(focusWindow->GetName()));
			else
				JFileIOHelper::StoreJString(stream, L"FocusWindowName:", L"None");


			JFileIOHelper::StoreAtomicData(stream, L"WindowCount:", (uint)windows.size());
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
			bool active;
			bool isFocus;
			bool hasFocusWindow;
			std::wstring focusWindowName;
			int windowCount;

			JFileIOHelper::LoadJString(stream, name);
			JFileIOHelper::LoadAtomicData(stream, pageGuid);
			JFileIOHelper::LoadAtomicData(stream, isOpen);
			JFileIOHelper::LoadAtomicData(stream, active);
			JFileIOHelper::LoadAtomicData(stream, isFocus);

			Core::JIdentifier* openObj = JFileIOHelper::LoadHasObjectIden(stream);
			Core::JIdentifier* selectObj = JFileIOHelper::LoadHasObjectIden(stream);
			JFileIOHelper::LoadAtomicData(stream, hasFocusWindow);
			JFileIOHelper::LoadJString(stream, focusWindowName);
			JFileIOHelper::LoadAtomicData(stream, windowCount);

			if (isOpen)
			{
				bool reqOpenObj = JEditorPageShareData::HasValidOpenPageData(GetPageType());
				bool hasOpenObj = openObj != nullptr && openObj->GetTypeInfo().IsChildOf(JObject::StaticTypeInfo());
				if (hasOpenObj)
				{
					auto userObj = Core::JUserPtr<JObject>::ConvertChildType(Core::GetUserPtr(openObj->TypeName(), openObj->GetGuid()));
					AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::OPEN_PAGE,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorOpenPageEvStruct>(GetPageType(), userObj)));
				}
				else if (reqOpenObj)
				{
					AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::OPEN_PAGE,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorOpenPageEvStruct>(GetPageType(), Core::JUserPtr<JObject>())));
				}
			}
			if (active)
			{
				AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::ACTIVATE_PAGE,
					JEditorEvent::RegisterEvStruct(std::make_unique<JEditorActPageEvStruct>(this)));
			}
			if (isFocus)
			{
				AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::FOCUS_PAGE,
					JEditorEvent::RegisterEvStruct(std::make_unique<JEditorFocusPageEvStruct>(this)));
			}

			if (selectObj != nullptr && selectObj->GetTypeInfo().IsChildOf(JObject::StaticTypeInfo()))
			{
				auto userObj = Core::JUserPtr<JObject>::ConvertChildType(Core::GetUserPtr(selectObj->TypeName(), selectObj->GetGuid()));
				AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::SELECT_OBJECT,
					JEditorEvent::RegisterEvStruct(std::make_unique<JEditorSelectObjectEvStruct>(GetPageType(), userObj)));
			}

			for (uint i = 0; i < windowCount; ++i)
				windows[i]->LoadEditorWindow(stream);
		}
	}
}