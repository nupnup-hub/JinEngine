#pragma once 
#include"JWindowEventType.h"
#include"../Utility/JVector.h" 
#include"../Core/Func/Functor/JFunctor.h"
#include"../Core/Event/JEventManager.h"
#include<windows.h>  
#include<string>

namespace JinEngine
{
	namespace Window
	{ 
		using WindowEvManager = Core::JEventManager<size_t, J_WINDOW_EVENT>;
		using WindowEvInterface = WindowEvManager::Interface;

		class JWindow
		{
		public:
			static RECT GetWindowR()noexcept;
			static RECT GetPreWindowR()noexcept;
			static RECT GetClientR()noexcept;
			static RECT GetPreClientR()noexcept;
			static int GetDisplayWidth()noexcept;
			static int GetDisplayHeight()noexcept;
			static int GetMinWidth()noexcept;
			static int GetMinHeight()noexcept;
			static int GetWindowPositionX()noexcept;
			static int GetWindowPositionY()noexcept;
			static int GetWindowWidth()noexcept;
			static int GetWindowHeight()noexcept;
			static JVector2<int> GetClientPos()noexcept;
			static int GetClientPositionX()noexcept;
			static int GetClientPositionY()noexcept;
			static JVector2<int> GetClientSize()noexcept;
			static int GetClientWidth()noexcept;
			static int GetClientHeight()noexcept;
		public:
			static bool IsFullScreen()noexcept;
			static bool HasStorageSpace(const std::wstring& dirPath, size_t capacity)noexcept;
		public:
			static bool SelectDirectory(std::wstring& dirPath, const std::wstring& guide)noexcept;
			static bool SelectFile(std::wstring& filePath, const std::wstring& guide)noexcept;
		public:
			static WindowEvInterface* EvInterface()noexcept;
		};
	} 
	using JWindow = Window::JWindow;
}