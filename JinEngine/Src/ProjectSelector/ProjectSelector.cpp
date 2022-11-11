#include"ProjectSelector.h" 
#include"../Application/JApplicationVariable.h"
#include"../Utility/JCommonUtility.h"
#include"../Graphic/JGraphic.h"
#include"../Graphic/JGraphicResourceManager.h"
#include"../Object/Resource/JResourceManager.h"
#include"../Object/Resource/Texture/JTexture.h"
#include"../Editor/GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../Window/JWindows.h"
#include"../../Lib/imgui/imgui.h"
#include<shellapi.h>
#include<ShlObj_core.h>
#include<tchar.h>
#include<fstream>  
#include<io.h>
#pragma comment(lib, "shell32")

namespace JinEngine
{
	void ProjectSelector::Initialize()
	{  
		LoadProjectInfo();
	}
	void ProjectSelector::UpdateWindow(JApplicationVariable* applicationVar)
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2((float)JWindow::Instance().GetClientWidth(), (float)JWindow::Instance().GetClientHeight()));
		ImGui::Begin("main", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

		ImGuiManager::PushFont(1);
		TitleOnScreen();
		MenuListOnScreen(applicationVar);
		ProjectListOnScreen(applicationVar);
		ImGuiManager::PopFont();
		ImGui::End();
	}
	void ProjectSelector::TitleOnScreen()
	{
		ImGui::BeginChild("Title", ImVec2((float)JWindow::Instance().GetClientWidth(), (float)JWindow::Instance().GetClientHeight() * 0.1f), true, 0);
		ImGui::SetCursorPos(ImVec2(JWindow::Instance().GetClientWidth() * 0.05f, JWindow::Instance().GetClientHeight() * 0.015f));

		ImGui::Text("JinEngine");
		ImGui::EndChild();
	}
	void ProjectSelector::MenuListOnScreen(JApplicationVariable* applicationVar)
	{
		optionListCusorY = ImGui::GetCursorPosY();
		ImGui::BeginChild("Option List", ImVec2(JWindow::Instance().GetClientWidth() * 0.25f, JWindow::Instance().GetClientHeight() * 0.9f), true, 0);
		ImGui::SetWindowFontScale(0.85f);
		if (ImGui::Button("New Project", ImVec2(JWindow::Instance().GetClientWidth() * 0.25f, JWindow::Instance().GetClientHeight() * 0.1f)))
		{
			menuListValues.newProjectButton = !menuListValues.newProjectButton;
			if (menuListValues.newProjectButton)
			{
				menuListValues.OpenCreateProjectMenu();
			}
		} 
		if (ImGui::Button("Load Project", ImVec2(JWindow::Instance().GetClientWidth() * 0.25f, JWindow::Instance().GetClientHeight() * 0.1f)))
		{

		}
		ImGui::EndChild();
		ImGui::SetWindowFontScale(1);
		if (menuListValues.newProjectButton)
		{
			CreateNewProjectOnScreen(applicationVar);
		}
	}
	void ProjectSelector::ProjectListOnScreen(JApplicationVariable* applicationVar)
	{ 
		ImGui::SetCursorPos(ImVec2(JWindow::Instance().GetClientWidth() * 0.265f, optionListCusorY));
		ImGui::BeginChild("Project List", ImVec2(JWindow::Instance().GetClientWidth() * 0.72f, JWindow::Instance().GetClientHeight() * 0.9f), true, 0);
		
		float titleTextYOffset = (JWindow::Instance().GetClientHeight() * 0.13f) * 0.1f;
		float pathTextYOffset = (JWindow::Instance().GetClientHeight() * 0.13f) * 0.65f;
 
		for (int i = 0; i < projectInfoList.size(); ++i)
		{
			ImGui::SetCursorPosY(JWindow::Instance().GetClientHeight() * 0.13f * i);
			if (ImGui::Button(("##" + projectInfoList[i].name).c_str(), ImVec2(JWindow::Instance().GetClientWidth() * 0.7f, JWindow::Instance().GetClientHeight() * 0.125f)))
			{
				int index = (int)projectInfoList[i].path.find_last_of("\\");
				std::string path = projectInfoList[i].path.substr(0, index);

				applicationVar->MakeFolderPath(projectInfoList[i].name, path);
				applicationVar->AccessProjectFolder();
				applicationVar->StartNewProject();
			}		 
			ImGui::SetWindowFontScale(0.85f);
			ImGui::SetCursorPos(ImVec2(JWindow::Instance().GetClientWidth() * 0.025f, JWindow::Instance().GetClientHeight()  * 0.13f * i + titleTextYOffset));
			ImGui::Text(projectInfoList[i].name.c_str());
			ImGui::SetWindowFontScale(0.5f);
			ImGui::SetCursorPos(ImVec2(JWindow::Instance().GetClientWidth() * 0.025f, JWindow::Instance().GetClientHeight()  * 0.13f * i + pathTextYOffset));
			ImGui::Text(projectInfoList[i].path.c_str());
		} 
		ImGui::SetWindowFontScale(1);
		ImGui::EndChild();
	}
	void ProjectSelector::SaveProjectInfo()
	{
		//std::locale::global(std::locale("Korean"));
		std::wofstream stream;

		stream.open(JApplicationVariable::GetEnginePath() + "\\" +
			projectInfoSaveFolder + "\\" +
			projectInfoSaveFileName +
			projectInfoSaveFileFormat,
			std::ios::out | std::ios::binary);
		 
		for (const auto& data : projectInfoList)
		{ 
			stream << JCUtil::U8StrToWstr(data.name) << " " << 
				JCUtil::U8StrToWstr(data.path) << " " <<
				data.mainVersion << " " << data.subVersion << " ";
		}

		stream.close();
	}
	void ProjectSelector::LoadProjectInfo()
	{
		//std::locale::global(std::locale("Korean"));
		std::wifstream stream;

		stream.open(JApplicationVariable::GetEnginePath() + "\\" +
			projectInfoSaveFolder + "\\" +
			projectInfoSaveFileName + 
			projectInfoSaveFileFormat, 
			std::ios::in | std::ios::binary);

		std::string dirPath = JApplicationVariable::GetEnginePath() + "\\" + projectInfoSaveFolder;
		std::wstring name;
		std::wstring path;
		int mainVersion;
		float subVersion;

		if (stream.fail())
		{
			if (_access(dirPath.c_str(), 00) == -1)
			{
				if (_mkdir(dirPath.c_str()) == -1)
					MessageBox(0, L"ProejectList 폴더생성실패", 0, 0);
			}
		}
		else
		{
			// stream.eof == 파일끝을지나 읽기시도시
			//https://stackoverflow.com/questions/4533063/how-does-ifstreams-eof-work
			while (stream >> name >> path >> mainVersion >> subVersion)
			{
				if (_access(JCUtil::WstrToU8Str(path).c_str(), 00) == 0)
				{
					// 필) 프로젝트 존재여부를 확인할 방법추가 
					projectInfoList.push_back(ProjectInfo(JCUtil::WstrToU8Str(name),
						JCUtil::WstrToU8Str(path),
						mainVersion, subVersion));
				}
			}
		}
		stream.close();
	}
	void ProjectSelector::CreateNewProjectOnScreen(JApplicationVariable* applicationVar)
	{
		std::wstring dirPath;
		if (menuListValues.newProjectWinow)
		{
			ImGui::Begin("Create New Project", &menuListValues.newProjectWinow,
				ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoCollapse);

			ImGui::SetWindowSize(ImVec2(JWindow::Instance().GetClientWidth() * 0.7f, JWindow::Instance().GetClientHeight() * 0.6f));
			ImGui::SetWindowFontScale(0.6f);
			ImGui::SetCursorPosY(JWindow::Instance().GetClientHeight() * 0.4f);
			ImGui::Text("Project Name ");
			ImGui::InputText("##Project Name", menuListValues.newProejctName, 50);

			ImGui::Text("Folder");
			ImGui::InputText("##Folder Path", menuListValues.newProejctPath, 260, ImGuiInputTextFlags_ReadOnly);

			ImGui::SameLine();
			/*uint index = JResourceManager::Instance().GetEditorTexture(J_EDITOR_TEXTURE::SEARCH_FOLDER_ICON)->GetTextureHeapIndex();
			if (ImGui::ImageButton((ImTextureID)JGraphic::Instance().ResourceInterface()->GetGpuSrvDescriptorHandle(index).ptr, ImVec2((float)25, (float)25)))
			{
				std::wstring dirPath;
				if (SelectFolderPath(dirPath) && HasStroageSpace(dirPath))
				{
					const std::string strDirPath = JCUtil::WstrToU8Str(dirPath);
					const uint strDirPathSize = (uint)strDirPath.size();

					for (uint i = 0; i < strDirPathSize; ++i)
						menuListValues.newProejctPath[i] = strDirPath[i];
				}
			}

			if (ImGui::Button("Create Proejct"))
			{  
				if (JCUtil::IsOverlappedDirectoryPath(JCUtil::U8StrToWstr(menuListValues.newProejctName),
					JCUtil::U8StrToWstr(menuListValues.newProejctPath)))
					MessageBox(0, L"Overlapped Project Name", 0, 0);
				else if (menuListValues.newProejctName != " " &&
					menuListValues.newProejctPath != " ")
					CreateNewProjectFolderes(applicationVar);
			}*/
			ImGui::End();
		}
		else
		{
			menuListValues.newProjectButton = false;
		}
	}
	void ProjectSelector::CreateNewProjectFolderes(JApplicationVariable* applicationVar)
	{ 
		applicationVar->MakeFolderPath(menuListValues.newProejctName, menuListValues.newProejctPath);

		if (applicationVar->AccessProjectFolder())
		{
			applicationVar->StartNewProject();
			projectInfoList.push_back(ProjectInfo(JApplicationVariable::GetActivatedProjectName(),
				JApplicationVariable::GetActivatedProjectPath(), 1, 0));
			SaveProjectInfo();
		}
		else
			MessageBox(0, L"StartNewProject Fail", 0, 0);
	}
	bool ProjectSelector::SelectFolderPath(std::wstring& dirPath)
	{
		BROWSEINFO   browserInfo;
		LPITEMIDLIST  idl;
		TCHAR path[MAX_PATH] = { 0, };
		ZeroMemory(&browserInfo, sizeof(BROWSEINFO));
		browserInfo.hwndOwner = JWindow::Instance().HandleInterface()->GetHandle();
		browserInfo.pszDisplayName = path;
		browserInfo.lpszTitle = _T("폴더를 선택해 주세요");
		browserInfo.ulFlags = BIF_EDITBOX | BIF_USENEWUI | 0x0040;

		idl = SHBrowseForFolder(&browserInfo);
		if (idl)
		{
			SHGetPathFromIDList(idl, path);
			dirPath = path;
			return true;
		}
		else
			return false;
	}
	bool ProjectSelector::HasStroageSpace(const std::wstring& dirPath)
	{
		ULARGE_INTEGER uliAvailable;
		ULARGE_INTEGER uliTotal;
		ULARGE_INTEGER uliFree;

		if (GetDiskFreeSpaceExA(JCUtil::WstrToU8Str(dirPath).c_str(), &uliAvailable, &uliTotal, &uliFree))
		{
			ULONGLONG result = uliAvailable.QuadPart / 1048576;
			if (result > necessaryCapacityMB)
				return true;
			else
				return false;
		}
		return false;
	}
}