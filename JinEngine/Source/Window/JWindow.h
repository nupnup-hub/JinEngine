#pragma once 
#include"JWindowEventType.h"
#include"../Core/Math/JVector.h" 
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
			static JVector2<float> GetDisplayPosition()noexcept;
			static JVector2<float> GetDisplaySize()noexcept; 
			static JVector2<float> GetMinSize()noexcept;
			static JVector2<float> GetWindowPosition()noexcept;
			static JVector2<float> GetWindowSize()noexcept;
			static JVector2<float> GetClientPosition()noexcept; 
			static JVector2<float> GetClientSize()noexcept; 
			static uint GetMaxDisplayFrequency()noexcept;
		public:
			static bool IsFullScreen()noexcept;
			static bool IsActivated()noexcept;
			static bool HasStorageSpace(const std::wstring& dirPath, size_t capacity)noexcept;
		public:
			static bool SelectDirectory(_Out_ std::wstring& dirPath, const std::wstring& guide)noexcept;
			static bool SelectFile(_Out_ std::wstring& filePath, const std::wstring& guide)noexcept;
			static bool SelectMultiFile(_Out_ std::vector< std::wstring>& filePath, const std::wstring& guide)noexcept;
		public:
			static WindowEvInterface* EvInterface()noexcept;
		};
	} 
	using JWindow = Window::JWindow;
}