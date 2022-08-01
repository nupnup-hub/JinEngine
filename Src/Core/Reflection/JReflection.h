#pragma once
#include"JReflectionMacro.h" 
#include"JPropertyRegister.h"
#include"JMethodInfoRegister.h"  
#include"JEnumInfo.h"
#include"JReflectionInfo.h" 

namespace JinEngine
{
	namespace Core
	{
		template<typename T>
		std::string GetName()
		{
			if constexpr (IsClass_V<T>)
			{
				JTypeInfo* jTypeInfo = JReflectionInfo::Instance().GetTypeInfo(typeid(RemoveAll_T<T>).name());
				if (jTypeInfo != nullptr)
					return jTypeInfo->Name();
				else
					return "UnKnown";
			}
			else if constexpr (IsEnum_V<T>)
			{
				JEnumInfo* jEnumInfo = JReflectionInfo::Instance().GetEnumInfo(typeid(RemoveAll_T<T>).name());
				if (jEnumInfo != nullptr)
					return jEnumInfo->Name();
				else
					return "UnKnown";
			} 
			else
				return "UnKnown";
		}

		template<typename T>
		std::string GetName(const T& value)
		{
			if constexpr (IsClass_V<T>)
			{
				JTypeInfo* jTypeInfo = JReflectionInfo::Instance().GetTypeInfo(typeid(RemoveAll_T<T>).name());
				if (jTypeInfo != nullptr)
					return jTypeInfo->Name();
				else
					return "UnKnown";
			}
			else if constexpr (IsEnum_V<T>)
			{
				JEnumInfo* jEnumInfo = JReflectionInfo::Instance().GetEnumInfo(typeid(RemoveAll_T<T>).name());
				if (jEnumInfo != nullptr)
					return jEnumInfo->ElementName((int)value);
				else
					return "UnKnown";
			} 
			return "UnKnown";
		}
	}
}