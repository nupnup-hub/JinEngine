#pragma once
#include<windows.h>
#include<memory>
#include<optional>
#include"../Core/Func/Functor/JFunctor.h"

namespace JinEngine
{
	namespace Application
	{
		class JApplication;
	}
	namespace Core
	{
		class JWindowException;
	}
	namespace Graphic
	{
		class JGraphic;
	}
	namespace Editor
	{
		class JEditorCameraControl;
		class JImGui;
		class JProjectSelectorHub;
	}

	namespace Window
	{
		class JWindowPrivate
		{
		public:
			class ApplicationAccess
			{
			private:
				friend class Application::JApplication;
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
				friend class Application::JApplication;
				friend class Core::JWindowException;
				friend class Graphic::JGraphic;
				friend class Editor::JEditorCameraControl;
				friend class Editor::JProjectSelectorHub;
				friend class Editor::JImGui;
			private:
				static HWND GetHandle()noexcept;
			};
		};
	}
}