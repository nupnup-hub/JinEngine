#pragma once
#include"JReflectionMacro.h" 
#include"JTypeInfoRegister.h"
#include"JPropertyRegister.h"
#include"JMethodInfoRegister.h"  
#include"JEnumRegister.h"
#include"JReflectionInfo.h"  

namespace JinEngine
{
	class JObject;
	namespace Core
	{
		template<typename T>
		std::string GetName()
		{
			if constexpr (IsClass_V<T>)
			{
				JTypeInfo* jTypeInfo = JReflectionInfo::Instance().GetTypeInfo(RemoveAll_T<T>::TypeName());
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
				JTypeInfo* jTypeInfo = JReflectionInfo::Instance().GetTypeInfo(RemoveAll_T<T>::TypeName());
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
		template<typename T>
		T* GetObject(const size_t guid)
		{
			if (!std::is_base_of_v<JObject, T>)
				return nullptr;

			return static_cast<T*>(JReflectionInfo::Instance().GetTypeInfo(T::TypeName())->GetInstanceRawPtr(guid));
		}
		template<typename T>
		JUserPtr<T> GetUserPtr(const size_t guid)
		{
			if (!std::is_base_of_v<JObject, T>)
				return nullptr;

			JUserPtr<JObject> userObjPtr = JReflectionInfo::Instance().GetTypeInfo(T::TypeName())->GetInstanceUserPtr(guid);
			JUserPtr<T> userTPtr;
			userTPtr.ConnnectBaseUser(userObjPtr);
			return userTPtr;
		}

		template<typename enumType>
		enumType AddTwoSquareValueEnum(const enumType ori, const enumType addValue)
		{
			return (enumType)(ori | (addValue ^ (addValue & ori)));
		}
		template<typename enumType>
		enumType MinusTwoSquareValueEnum(const enumType ori, const enumType minusValue)
		{
			return (enumType)(ori ^ (minusValue & ori));
		}
	}
}