#pragma once
#include"../../JEditorWindow.h"
#include"../../../../Object/Resource/JResourceUserInterface.h"
#include<string>

namespace JinEngine
{
	class JTexture;
	namespace Editor
	{
		class JProjectSelectorHub : public JEditorWindow ,
			public JResourceUserInterface
		{ 
		private:
			struct MenuListValues
			{
			public:
				std::string newProjectName;
				std::string newProjectPath;
				bool newProjectWinow = false;
				bool newProjectButton = false;
				bool loadProjectButton = false;
			public:
				static constexpr uint maxNameRange = 50;
				static constexpr uint maxPathRange = 260;
			public:
				void OpenCreateProjectMenu()
				{
					newProjectWinow = true;
					newProjectName.clear();
					newProjectPath.clear();
					newProjectName.resize(maxNameRange);
					newProjectPath.resize(maxPathRange);
				}
			};
		private: 
			Core::JUserPtr<JTexture> serachIconTexture;
			Core::JUserPtr<JTexture> backgroundTexture;
		private:
			MenuListValues menuListValues;
			float optionListCusorY;
			static constexpr uint necessaryCapacityMB = 200;
		private:
			int newProjVersion; 
			int versionIndex = 0;
		public:
			JProjectSelectorHub(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType);
			~JProjectSelectorHub();
			JProjectSelectorHub(const JProjectSelectorHub& rhs) = delete;
			JProjectSelectorHub& operator=(const JProjectSelectorHub& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public: 
			void UpdateWindow()final;
		private:
			void TitleOnScreen();
			void MenuListOnScreen();
			void ProjectListOnScreen();
			void CreateNewProjectOnScreen();
			void CreateNewProjectFolderes();   
		private:
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
		};
	}
}