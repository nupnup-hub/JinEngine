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
#include"../../JEditorWindow.h" 
#include"../../../EditTool/JEditorTabBarHelper.h"

namespace JinEngine
{ 
	namespace Core
	{
		class JLogBase;
	}
	namespace Editor
	{
		template<int itemCount> class JEditorTabBarHelper;
		class JLogViewer final : public JEditorWindow
		{  
		private:
			using GetLogVecPtr = std::vector<Core::JLogBase*>(*)();
			using ClearLogHandlerPtr = void(*)();
		private:
			static constexpr uint tabItemCount = 4;
			std::unique_ptr<JEditorTabBarHelper<tabItemCount>> tabBarHelper;
		private:
			GetLogVecPtr getLogVecPtr[tabItemCount];
			ClearLogHandlerPtr clearLogHandlerPtr[tabItemCount];
		private: 
			int selectedTabIndex = 0;
			int selectedLogIndex = invalidIndex;
		public:
			JLogViewer(const std::string& name, 
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JLogViewer() = default;
			JLogViewer(const JLogViewer& rhs) = delete;
			JLogViewer& operator=(const JLogViewer& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize()noexcept;
			void UpdateWindow()final;
		private:
			void BuildLogViewer(); 
		private: 
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		};
	}
}
