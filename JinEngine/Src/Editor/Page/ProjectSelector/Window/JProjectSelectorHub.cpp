#include"JProjectSelectorHub.h"
#include"../../JEditorAttribute.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Application/JApplicationVariable.h" 
#include"../../../../Object/Resource/JResourceManager.h"
#include"../../../../Object/Resource/Texture/JTexture.h"
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../Window/JWindows.h"

namespace JinEngine
{
	namespace Editor
	{
		JProjectSelectorHub::JProjectSelectorHub(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType)
			:JEditorWindow(name, std::move(attribute), ownerPageType)
		{
			JResourceUserInterface::AddEventListener(*JResourceManager::Instance().EvInterface(), GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
			JApplicationProject::LoadProjectList();
		}
		JProjectSelectorHub::~JProjectSelectorHub()
		{
			JResourceUserInterface::RemoveListener(*JResourceManager::Instance().EvInterface(), GetGuid());
			JApplicationProject::StoreProjectList();
		}
		J_EDITOR_WINDOW_TYPE JProjectSelectorHub::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::PROJECT_SELECTOR_HUB;
		}
		void JProjectSelectorHub::UpdateWindow()
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowPos(viewport->WorkPos);

			int flag = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
			EnterWindow(flag);
			if (IsActivated())
			{
				//JImGuiImpl::AddImage(*backgroundTexture.Get(), viewport->WorkPos, viewport->WorkSize, false, IM_COL32(255, 255, 255, 50));
				JImGuiImpl::PushFont();
				TitleOnScreen();
				MenuListOnScreen();
				ProjectListOnScreen();
				JImGuiImpl::PopFont();
			}
			CloseWindow();
		}
		void JProjectSelectorHub::TitleOnScreen()
		{
			ImGui::SetWindowFontScale(0.85f);
			JVector2<int> clientSize = JImGuiImpl::GetClientWindowSize();
			JImGuiImpl::BeginChildWindow("Title##" + GetName(), JVector2<float>(clientSize.x, clientSize.y * 0.1f), true, 0);
			ImGui::SetCursorPos(ImVec2(clientSize.x * 0.05f, clientSize.y * 0.015f));

			JImGuiImpl::Text("JinEngine");
			JImGuiImpl::EndChildWindow();
			ImGui::SetWindowFontScale(1);
		}
		void JProjectSelectorHub::MenuListOnScreen()
		{
			ImGui::SetWindowFontScale(0.85f);
			JVector2<int> clientSize = JImGuiImpl::GetClientWindowSize();
			optionListCusorY = ImGui::GetCursorPosY();
			JImGuiImpl::BeginChildWindow("Option List##" + GetName(), JVector2<float>(clientSize.x * 0.2f, clientSize.y * 0.9f), true, 0);
			if (JImGuiImpl::Button("New Project##" + GetName(), JVector2<float>(clientSize.x * 0.2f, clientSize.y * 0.1f)))
			{
				versionIndex = 0;
				menuListValues.newProjectButton = !menuListValues.newProjectButton;
				if (menuListValues.newProjectButton)
					menuListValues.OpenCreateProjectMenu();
			}
			if (JImGuiImpl::Button("Load Project##" + GetName(), JVector2<float>(clientSize.x * 0.2f, clientSize.y * 0.1f)))
			{

			}
			JImGuiImpl::EndChildWindow();
			ImGui::SetWindowFontScale(1);

			if (menuListValues.newProjectButton)
				CreateNewProjectOnScreen();
		}
		void JProjectSelectorHub::ProjectListOnScreen()
		{
			JVector2<int> clientSize = JImGuiImpl::GetClientWindowSize();

			ImGui::SetCursorPos(ImVec2(clientSize.x * 0.21f, optionListCusorY));
			JImGuiImpl::BeginChildWindow("Project List##" + GetName(), JVector2<float>(clientSize.x * 0.78f, clientSize.y * 0.9f), true, 0);

			float titleTextYOffset = (clientSize.y * 0.13f) * 0.1f;
			float pathTextYOffset = (clientSize.y * 0.13f) * 0.65f;

			const uint projectListCount = JApplicationProject::GetProjectInfoCount();
			for (int i = 0; i < projectListCount; ++i)
			{
				JApplicationProject::JProjectInfo* info = JApplicationProject::GetProjectInfo(i);
				ImGui::SetCursorPosY(JWindow::Instance().GetClientHeight() * 0.13f * i);
				if (JImGuiImpl::Button(JCUtil::WstrToU8Str(L"##" + info->GetName()).c_str(), JVector2<float>(clientSize.x * 0.78f, clientSize.y * 0.125f)))
				{ 
					JApplicationProject::SetNextProjectInfo(std::make_unique<JApplicationProject::JProjectInfo>(info->GetName(), info->GetPath(), info->GetVersion()));
					if (!JApplicationProject::StartNewProject())
						MessageBox(0, L"StartNewProject Fail", 0, 0);
				}
				ImGui::SetWindowFontScale(0.85f);
				ImGui::SetCursorPos(ImVec2(clientSize.x * 0.025f, clientSize.y * 0.13f * i + titleTextYOffset));
				JImGuiImpl::Text((JCUtil::WstrToU8Str(info->GetName())));
				ImGui::SetWindowFontScale(0.5f);
				ImGui::SetCursorPos(ImVec2(clientSize.x * 0.025f, clientSize.y * 0.13f * i + pathTextYOffset));
				JImGuiImpl::Text((JCUtil::WstrToU8Str(info->GetPath())));
			}
			ImGui::SetWindowFontScale(1);
			JImGuiImpl::EndChildWindow();
		}
		void JProjectSelectorHub::CreateNewProjectOnScreen()
		{
			std::wstring dirPath;
			if (menuListValues.newProjectWinow)
			{
				JImGuiImpl::BeginWindow("Create New Project##" + GetName(), &menuListValues.newProjectWinow,
					ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoCollapse);
				if (!ImGui::IsWindowFocused())
					ImGui::FocusWindow(ImGui::GetCurrentWindow());

				ImGui::SetWindowSize(ImVec2(JWindow::Instance().GetClientWidth() * 0.7f, JWindow::Instance().GetClientHeight() * 0.6f));
				ImGui::SetWindowFontScale(0.6f);
				//ImGui::SetCursorPosY(JWindow::Instance().GetClientHeight() * 0.4f);
				JImGuiImpl::Text("Project Name: ");
				JImGuiImpl::InputText("##Project Name", &menuListValues.newProjectName[0], menuListValues.maxNameRange);

				JImGuiImpl::Text("Folder: ");
				JImGuiImpl::InputText("##Folder Path", &menuListValues.newProjectPath[0], menuListValues.maxPathRange, ImGuiInputTextFlags_ReadOnly);

				if (JImGuiImpl::ImageButton(*serachIconTexture.Get(), JVector2<float> {25, 25}))
				{
					std::wstring dirPath;
					if (JWindow::Instance().SelectDirectory(dirPath, L"please, select project parent directory") && JWindow::Instance().HasStorageSpace(dirPath, necessaryCapacityMB))
						menuListValues.newProjectPath = JCUtil::WstrToU8Str(dirPath);
				}

				std::vector<std::string> version = JApplicationVariable::GetAppVersion();
				const uint versionCount = (uint)version.size();

				JImGuiImpl::Text("Version: ");
				if (JImGuiImpl::BeginCombo("##ApplicationVersion", version[0].c_str()))
				{
					for (uint i = 0; i < versionCount; i++)
					{
						bool isSelected = (versionIndex == i);
						if (JImGuiImpl::Selectable(version[i], &isSelected))
							versionIndex = i;

						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (JImGuiImpl::Button("Create Proejct"))
				{
					;
					if (!menuListValues.newProjectName.empty() && !menuListValues.newProjectPath.empty())
					{
						if (JCUtil::IsOverlappedDirectoryPath(JCUtil::U8StrToWstr(menuListValues.newProjectName),
							JCUtil::U8StrToWstr(menuListValues.newProjectPath)))
							MessageBox(0, L"Overlapped Project Name", 0, 0);
						else
							CreateNewProjectFolderes();
					}
				}
				JImGuiImpl::EndWindow();
			}
			else
				menuListValues.newProjectButton = false;
		}
		void JProjectSelectorHub::CreateNewProjectFolderes()
		{
			const std::wstring newProejctName = JCUtil::U8StrToWstr(JCUtil::EraseSideChar(menuListValues.newProjectName, ' '));
			const std::wstring newProejctPath = JCUtil::U8StrToWstr(JCUtil::EraseSideChar(menuListValues.newProjectPath, ' ')) + newProejctName;
			std::vector<std::string> version = JApplicationVariable::GetAppVersion();

			JApplicationProject::SetNextProjectInfo(std::make_unique<JApplicationProject::JProjectInfo>(newProejctName, newProejctPath, JCUtil::U8StrToWstr(version[versionIndex])));
			if (!JApplicationProject::StartNewProject())
				MessageBox(0, L"StartNewProject Fail", 0, 0);
		}
		void JProjectSelectorHub::DoActivate()noexcept
		{
			JEditor::DoActivate();
			serachIconTexture = Core::GetUserPtr(JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::SEARCH_FOLDER_ICON));
			CallOnResourceReference(serachIconTexture.Get());

			backgroundTexture = Core::GetUserPtr(JResourceManager::Instance().GetDefaultTexture(J_DEFAULT_TEXTURE::PROJECT_SELECTOR_BACKGROUND));
			CallOnResourceReference(backgroundTexture.Get());
		}
		void JProjectSelectorHub::DoDeActivate()noexcept
		{
			JEditor::DoDeActivate();
			CallOffResourceReference(serachIconTexture.Get());
			CallOffResourceReference(backgroundTexture.Get());
			serachIconTexture.Clear();
			backgroundTexture.Clear();
		}
		void JProjectSelectorHub::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (serachIconTexture.IsValid() && jRobj->GetGuid() == serachIconTexture->GetGuid())
				{
					CallOffResourceReference(serachIconTexture.Get());
					serachIconTexture.Clear();
				}
				if (backgroundTexture.IsValid() && jRobj->GetGuid() == backgroundTexture->GetGuid())
				{
					CallOffResourceReference(backgroundTexture.Get());
					backgroundTexture.Clear();
				}
			}
		}
	}
}