#pragma once
#include"../../JEditorWindow.h"  
#include"../../../../Core/Event/JEventListener.h"
#include"../../../../Object/Resource/JResourceObjectEventType.h"
#include<string>

namespace JinEngine
{
	class JTexture;
	class JResourceObject;
	namespace Editor
	{
		struct SelectorValues;
		class JEditorSearchBarHelper;
		class JEditorDynamicSpotColor;
		class JProjectSelectorHub : public JEditorWindow ,
			public Core::JEventListener<size_t, J_RESOURCE_EVENT_TYPE, JResourceObject*>
		{ 
		private:
			using ResourceEvListener = Core::JEventListener<size_t, J_RESOURCE_EVENT_TYPE, JResourceObject*>;
		private: 
			JUserPtr<JTexture> serachIconTexture;
			JUserPtr<JTexture> optionSettingTexture;
			JUserPtr<JTexture> backgroundTexture;
		private:
			//last project rendering scene
			std::vector<JUserPtr<JTexture>> lastRSVec;
			std::unique_ptr<JEditorSearchBarHelper> searchHelper;
			std::unique_ptr<JEditorDynamicSpotColor> dynamicCol;
		private:
			std::unique_ptr<SelectorValues> values;
			float optionListCusorY;
			static constexpr uint necessaryCapacityMB = 200;
		private:
			int newProjVersion; 
			int versionIndex = 0;
		public:
			JProjectSelectorHub(const std::string& name, 
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JProjectSelectorHub();
			JProjectSelectorHub(const JProjectSelectorHub& rhs) = delete;
			JProjectSelectorHub& operator=(const JProjectSelectorHub& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public: 
			void UpdateWindow()final;
		private:
			void UpdateCanvasSize();
			void TitleOnScreen();
			void ProjectListOnScreen(); 
			void ProjectDetailOnScreen();
			void GuideButtonOnScreen();
			void CreateNewProjectOnScreen();
			void LoadProjectOnScreen();
			void OptionOnScreen();
		private:
			void SetStartProjectProccess();
		private:
			void LoadLastRsTexture();
		private:
			void DoSetOpen()noexcept final;
			void DoSetClose()noexcept final; 
		private:
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
		};
	}
}