#pragma once
#include"JReflectionMacro.h" 
#include"JTypeInfoRegister.h"
//#include"JTypeInfoOption.h"
#include"JPropertyInfoRegister.h"
#include"JMethodInfoRegister.h"  
#include"JGuiWidgetInfo.h" 
#include"JGuiWidgetInfoHandle.h"
#include"JGuiWidgetInfoHandleBase.h"
#include"JReflectionInfo.h" 
#include"JEnumRegister.h"  
#include<vector>
 
namespace JinEngine
{ 
	namespace Core
	{
		class JIdentifier;
		template<typename T>
		std::string GetName()
		{
			if constexpr (JTypeInfoDetermine<T>::value)
				return RemoveAll_T<T>::TypeName();
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
		std::string GetName(T* ptr)
		{
			if (ptr == nullptr)
				return "UnKnown";

			if constexpr (JTypeInfoDetermine<T>::value)
				return ptr->GetTypeInfo()->Name();
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
		template<typename EnumT>
		std::string GetName(EnumT enumValue)
		{
			if constexpr (JTypeInfoDetermine<EnumT>::value)
				return EnumT::TypeName();
			else if constexpr (IsEnum_V<EnumT>)
			{
				JEnumInfo* jEnumInfo = JReflectionInfo::Instance().GetEnumInfo(typeid(RemoveAll_T<EnumT>).name());
				if (jEnumInfo != nullptr)
					return jEnumInfo->ElementName((int)enumValue);
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
		static JIdentifier* GetRawPtr(const JTypeInstanceSearchHint& hint)
		{
			if (!hint.isValid)
				return nullptr;

			return JReflectionInfo::Instance().GetTypeInfo(hint.typeName)->GetInstanceRawPtr(hint.guid);
		}

		static JIdentifier* SearchRawPtr(Core::JTypeInfo& baseTypeInfo, const size_t guid)
		{
			std::vector<Core::JTypeInfo*> derivedInfoVec = JReflectionInfo::Instance().GetDerivedTypeInfo(baseTypeInfo);
			for (const auto& data : derivedInfoVec)
			{
				auto rawPtr = data->GetInstanceRawPtr(guid);
				if (rawPtr != nullptr)
					return rawPtr;
			}
			return nullptr;
		}
		static JIdentifier* SearchRawPtr(const std::string& baseTypeName, const size_t guid)
		{ 
			return SearchRawPtr(*JReflectionInfo::Instance().GetTypeInfo(baseTypeName), guid);
		}


		template<typename T>
		JUserPtr<T> GetUserPtr(const size_t guid)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return JUserPtr<T>{};

			if (!std::is_base_of_v<JIdentifier, T>)
				return JUserPtr<T>{};

			JUserPtr<JIdentifier> userObjPtr = JReflectionInfo::Instance().GetTypeInfo(T::TypeName())->GetInstanceUserPtr(guid);
			JUserPtr<T> userTPtr;
			userTPtr.ConnnectChildUser(userObjPtr);
			return userTPtr;
		}
		template<typename T>
		JUserPtr<T> GetUserPtr(T* ptr)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return JUserPtr<T>{};

			if (!std::is_base_of_v<JIdentifier, T>)
				return JUserPtr<T>{};

			if (ptr == nullptr)
				return JUserPtr<T>{};

			JUserPtr<JIdentifier> userObjPtr = ptr->GetTypeInfo().GetInstanceUserPtr(ptr->GetGuid());
			JUserPtr<T> userTPtr;
			userTPtr.ConnnectChildUser(userObjPtr);
			return userTPtr;
		} 
		template<typename T, typename U>
		JUserPtr<T> GetUserPtr(U* ptr)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return JUserPtr<T>{};

			if (!std::is_base_of_v<JIdentifier, T> && !std::is_base_of_v<JIdentifier, U>)
				return JUserPtr<T>{};

			if (ptr == nullptr)
				return JUserPtr<T>{};

			JUserPtr<JIdentifier> userObjPtr = ptr->GetTypeInfo().GetInstanceUserPtr(ptr->GetGuid());
			JUserPtr<T> userTPtr;
			userTPtr.ConnnectChildUser(userObjPtr);
			return userTPtr;
		}
		static JUserPtr<JIdentifier> GetUserPtr(const std::string& typeName, const size_t guid)
		{ 
			return JReflectionInfo::Instance().GetTypeInfo(typeName)->GetInstanceUserPtr(guid);
		}
		static JUserPtr<JIdentifier> GetUserPtr(const JTypeInstanceSearchHint& hint)
		{
			if (!hint.isValid)
				return JUserPtr<JIdentifier>{};

			return JReflectionInfo::Instance().GetTypeInfo(hint.typeName)->GetInstanceUserPtr(hint.guid);
		}
		static JUserPtr<JIdentifier> SearchUserPtr(Core::JTypeInfo& baseTypeInfo, const size_t guid)
		{
			std::vector<Core::JTypeInfo*> derivedInfoVec = JReflectionInfo::Instance().GetDerivedTypeInfo(baseTypeInfo);
			for (const auto& data : derivedInfoVec)
			{
				auto userPtr = data->GetInstanceUserPtr(guid);
				if (userPtr.IsValid())
					return userPtr;
			}
			return JUserPtr<JIdentifier>{};
		}
		static JUserPtr<JIdentifier> SearchUserPtr(const std::string& baseTypeName, const size_t guid)
		{
			return SearchUserPtr(*JReflectionInfo::Instance().GetTypeInfo(baseTypeName), guid);
		}

		template<typename enumType>
		std::vector<enumType>GetEnumElementVec()
		{
			JEnumInfo* jEnumInfo = JReflectionInfo::Instance().GetEnumInfo(typeid(RemoveAll_T<enumType>).name());
			if (jEnumInfo != nullptr)
			{
				std::vector<int> intVec = jEnumInfo->GetEnumElementVec(); 
				const uint vecCount = (uint)intVec.size();
				std::vector<enumType> enumVec(vecCount);
				for (uint i = 0; i < vecCount; ++i)
					enumVec[i] = (enumType)intVec[i];
				return enumVec;
			}
			else
				return std::vector<enumType>{};
		}

		static std::string ErasePrefixJ(const std::string& str)
		{
			if (str[0] == 'J')
				return str.substr(1);
			else
				return str;
		}
		static std::wstring ErasePrefixJW(const std::wstring& str)
		{
			if (str[0] == L'J')
				return str.substr(1);
			else
				return str;
		}
	}
}