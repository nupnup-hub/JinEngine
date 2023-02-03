#pragma once
#include"../../JEditorWindow.h" 
#include"../../../../Object/Resource/JResourceUserInterface.h"
#include<string>

namespace JinEngine
{
	class JTexture;
	namespace Editor
	{
		class SelectorValues;
		class JProjectSelectorHub : public JEditorWindow ,
			public JResourceUserInterface
		{ 
		private: 
			Core::JUserPtr<JTexture> serachIconTexture;
			Core::JUserPtr<JTexture> backgroundTexture;
		private:
			std::vector<Core::JUserPtr<JTexture>> lastRSVec;
		private:
			std::unique_ptr<SelectorValues> selectorValues;
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
			void ProjectListOnScreen(); 
			void ProjectDetailOnScreen();
			void GuideButtonOnScreen();
			void CreateNewProjectOnScreen();
			void LoadProjectOnScreen();
			void SetStartProjectProccess();
		private:
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
		};
	}
}