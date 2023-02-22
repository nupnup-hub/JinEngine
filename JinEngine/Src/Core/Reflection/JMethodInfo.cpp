#include"JMethodInfo.h"
#include"JGuiWidgetInfoHandleBase.h"

namespace JinEngine
{
	namespace Core
	{
		std::string JMethodInfo::Name()const noexcept
		{
			return name;
		}
		std::string JMethodInfo::ReturnFieldName()const noexcept
		{
			return returnHint.name;
		}
		std::string JMethodInfo::ParameterFieldName()const noexcept
		{
			std::string names;
			for (int i = 0; i < parameterHint.size() - 1; ++i)
				names += parameterHint[i].name + ", ";
			names += parameterHint[parameterHint.size() - 1].name;
			return names;
		}
		uint JMethodInfo::ParameterCount()const noexcept
		{
			return (uint)parameterHint.size();
		}
		JParameterHint JMethodInfo::GetReturnHint()const noexcept
		{
			return returnHint;
		}
		JTypeInfo* JMethodInfo::GetTypeInfo()const noexcept
		{
			return ownerType;
		}
		JMethodOptionInfo* JMethodInfo::GetOptionInfo()const noexcept
		{
			return optionInfo;
		}
	}
}