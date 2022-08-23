#pragma once 
#include<windows.h> 
#include<optional>

namespace JinEngine
{
	class ProjectSelector;
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
		class JGraphicImpl;
	}
	namespace Editor
	{
		class JEditorCameraControl;
		class JImGui;
	}
	namespace Window
	{
		class JWindowAppInterface
		{
		private:
			friend class Application::JApplication;
		protected:
			virtual ~JWindowAppInterface() = default;
		private:
			virtual void Initialize(HINSTANCE hInstance) = 0;
			virtual void OpenWindow() = 0;
			virtual void CloseWindow() = 0;
			virtual void SetProjectSelectorWindow() = 0;
			virtual void SetEngineWindow() = 0;
			virtual std::optional<int> ProcessMessages() = 0;
		};
		class JWindowHandleInterface : public JWindowAppInterface
		{
		private:
			friend class ProjectSelector;
			friend class Application::JApplication;
			friend class Core::JWindowException;
			friend class Graphic::JGraphicImpl;
			friend class Editor::JEditorCameraControl;
			friend class Editor::JImGui;
		private:
			virtual HWND GetHandle()const noexcept = 0;
		};
	}
}