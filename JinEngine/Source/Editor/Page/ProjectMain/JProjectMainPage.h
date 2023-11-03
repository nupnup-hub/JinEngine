#pragma once  
#include"../JEditorPage.h"  
#include"../../Interface/JEditorProjectInterface.h" 
#include"../../../Object/JObjectModifyInterface.h"

namespace JinEngine
{
	namespace Editor
	{
		class JWindowDirectory;
		class JObjectExplorer;
		class JObjectDetail;
		class JSceneViewer;
		class JSceneObserver;
		class JLogViewer; 
		class JGraphicResourceWatcher;
		class JStringConvertTest;
		class JApplicationWatcher;
		class JEditorMenuBar;
		class JGraphicOptionSetting;
		class JWindowStateViewer;
		class JEditorCloseConfirmPopup;

		class JProjectMainPage final : public JEditorPage, public JModifedObjectInterface
		{ 
		private:
			using ClosePopupOpenF = Core::JSFunctorType<void, JProjectMainPage*>;
			using ClosePopupConfirmF = Core::JSFunctorType<void, JProjectMainPage*>;
			using ClosePopupCancelF = Core::JSFunctorType<void, JProjectMainPage*>;
			using ClosePopupContentsF = Core::JSFunctorType<void, JProjectMainPage*>;
			using ClosePopupCloseF = Core::JSFunctorType<void>;
		private:
			std::unique_ptr<JEditorMenuBar> menuBar = nullptr;
		private:
			//JEditor derivated window
			std::unique_ptr<JWindowDirectory> windowDirectory;
			std::unique_ptr<JObjectExplorer> objectExplorer;
			std::unique_ptr<JObjectDetail>objectDetail;
			std::unique_ptr<JSceneViewer> sceneViewer;
			std::unique_ptr<JSceneObserver> sceneObserver;
			std::unique_ptr<JLogViewer> logViewer; 
			std::unique_ptr<JGraphicResourceWatcher> graphicResourceWatcher;
			std::unique_ptr<JStringConvertTest> stringConvertTest;
			std::unique_ptr<JApplicationWatcher> appWatcher;
		private:
			std::unique_ptr<JEditorCloseConfirmPopup> closePopup;
		private:
			//simple window
			std::unique_ptr<JGraphicOptionSetting> graphicOptionSetting;
			std::unique_ptr<JWindowStateViewer> wndStateViewer;
		private:
			std::unique_ptr<ClosePopupOpenF::Functor> closePopupOpenF;
			std::unique_ptr<ClosePopupConfirmF::Functor> closePopupConfirmF; 
			std::unique_ptr<ClosePopupCancelF::Functor> closePopupCancelF;
			std::unique_ptr<ClosePopupContentsF::Functor> closePopupContetnsF;
		private:
			std::unique_ptr<JEditorProjectInterface> projInterface;
		public:
			JProjectMainPage(std::unique_ptr<JEditorProjectInterface>&& newProjInterface);
			~JProjectMainPage();
			JProjectMainPage(const JProjectMainPage& rhs) = delete;
			JProjectMainPage& operator=(const JProjectMainPage& rhs) = delete;
		public:
			J_EDITOR_PAGE_TYPE GetPageType()const noexcept final;
			void SetInitWindow() final;
		public:
			void Initialize()final;
			void UpdatePage()final;
		public:
			bool IsValidOpenRequest(const JUserPtr<Core::JIdentifier>& selectedObj)noexcept final;
		private:
			void RequestCloseConfirmPopup(const bool isCancel);
		private:
			void BuildDockNode();
			void BuildMenuNode();
		private:
			void BeginScenePlay();
			void EndScenePlay();
		};
	}
}