#pragma once 
#include"../Reflection/JReflection.h"

namespace JinEngine
{
	namespace Core
	{
		//Dependency injection data
		class JDITypeDataBase
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JDITypeDataBase)
		public:
			JDITypeDataBase() = default;
			virtual ~JDITypeDataBase() = default;
		public:
			virtual bool IsValidData()const noexcept;
		public:
			static bool IsValidData(JDITypeDataBase* base, const JTypeInfo& correctType);
			static bool IsValidChildData(JDITypeDataBase* base, const JTypeInfo& correctType);
		};
	}
}