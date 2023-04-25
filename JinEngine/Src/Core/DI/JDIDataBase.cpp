#include"JDIDataBase.h"

namespace JinEngine
{
	namespace Core
	{ 
		bool JDITypeDataBase::IsValidData()const noexcept
		{
			return true;
		}
		bool JDITypeDataBase::IsValidData(JDITypeDataBase* base, const JTypeInfo& correctType)
		{
			return base != nullptr && base->IsValidData() && base->GetTypeInfo().IsA(correctType);
		}
		bool JDITypeDataBase::IsValidChildData(JDITypeDataBase* base, const JTypeInfo& correctType)
		{
			return base != nullptr && base->IsValidData() && base->GetTypeInfo().IsChildOf(correctType);
		}
	}
}