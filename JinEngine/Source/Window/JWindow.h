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
#include"JWindowEventType.h"
#include"../Core/Math/JVector.h" 
#include"../Core/Func/Functor/JFunctor.h"
#include"../Core/Event/JEventManager.h"
#include<windows.h>   

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