#pragma once
#include"../../Core/JCoreEssential.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Core/Math/JVector.h"
#include"../../Core/Reflection/JReflection.h"
#include"../Gui/JGuiType.h"
#include<string>

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