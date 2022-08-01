#pragma once 
#include<windows.h> 
#include<optional>

namespace JinEngine
{
	class ImGuiManager;
	class ProjectSelector;
	class EditorCameraControl;
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
			friend class ImGuiManager;
			friend class EditorCameraControl;
			friend class Application::JApplication;
			friend class Core::JWindowException;
			friend class Graphic::JGraphicImpl;
		private:
			virtual HWND GetHandle()const noexcept = 0;
		};
	}
}
/*

		class JWindowEventInterface : public JWindowAppInterface
		{
		private:
			friend class ImGuiManager;
			friend class Graphic::JGraphicImpl;
		private:
			virtual bool AddEventListener(const size_t senderGuid,
				const J_WINDOW_EVENT eventType,
				OnEventPtr ptr,
				IJWindowEventListener* listener) = 0;
			virtual void EraseListener(const size_t senderGuid) = 0;
			virtual void EraseListenerEvent(const size_t senderGuid, const J_WINDOW_EVENT eventType) = 0;
			virtual void ClearEventListener()= 0;
		};
*/