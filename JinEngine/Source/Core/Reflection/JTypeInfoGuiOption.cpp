#include"JTypeInfoGuiOption.h"
#include"JPropertyInfo.h"
#include"JMethodInfo.h"
#include"JGuiWidgetInfoHandle.h" 
#include"../Pointer/JOwnerPtr.h"

namespace JinEngine
{
	namespace Core
	{
		uint JTypeInfoGuiOption::GetGuiWidgetInfoHandleCount()const noexcept
		{
			return (uint)widgetHandleVec.size();
		}
		JUserPtr<JGuiWidgetInfoHandleBase> JTypeInfoGuiOption::GetGuiWidgetInfoHandle(const uint index)const noexcept
		{
			return widgetHandleVec[index];
		}
		J_GUI_OPTION_FLAG JTypeInfoGuiOption::GetGuiWidgetFlag()const noexcept
		{
			return guiFlag;
		}
		void JTypeInfoGuiOption::SetGuiWidgetFlag(const J_GUI_OPTION_FLAG value)noexcept
		{ 
			guiFlag = AddSQValueEnum(guiFlag, value); 
		}
		void JTypeInfoGuiOption::AddGuiWidgetInfoHandle(JOwnerPtr<JGuiWidgetInfoHandleBase>&& handle)
		{
			if (handle != nullptr)
				widgetHandleVec.push_back(std::move(handle));
		} 

		void JTypeInfoGuiOptionSetting::SetTypeInfoOption(JTypeInfo* typeInfo, const J_GUI_OPTION_FLAG guiOption)
		{
			typeInfo->GetOption()->SetGuiWidgetFlag(guiOption);
		}
	}
}