/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"../../JEditorWindow.h"    

namespace JinEngine
{  
	class JScene;
	class JCamera;
	class JTexture;
	namespace Editor
	{
		class JEditorCameraControl;
		class JEditorIdentifierList;
		class JSceneViewer final : public JEditorWindow
		{
		private:
			std::unique_ptr<JEditorIdentifierList> camList;
			//JUserPtr<JTexture> shiroBack;
		private: 
			JUserPtr<JScene> scene;
			JUserPtr<JCamera> selectedCam;
			std::unique_ptr<JEditorCameraControl> editorCamCtrl;
		public:
			JSceneViewer(const std::string& name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE pageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JSceneViewer();
			JSceneViewer(const JSceneViewer& rhs) = delete;
			JSceneViewer& operator=(const JSceneViewer& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept;
		public:
			void Initialize(JUserPtr<JScene> newScene);
			void UpdateWindow()final;
		private:
			void UpdateMouseWheel()final;
		private:
			void DoActivate() noexcept final;
			void DoDeActivate() noexcept final;
		private:
			void LoadEditorWindow(JFileIOTool& tool)final;
			void StoreEditorWindow(JFileIOTool& tool)final;
		private:
			void TestLight();
		};
	}
}
