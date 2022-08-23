#pragma once
#include"JInputManager.h"  
#include"JWindowInterface.h"
#include"JWindowEventType.h"
#include"../Core/JDataType.h"
#include"../Core/Exception/JException.h"  
#include"../Core/Event/JEventManager.h"
#include"../Core/Singleton/JSingletonHolder.h"
#include<windowsx.h> 
#include<windows.h> 
#include<vector>
#include<functional>

namespace JinEngine
{
	namespace Core
	{
		template<typename T>class JCreateUsingNew;
	}

	namespace Window
	{
		class JWindowImpl :public JWindowHandleInterface, public Core::JEventManager<size_t, J_WINDOW_EVENT>
		{
		private:
			template<typename T>friend class Core::JCreateUsingNew;
		private:
			const size_t guid;
			JInputManager inputManager;
			WNDCLASSEX wc;
			HINSTANCE hInst;
			const std::wstring windowClassName = L"JinEngineClass";
			const std::wstring windowName = L"JinEngine";
			bool cursorEnabled = true;
			RECT preWindowRect;
			RECT preClinetRect;
			const int minWidth;
			const int minHeight;
			HWND hwnd;
		public:
			RECT GetWindowR()const noexcept;
			RECT GetPreWindowR()const noexcept;
			RECT GetClientR()const noexcept;
			RECT GetPreClientR()const noexcept;

			int GetDisplayWidth()const noexcept;
			int GetDisplayHeight()const noexcept;
			int GetMinWidth()const noexcept;
			int GetMinHeight()const noexcept;
			int GetWindowPositionX()const noexcept;
			int GetWindowPositionY()const noexcept;
			int GetWindowWidth()const noexcept;
			int GetWindowHeight()const noexcept;
			int GetClientPositionX()const noexcept;
			int GetClientPositionY()const noexcept;
			int GetClientWidth()const noexcept;
			int GetClientHeight()const noexcept;
		public:
			JWindowHandleInterface* HandleInterface() ;
			JEventInterface* EvInterface()final;
			JWindowAppInterface* AppInterface();
		private:
			void Initialize(HINSTANCE hInstance)final;
			void OpenWindow()final;
			void CloseWindow()final;
			void SetProjectSelectorWindow()final;
			void SetEngineWindow()final;
			std::optional<int> ProcessMessages()final;
		private:
			HWND GetHandle()const noexcept final;
		private:
			static LRESULT CALLBACK HandleMsgSetup(HWND hwnd, uint msg, WPARAM wParam, LPARAM lParam);
			static LRESULT CALLBACK HandleMsgThunk(HWND hwnd, uint msg, WPARAM wParam, LPARAM lParam);
			static LRESULT HandleMsg(HWND hwnd, uint msg, WPARAM wParam, LPARAM lParam);
			void Resize(WPARAM wParam);
		private:
			void RegisterWindowClass();
			void RegistEvCallable() final;
		private:
			JWindowImpl();
			~JWindowImpl();
		};
	}
	using JWindow = JinEngine::Core::JSingletonHolder<Window::JWindowImpl>;
}

//JWindowInterface
//JWindowAppInterface
//JWindowEventInterface

/*
		class WindowClass
		{
		public:
			 const wchar_t* GetName() noexcept;
			 HINSTANCE GetInstance() noexcept;
		private:
			WindowClass() noexcept;
			~WindowClass();
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator=(const WindowClass&) = delete;
			 const wchar_t* wndClassName;
			 WindowClass wndClass;
			HINSTANCE hInst;
		};
*/