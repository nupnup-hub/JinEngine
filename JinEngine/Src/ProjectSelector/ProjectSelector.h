#pragma once
#include<string> 
#include<vector> 
#include<direct.h>
#include<Windows.h>
#include"../Core/JDataType.h"
#include"../Object/Resource/Texture/JEditorTextureEnum.h"
#include"../Application/JApplicationVariable.h"

namespace JinEngine
{
	struct ProjectInfo
	{
	public:
		std::string name;
		std::string path;
		int mainVersion;
		float subVersion;
	public:
		ProjectInfo(const std::string& name, const std::string& path, int mainVersion, float subVersion)
			:name(name), path(path), mainVersion(mainVersion), subVersion(subVersion)
		{}
	};

	class ProjectSelector
	{
		struct MenuListValues
		{
		public:
			char newProejctName[50] = "";
			char newProejctPath[260] = "";
			//std::string newProejctName = " ";
			//std::string newProejctPath = " ";
			bool newProjectWinow = false;
			bool newProjectButton = false;
			bool loadProjectButton = false;
		public:
			void OpenCreateProjectMenu()
			{
				newProjectWinow = true;
				newProejctName[0] = '\0';
				newProejctPath[0] = '\0';
				//newProejctName = " ";
				//newProejctPath = " ";
			}
		};
	private:
		std::vector<ProjectInfo> projectInfoList;
		const std::string projectInfoSaveFolder = "ProjectSelectorCash";
		const std::string projectInfoSaveFileName = "ProjectList";
		const std::string projectInfoSaveFileFormat = ".txt";
		MenuListValues menuListValues;
		const uint necessaryCapacityMB = 200;
		float optionListCusorY;
	public:
		void Initialize();
		void UpdateWindow(JApplicationVariable* applicationVar);
	private:
		//void SearchDirectory();
		//void SaveProjectInfo();
		void TitleOnScreen();
		void MenuListOnScreen(JApplicationVariable* applicationVar);
		void ProjectListOnScreen(JApplicationVariable* applicationVar);
		void SaveProjectInfo();
		void LoadProjectInfo();
		void CreateNewProjectOnScreen(JApplicationVariable* applicationVar);
		void CreateNewProjectFolderes(JApplicationVariable* applicationVar);
		bool SelectFolderPath(std::wstring& path);
		bool HasStroageSpace(const std::wstring& dirPath);
	};
}