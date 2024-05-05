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