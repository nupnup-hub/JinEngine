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
#include"../Core/Func/Functor/JFunctor.h"
#include<windows.h> 
#include<optional>

namespace JinEngine
{
	class JMain; 
	namespace Graphic
	{
		class JGraphic;
		class JGraphicResourceManager; 
	}
	namespace Editor
	{
		class JEditorCameraControl; 
		class JProjectSelectorHub;
	}

	namespace Window
	{
		class JWindowException;
		class JWindowPrivate
		{
		public:
			class MainAccess
			{
			private:
				friend class JMain;
			public:
				using CloseConfirmF = Core::JFunctor<void>;
			private:
				static void Initialize(HINSTANCE hInstance, std::unique_ptr<CloseConfirmF>&& closeConfirmF) ;
				static void OpenProjecSelectorWindow();
				static void OpenEngineWindow();
				static void CloseWindow();
				static std::optional<int> ProcessMessages();
			};
			class HandleInterface
			{
			private:
				friend class JMain;
				friend class JWindowException;
				friend class Graphic::JGraphic;
				friend class Graphic::JGraphicResourceManager; 
				friend class Editor::JEditorCameraControl;
				friend class Editor::JProjectSelectorHub; 
			private:
				static HWND GetHandle()noexcept;
			};
		};
	}
}