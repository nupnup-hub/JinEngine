#pragma once
#include"../JEditorPage.h"  
#include"../../../Core/Event/JEventListener.h" 
#include"../../../Object/Resource/JResourceObjectEventType.h"
#include"../../../Object/Resource/JResourceObject.h"

namespace JinEngine
{ 
	class JAnimationController;
	class JScene;
	namespace Editor
	{ 
		class JAnimationDiagramList;
		class JAnimationParameterList;
		class JAnimationStateView;
		class JSceneObserver;
		class JObjectDetail;
		class JEditorMenuBar;
		class JEditorAniContInterface;
		class JEditorAniContPage final : public JEditorPage, public JResourceEventManager::Listener
		{
		private:
			using ResourceEvListener = JResourceEventManager::Listener;
		private:
			JUserPtr<JAnimationController> aniCont;
			JUserPtr<JScene> aniPreviweScene;
		private:
			std::unique_ptr<JAnimationDiagramList>diagramList;
			std::unique_ptr<JAnimationParameterList>conditionList;
			std::unique_ptr<JAnimationStateView>stateView; 
			std::unique_ptr<JSceneObserver> aniContObserver;
			std::unique_ptr<JObjectDetail> aniContDetail;
		private:
			std::unique_ptr<JEditorMenuBar> menuBar; 
		private:
			std::vector<JEditorAniContInterface*> contEditVec;
		private:
			bool setWndOptionOnce = false;
		public:
			JEditorAniContPage();
			~JEditorAniContPage();
		public:
			J_EDITOR_PAGE_TYPE GetPageType()const noexcept final;
			void SetInitWindow() final;
		public:
			void Initialize()final;
			void UpdatePage()final;
		public:
			bool IsValidOpenRequest(const JUserPtr<Core::JIdentifier>& selectedObj)noexcept final;
		private:
			bool StuffAniContData(const JUserPtr<Core::JIdentifier>& selectedObj);
		protected:
			void DoSetClose()noexcept final;
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void LoadPage(JFileIOTool& tool)final;
			void StorePage(JFileIOTool& tool)final;
		private:
			void BuildDockNode();
			void BuildMenuNode();
		private:
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj, JResourceEventDesc* desc)final;
			void OnEvent(const size_t& iden, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)final;
		};
	}
}
