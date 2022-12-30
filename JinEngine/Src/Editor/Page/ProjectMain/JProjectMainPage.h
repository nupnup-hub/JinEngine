#pragma once  
#include"../JEditorPage.h"  
#include"../CommonWindow/Debug/JStringConvertTest.h" 
#include"../CommonWindow/View/JSceneViewer.h"
#include"../CommonWindow/View/JSceneObserver.h"
#include"../CommonWindow/Detail/JObjectDetail.h"
#include"../CommonWindow/Explorer/JObjectExplorer.h"
#include"../CommonWindow/Debug/JGraphicResourceWatcher.h"
#include"../CommonWindow/Debug/JAppElapsedTime.h"
#include"Window/JAnimationControllerEditor.h"
#include"Window/JLogViewer.h" 
#include"Window/JWindowDirectory.h"  
#include"Window/JGraphicOptionSetting.h"

namespace JinEngine
{
	namespace Editor
	{
		//class JWindowDirectory;
		//class JSceneViewer;
		//class JSceneObserver;
		//class JObjectExplorer;
		//class JLogViewer;

	//	class JAnimationControllerEditor;
		//class JStringConvertTest;
		//class JGraphicResourceWatcher;

		class JProjectMainPage final : public JEditorPage
		{
		private:
			using StoreProjectF = Core::JSFunctorType<void>;
			using LoadProjectF = Core::JSFunctorType<void>;
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
			//simple window
			std::unique_ptr<JGraphicOptionSetting> graphicOptionSetting;
		private:
			std::unique_ptr<StoreProjectF::Functor> storeProjectF;
			std::unique_ptr<LoadProjectF::Functor> loadProjectF;
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