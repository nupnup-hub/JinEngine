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
		const JParameterHint& JPropertyInfo::GetHint()const noexcept
		{
			return fieldHint;
		}
	}
}