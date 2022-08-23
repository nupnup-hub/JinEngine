#pragma once  
#include"../JEditorPage.h"  

namespace JinEngine
{
	namespace Editor
	{
		class JLogViewer;
		class JObjectDetail;
		class JObjectExplorer;
		class JMainSceneEditor;
		class JSceneViewer;
		class JWindowDirectory;
		class JAnimationControllerEditor;
		class JGraphicResourceWatcher;
		class JStringConvertTest;

		class JProjectMainPage : public JEditorPage
		{
		private:
			std::unique_ptr<JWindowDirectory>windowDirectory;
			std::unique_ptr<JObjectExplorer>objectExplorer;
			std::unique_ptr<JMainSceneEditor>sceneEditor;
			std::unique_ptr<JObjectDetail>objectDetail;
			std::unique_ptr<JLogViewer>logViewer;
			std::unique_ptr<JSceneViewer> sceneViewer;
			std::unique_ptr<JAnimationControllerEditor> animationControllerEditor;
			std::unique_ptr<JGraphicResourceWatcher> graphicResourceWatcher;
			std::unique_ptr<JStringConvertTest> stringConvertTest;

		public:
			JProjectMainPage(bool hasMetadata);
			~JProjectMainPage() = default;
			JProjectMainPage(const JProjectMainPage& rhs) = delete;
			JProjectMainPage& operator=(const JProjectMainPage& rhs) = delete;
		public:
			void Initialize(bool hasImguiTxt)override;
			void UpdatePage()override;
		};
	}
}