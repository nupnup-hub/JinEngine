#pragma once
#include"../JEditorPage.h"  
#include"../../../Core/Event/JEventListener.h" 
#include"../../../Object/Resource/JResourceObjectEventType.h"

namespace JinEngine
{
	class JResourceObject;
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
		class JEditorAniContPage final : public JEditorPage, public Core::JEventListener<size_t, J_RESOURCE_EVENT_TYPE, JResourceObject*>
		{
		private:
			using ResourceEvListener = Core::JEventListener<size_t, J_RESOURCE_EVENT_TYPE, JResourceObject*>;
		private:
			Core::JUserPtr<JAnimationController> aniCont;
			Core::JUserPtr<JScene> aniPreviweScene;
		private:
			std::unique_ptr<JAnimationDiagramList>diagramList;
			std::unique_ptr<JAnimationParameterList>conditionList;
			std::unique_ptr<JAnimationStateView>stateView; 
			std::unique_ptr<JSceneObserver> aniContObserver;
			std::unique_ptr<JObjectDetail> aniContDetail;
		private:
			std::unique_ptr<JEditorMenuBar> menuBar; 
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
			bool IsValidOpenRequest(const Core::JUserPtr<Core::JIdentifier>& selectedObj)noexcept final;
		private:
			bool StuffAniContData(const Core::JUserPtr<Core::JIdentifier>& selectedObj);
		protected:
			void DoSetClose()noexcept final;
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		public:
			void StorePage(std::wofstream& stream)final;
			void LoadPage(std::wifstream& stream)final;
		private:
			void BuildDockNode();
			void BuildMenuNode();
		private:
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
		};
	}
}
