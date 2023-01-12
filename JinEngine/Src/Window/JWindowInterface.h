#pragma once 
#include"../Core/Func/Functor/JFunctor.h"
#include<windows.h> 
#include<optional>
#include<memory>

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
		class JGraphicImpl;
	}
	namespace Editor
	{
		class JEditorCameraControl;
		class JImGui;
		class JProjectSelectorHub;
	}
	namespace Window
	{
		class JWindowAppInterface
		{
		private:
			friend class Application::JApplication;
		protected:
			using CloseConfirmF = Core::JMFunctorType<Application::JApplication, void>;
		protected:
			virtual ~JWindowAppInterface() = default;
		private:
			virtual void Initialize(HINSTANCE hInstance, std::unique_ptr<CloseConfirmF::Functor> closeConfirmF) = 0;
			virtual void OpenProjecSelectorWindow() = 0;
			virtual void OpenEngineWindow() = 0; 
			virtual void CloseWindow() = 0; 
			virtual std::optional<int> ProcessMessages() = 0; 
		};
		class JWindowHandleInterface : public JWindowAppInterface
		{
		private:
			friend class Application::JApplication;
			friend class Core::JWindowException;
			friend class Graphic::JGraphicImpl;
			friend class Editor::JEditorCameraControl;
			friend class Editor::JProjectSelectorHub;
			friend class Editor::JImGui;
		private:
			virtual HWND GetHandle()const noexcept = 0;
		};
	}
}