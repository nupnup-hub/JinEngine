#pragma once  
#include"../JEditorPage.h"  
#include"../../Interface/JEditorObjectInterface.h"

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
		class JAnimationControllerEditor;
		class JGraphicResourceWatcher;
		class JStringConvertTest;
		class JAppElapsedTime;
		class JEditorMenuBar;
		class JGraphicOptionSetting;
		class JEditorCloseConfirmPopup;

		class JProjectMainPage final : public JEditorPage, 
			public JEditorModifedObjectStructureInterface
		{
		private:
			using StoreProjectF = Core::JSFunctorType<void>;
			using LoadProjectF = Core::JSFunctorType<void>;
		private:
			using ClosePopupConfirmF = Core::JSFunctorType<void, JEditorPage*>; 
			using ClosePopupCancelF = Core::JSFunctorType<void, JEditorPage*>;
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
			std::unique_ptr<JAnimationControllerEditor> animationControllerEditor;
			std::unique_ptr<JGraphicResourceWatcher> graphicResourceWatcher;
			std::unique_ptr<JStringConvertTest> stringConvertTest;
			std::unique_ptr<JAppElapsedTime> appElapseTime;
		private:
			std::unique_ptr<JEditorCloseConfirmPopup> closePopup;
		private:
			//simple window
			std::unique_ptr<JGraphicOptionSetting> graphicOptionSetting;
		private:
			std::unique_ptr<StoreProjectF::Functor> storeProjectF;
			std::unique_ptr<LoadProjectF::Functor> loadProjectF;
		private:
			std::unique_ptr<ClosePopupConfirmF::Functor> closePopupConfirmF; 
			std::unique_ptr<ClosePopupCancelF::Functor> closePopupCancelF;
		private:
			bool reqInitDockNode = false; 
		public:
			JProjectMainPage(const bool hasMetadata);
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
			bool IsValidOpenRequest(const Core::JUserPtr<JObject>& selectedObj)noexcept final;
		private:
			void BuildDockNode();
			void BuildMenuNode();
		};
	}
}