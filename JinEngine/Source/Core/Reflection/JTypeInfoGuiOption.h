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
#include"JGuiWidgetInfoHandleBase.h"   
#include"../Pointer/JOwnerPtr.h" 

namespace JinEngine
{
	namespace Core
	{
		enum J_GUI_OPTION_FLAG
		{
			J_GUI_OPTION_NONE = 0,
			J_GUI_OPTION_DISPLAY_PARENT = 1 << 0,
			J_GUI_OPTION_DISPLAY_PARENT_TO_CHILD = 1 << 1,		//	default is child to parent
		};
	 
		//template<typename Type> class JTypeInfoRegister;
		template<typename Type, typename Field, typename Pointer, Pointer ptr> class JPropertyInfoRegister;
		template<typename Type, typename Field, typename Pointer, Pointer ptr> class JPropertyExInfoRegister;
		template<typename Type, typename GetPointer, GetPointer getPtr>  class JMethodReadOnlyGuiWidgetRegister;
		template<typename Type, typename GetPointer, GetPointer getPtr>  class JMethodGuiWidgetRegister;
		class JTypeInfo;
		 
		class JTypeInfoGuiOption
		{
		private:  
			friend JTypeInfo;
			template<typename Type, typename Field, typename Pointer, Pointer ptr> friend class JPropertyInfoRegister;
			template<typename Type, typename Field, typename Pointer, Pointer ptr> friend class JPropertyExInfoRegister;
			template<typename Type, typename GetPointer, GetPointer getPtr> friend class JMethodReadOnlyGuiWidgetRegister;
			template<typename Type, typename GetPointer, GetPointer getPtr> friend class JMethodGuiWidgetRegister; 
		private:
			//Gui option
			std::vector<JOwnerPtr<JGuiWidgetInfoHandleBase>> widgetHandleVec;
			J_GUI_OPTION_FLAG guiFlag = J_GUI_OPTION_NONE;
		public:
			uint GetGuiWidgetInfoHandleCount()const noexcept; 
			JUserPtr<JGuiWidgetInfoHandleBase> GetGuiWidgetInfoHandle(const uint index)const noexcept;
			J_GUI_OPTION_FLAG GetGuiWidgetFlag()const noexcept;
		public:
			void SetGuiWidgetFlag(const J_GUI_OPTION_FLAG value)noexcept; 
		private:
			void AddGuiWidgetInfoHandle(JOwnerPtr<JGuiWidgetInfoHandleBase>&& handle);
		private:
			JTypeInfoGuiOption() = default;
			~JTypeInfoGuiOption() = default;
		};

		class JTypeInfoGuiOptionSetting
		{
		public:
			static void SetTypeInfoOption(JTypeInfo* typeInfo, const J_GUI_OPTION_FLAG guiOption);
		};
	}
}