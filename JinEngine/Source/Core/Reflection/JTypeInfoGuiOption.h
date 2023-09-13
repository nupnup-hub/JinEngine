#pragma once 
#include"JGuiWidgetInfoHandleBase.h"  
#include"../JCoreEssential.h" 
#include"../Pointer/JOwnerPtr.h"
#include<memory>
#include<vector>

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
			J_GUI_OPTION_FLAG guiFlag;
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