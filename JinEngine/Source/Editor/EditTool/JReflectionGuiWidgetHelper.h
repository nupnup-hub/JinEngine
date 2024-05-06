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
#include"../../Editor/Page/JEditorPageEnum.h"
#include"../../Object/JObjectModifyInterface.h"
#include"../../Core/Reflection/JGuiWidgetInfo.h"
#include"../../Core/Reflection/JGuiWidgetType.h"  
#include<stack>

namespace JinEngine
{
	namespace Core
	{
		class Core::JIdentifier;
		class JTypeInfo;  
		class JGuiWidgetInfoHandleBase;
	}
	namespace Editor
	{
		class JEditorWindow;
		//widget need info 
		//info is declared reflect system
		class JReflectionGuiWidgetHelper
		{
		public:
			struct WidgetOption
			{
			public:
				float nameSpaceOffset = 0;
			};
		private:
			const size_t guid;
			JEditorWindow* ownerWnd; 
		private:
			bool isTableOpen = false;
		public:
			JReflectionGuiWidgetHelper(JEditorWindow* ownerWnd);
			~JReflectionGuiWidgetHelper();
		public:
			void BeginGuiWidget(const Core::JUserPtr<Core::JIdentifier>& obj, const size_t userGuid);
			void EndGuiWidget();
		public:
			JUserPtr<Core::JIdentifier> GetLastSelected()const noexcept;
		public:
			void UpdateGuiWidget(const Core::JUserPtr<Core::JIdentifier>& obj, Core::JTypeInfo* typeInfo);
			void Clear();
		};

		using JRGWH = JReflectionGuiWidgetHelper;
	}
}