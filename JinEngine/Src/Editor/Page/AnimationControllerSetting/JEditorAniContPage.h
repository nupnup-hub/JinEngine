#pragma once
#include"../JEditorPage.h" 
#include"../../Interface/JEditorObjectInterface.h"
#include"../../../Object/Resource/JResourceUserInterface.h"

namespace JinEngine
{
	class JAnimationController;
	class JScene;
	namespace Editor
	{ 
		class JAnimationDiagramList;
		class JAnimationConditionList;
		class JAnimationStateView;
		class JSceneObserver;
		class JObjectDetail;
		class JEditorMenuBar;
		class JEditorAniContPage final : public JEditorPage, public JResourceUserInterface
		{
		private:
			Core::JUserPtr<JAnimationController> aniCont;
			Core::JUserPtr<JScene> aniPreviweScene;
		private:
			std::unique_ptr<JAnimationDiagramList>diagramList;
			std::unique_ptr<JAnimationConditionList>conditionList;
			std::unique_ptr<JAnimationStateView>stateView; 
			std::unique_ptr<JSceneObserver> aniContObserver;
			std::unique_ptr<JObjectDetail> aniContDetail;
		private:
			std::unique_ptr<JEditorMenuBar> menuBar;
		private:
			bool reqInit;
		public:
			JEditorAniContPage(bool hasMetadata);
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
