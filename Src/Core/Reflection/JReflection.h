#pragma once
#include"JReflectionMacro.h" 
#include"JTypeInfoRegister.h"
#include"JPropertyRegister.h"
#include"JMethodInfoRegister.h"  
#include"JEnumRegister.h"
#include"JReflectionInfo.h"   
#include<vector>

namespace JinEngine
{ 
	namespace Core
	{
		class JIdentifier;
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
		T* GetRawPtr(const size_t guid)
		{
			if (!std::is_base_of_v<JIdentifier, T>)
				return nullptr;

			return static_cast<T*>(JReflectionInfo::Instance().GetTypeInfo(T::TypeName())->GetInstanceRawPtr(guid));
		}
		static JIdentifier* GetRawPtr(const std::string& typeName, const size_t guid)
		{
			return JReflectionInfo::Instance().GetTypeInfo(typeName)->GetInstanceRawPtr(guid);
		}
			
		template<typename T>
		JUserPtr<T> GetUserPtr(const size_t guid)
		{
			if (!std::is_base_of_v<JIdentifier, T>)
				return JUserPtr<T>{};

			JUserPtr<JIdentifier> userObjPtr = JReflectionInfo::Instance().GetTypeInfo(T::TypeName())->GetInstanceUserPtr(guid);
			JUserPtr<T> userTPtr;
			userTPtr.ConnnectBaseUser(userObjPtr);
			return userTPtr;
		}
		template<typename T>
		JUserPtr<T> GetUserPtr(T* ptr)
		{
			if (!std::is_base_of_v<JIdentifier, T>)
				return JUserPtr<T>{};

			if (ptr == nullptr)
				return JUserPtr<T>{};

			JUserPtr<JIdentifier> userObjPtr = ptr->GetTypeInfo().GetInstanceUserPtr(ptr->GetGuid());
			JUserPtr<T> userTPtr;
			userTPtr.ConnnectBaseUser(userObjPtr);
			return userTPtr;
		} 
		static JUserPtr<JIdentifier> GetUserPtr(const std::string& typeName, const size_t guid)
		{ 
			return JReflectionInfo::Instance().GetTypeInfo(typeName)->GetInstanceUserPtr(guid);
		}

		template<typename enumType>
		std::vector<enumType>GetEnumVec()
		{
			JEnumInfo* jEnumInfo = JReflectionInfo::Instance().GetEnumInfo(typeid(RemoveAll_T<enumType>).name());
			if (jEnumInfo != nullptr)
			{
				std::vector<int> intVec = jEnumInfo->GetEnumVec(); 
				const uint vecCount = (uint)intVec.size();
				std::vector<enumType> enumVec(vecCount);
				for (uint i = 0; i < vecCount; ++i)
					enumVec[i] = (enumType)intVec[i];
				return enumVec;
			}
			else
				return std::vector<enumType>{};
		}
		//add 2^n value enum
		template<typename enumType>
		auto AddSQValueEnum(const enumType ori, const enumType addValue) -> TypeCondition_T< enumType, std::is_enum_v< enumType>>
		{
			return (enumType)((int)ori | ((int)addValue ^ ((int)addValue & (int)ori)));
		}
		template<typename enumType, typename ...Param>
		auto AddSQValueEnum(enumType ori, Param... var) -> TypeCondition_T< enumType, std::is_enum_v< enumType>>
		{
			auto addSQValueEnumLam = [](enumType& ori, enumType addValue)
			{
				ori = (enumType)((int)ori | ((int)addValue ^ ((int)addValue & (int)ori)));
			};
			((addSQValueEnumLam(ori, var)), ...);
			return ori;
		}
		//minus 2^n value enum
		template<typename enumType>
		auto MinusSQValueEnum(const enumType ori, const enumType minusValue) -> TypeCondition_T< enumType, std::is_enum_v< enumType>>
		{
			return (enumType)((int)ori ^ ((int)minusValue & (int)ori));
		}
		//has 2^n value enum
		template<typename enumType, std::enable_if_t<std::is_enum_v<enumType>, int> = 0>
		bool HasSQValueEnum(const enumType ori, const enumType tar) 
		{
			return (((int)ori & (int)tar) > 0);
		}
	}
}