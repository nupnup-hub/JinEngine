#include"JTypeInfoOption.h"
#include"JPropertyInfo.h"
#include"JMethodInfo.h"
#include"JGuiWidgetInfoHandle.h" 

namespace JinEngine
{
	namespace Core
	{
		uint JTypeInfoOption::GetGuiWidgetInfoHandleCount()const noexcept
		{
			return (uint)widgetHandleVec.size();
		}
		JGuiWidgetInfoHandleBase* JTypeInfoOption::GetGuiWidgetInfoHandle(const uint index)const noexcept
		{
			return widgetHandleVec[index].get();
		}
		J_GUI_OPTION_FLAG JTypeInfoOption::GetGuiWidgetFlag()const noexcept
		{
			return guiFlag;
		}
		void JTypeInfoOption::SetGuiWidgetFlag(const J_GUI_OPTION_FLAG value)noexcept
		{ 
			guiFlag = AddSQValueEnum(guiFlag, value); 
		}
		void JTypeInfoOption::AddGuiWidgetInfoHandle(std::unique_ptr<JGuiWidgetInfoHandleBase>&& handle)
		{
			if (handle != nullptr)
				widgetHandleVec.push_back(std::move(handle));
		} 

		void JTypeInfoOptionSetting::SetTypeInfoOption(JTypeInfo* typeInfo, const J_GUI_OPTION_FLAG guiOption)
		{
			typeInfo->GetOption()->SetGuiWidgetFlag(guiOption);
		}
	}
}