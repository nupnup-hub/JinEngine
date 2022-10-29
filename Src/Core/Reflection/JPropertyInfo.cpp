#include"JPropertyInfo.h"  

namespace JinEngine
{
	namespace Core
	{
		std::string JPropertyInfo::Name()const noexcept
		{
			return name;
		}
		std::string JPropertyInfo::FieldName()const noexcept
		{
			return fieldHint.name;
		}
		JParameterHint JPropertyInfo::GetHint()const noexcept
		{
			return fieldHint;
		}
		JTypeInfo* JPropertyInfo::GetTypeInfo()const noexcept
		{
			return ownerType;
		}
		JPropertyOptionInfo* JPropertyInfo::GetOptionInfo()const noexcept
		{
			return optionInfo;
		}
	}
}