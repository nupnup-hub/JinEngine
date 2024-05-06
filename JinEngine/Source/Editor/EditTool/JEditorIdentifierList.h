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
#include"../../Core/JCoreEssential.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Core/Math/JVector.h"
#include"../../Core/Reflection/JReflection.h"
#include"../Gui/JGuiType.h" 

namespace JinEngine
{
	namespace Core
	{
		class JIdentifier;
	}
	namespace Editor
	{
		class JEditorIdentifierList
		{
		public:
			struct TypeConditionDataSet
			{
			public:
				size_t ownerGuid = 0;
				size_t parentGuid = 0;
			};
		public:
			using CanDisplayF = Core::JSFunctorType<bool, TypeConditionDataSet, Core::JIdentifier*>;
			using CanDisplayB = Core::JBindHandle<CanDisplayF::Functor, TypeConditionDataSet, Core::EmptyParam>;
			using DisplayName = Core::JSFunctorType<std::string, Core::JIdentifier*>;
			using SelectF = Core::JSFunctorType<void, Core::JIdentifier*>;
		public:
			struct DisplayDataSet
			{
			public:
				std::string unqLabel;
				JVector2<float> winodwSize;		//0, 0 => gui api deafult size
				float listWidth = 0;
				J_GUI_WINDOW_FLAG_ customWindowFlag = J_GUI_WINDOW_FLAG_NONE;
				J_GUI_COMBO_FLAG_ customComboFlag = J_GUI_COMBO_FLAG_NONE;
			public: 
				std::unique_ptr<CanDisplayB> canDisplayB = nullptr;
				std::unique_ptr<SelectF::Functor> selectF = nullptr;
				std::unique_ptr<Core::JBindHandleBase> selectB = nullptr;
			public:
				bool callWindowAutomatically = true;
				bool canDisplayContents = true;
			public:
				DisplayDataSet(const std::string& unqLabel); 
			};
		private:
			size_t typeGuid = 0;
		private:
			size_t preSelectedGuid = 0;  
		public:
			void Display(const DisplayDataSet& dataSet)noexcept;
		public: 
			size_t GetSelectedGuid()const noexcept;
			template<typename T>
			JUserPtr<T> GetSelectedUser()const noexcept
			{
				return Core::GetUserPtr<T>(preSelectedGuid);
			}
		public:
			void SetTypeGuid(const size_t guid);
			void SetSelecetdGuid(const size_t guid)noexcept; 
		};
	}
}