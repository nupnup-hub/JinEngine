#pragma once
#include"JReflectionMacro.h" 
#include"JTypeInfoRegister.h" 
#include"JTypeAllocationCreator.h"
#include"JPropertyInfoRegister.h"
#include"JMethodInfoRegister.h"  
#include"JGuiWidgetInfo.h" 
#include"JGuiWidgetInfoHandle.h"
#include"JGuiWidgetInfoHandleBase.h"
#include"JReflectionInfo.h" 
#include"JEnumRegister.h"    
#include"../Utility/JCommonUtility.h"
#include"../Pointer/JOwnerPtr.h"
#include<vector>

namespace JinEngine
{ 
	namespace Core
	{
		class JTypeBase;
		template<typename T>
		static std::string GetName()
		{
			if constexpr (JTypeInfoDetermine<T>::value)
				return RemoveAll_T<T>::TypeName();
			else if constexpr (IsEnum_V<T>)
			{
				JEnumInfo* jEnumInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(RemoveAll_T<T>).name());
				if (jEnumInfo != nullptr)
					return jEnumInfo->Name();
				else
					return "UnKnown";
			} 
			else
				return "UnKnown";
		}
		template<typename T>
		static std::wstring GetWName()
		{
			return JCUtil::StrToWstr(GetName<T>());
		}
		template<typename T>
		static std::string GetName(T* ptr)
		{
			if (ptr == nullptr)
				return "UnKnown";

			if constexpr (JTypeInfoDetermine<T>::value)
				return ptr->GetTypeInfo()->Name();
			else if constexpr (IsEnum_V<T>)
			{
				JEnumInfo* jEnumInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(RemoveAll_T<T>).name());
				if (jEnumInfo != nullptr)
					return jEnumInfo->ElementName(*ptr);
				else
					return "UnKnown";
			}
			else
				return "UnKnown";
		}
		template<typename T>
		static std::wstring GetWName(T* ptr)
		{
			if (ptr == nullptr)
				return L"UnKnown";

			if constexpr (std::is_base_of_v<JIdentifier, T>)
				return ptr->GetName();
			else  
				return JCUtil::StrToWstr(GetName(ptr));
		}
		template<typename T>
		static std::string GetName(T value)
		{
			if constexpr (IsEnum_V<T>)
			{
				JEnumInfo* jEnumInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(RemoveAll_T<T>).name());
				if (jEnumInfo != nullptr)
					return jEnumInfo->ElementName(value);
				else
					return "UnKnown";
			}
			else
				return "UnKnown";
		} 
		template<typename T>
		static std::wstring GetWName(T value)
		{
			return JCUtil::StrToWstr(GetName(value));
		}
		template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
		static std::string GetEnumName(T value, const bool toLowercase, const bool eraseUnderbar)
		{
			JEnumInfo* jEnumInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(RemoveAll_T<T>).name());
			if (jEnumInfo != nullptr)
			{
				std::string str = jEnumInfo->ElementName(value);
				if (eraseUnderbar)
					str = JCUtil::ChangeWord(str, "_", " ");
				if (toLowercase)
					str = JCUtil::ToLowercase(str, 1);
				return str;
			}
			else
				return "UnKnown";
		}


		static JTypeBase* GetRawPtr(const size_t typeGuid, const size_t objGuid)
		{
			return _JReflectionInfo::Instance().GetTypeInfo(typeGuid)->GetInstanceRawPtr(objGuid);
		}
		static JTypeBase* GetRawPtr(const std::string& typeFullName, const size_t objGuid)
		{
			return _JReflectionInfo::Instance().GetTypeInfo(typeFullName)->GetInstanceRawPtr(objGuid);
		}
		static JTypeBase* GetRawPtr(const JTypeInstanceSearchHint& hint)
		{
			if (!hint.isValid)
				return nullptr;

			return _JReflectionInfo::Instance().GetTypeInfo(hint.typeGuid)->GetInstanceRawPtr(hint.objectGuid);
		}
		template<typename T>
		static T* GetRawPtr(const size_t objGuid)
		{
			if (!std::is_base_of_v<JTypeBase, T>)
				return nullptr;

			return T::StaticTypeInfo().GetInstanceRawPtr<T>(objGuid);
		}
		template<typename T>
		static T* GetRawPtr(T* ptr)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return nullptr;

			if (!std::is_base_of_v<JTypeBase, T>)
				return nullptr;

			if (ptr == nullptr)
				return nullptr;

			return ptr->GetTypeInfo().GetInstanceRawPtr<T>(ptr->GetGuid());
		}
		template<typename T, typename U>
		static T* GetRawPtr(U* ptr)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return nullptr;

			if (!std::is_base_of_v<JTypeBase, T> && !std::is_base_of_v<JTypeBase, U>)
				return nullptr;

			if (ptr == nullptr)
				return nullptr;

			return ptr->GetTypeInfo().GetInstanceRawPtr<T>(ptr->GetGuid());
		}
		template<typename T>
		static T* GetRawPtr(const size_t typeGuid, const size_t objGuid)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return nullptr;

			if (!std::is_base_of_v<JTypeBase, T>)
				return nullptr;

			return _JReflectionInfo::Instance().GetTypeInfo(typeGuid)->GetInstanceRawPtr<T>(objGuid);
		}
		template<typename T>
		static T* GetRawPtr(const std::string& typeFullName, const size_t objGuid)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return nullptr;

			if (!std::is_base_of_v<JTypeBase, T>)
				return nullptr;

			return _JReflectionInfo::Instance().GetTypeInfo(typeFullName)->GetInstanceRawPtr<T>(objGuid);
		}
		template<typename T>
		static T* GetRawPtr(const JTypeInstanceSearchHint& hint)
		{
			if (!hint.isValid)
				return nullptr;

			return _JReflectionInfo::Instance().GetTypeInfo(hint.typeGuid)->GetInstanceRawPtr<T>(hint.objectGuid);
		}
		static JTypeBase* SearchRawPtr(JTypeInfo& baseTypeInfo, const size_t objGuid)
		{
			std::vector<JTypeInfo*> derivedInfoVec = _JReflectionInfo::Instance().GetDerivedTypeInfo(baseTypeInfo);
			for (const auto& data : derivedInfoVec)
			{
				auto rawPtr = data->GetInstanceRawPtr(objGuid);
				if (rawPtr != nullptr)
					return rawPtr;
			}
			return nullptr;
		}
		static JTypeBase* SearchRawPtr(const std::string& baseTypeFullName, const size_t objGuid)
		{ 
			return SearchRawPtr(*_JReflectionInfo::Instance().GetTypeInfo(baseTypeFullName), objGuid);
		}
		template<typename  T>
		static T* SearchRawPtr(JTypeInfo& baseTypeInfo, const size_t objGuid)
		{
			return static_cast<T*>(SearchRawPtr(baseTypeInfo, objGuid));
		}
		template<typename  T>
		static T* SearchRawPtr(const std::string& baseTypeFullName, const size_t objGuid)
		{
			return static_cast<T*>(SearchRawPtr(baseTypeFullName, objGuid));
		}

		static JUserPtr<JTypeBase> GetUserPtr(const size_t typeGuid, const size_t objGuid)
		{
			return _JReflectionInfo::Instance().GetTypeInfo(typeGuid)->GetInstanceUserPtr(objGuid);
		}
		static JUserPtr<JTypeBase> GetUserPtr(const std::string& typeFullName, const size_t objGuid)
		{ 
			return _JReflectionInfo::Instance().GetTypeInfo(typeFullName)->GetInstanceUserPtr(objGuid);
		}
		static JUserPtr<JTypeBase> GetUserPtr(const JTypeInstanceSearchHint& hint)
		{
			if (!hint.isValid)
				return JUserPtr<JTypeBase>{};

			return _JReflectionInfo::Instance().GetTypeInfo(hint.typeGuid)->GetInstanceUserPtr(hint.objectGuid);
		}		
		template<typename T>
		static JUserPtr<T> GetUserPtr(const size_t objGuid)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return JUserPtr<T>{};

			if (!std::is_base_of_v<JTypeBase, T>)
				return JUserPtr<T>{};

			return T::StaticTypeInfo().GetInstanceUserPtr<T>(objGuid);
		}
		template<typename T>
		static JUserPtr<T> GetUserPtr(T* ptr)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return JUserPtr<T>{};

			if (!std::is_base_of_v<JTypeBase, T>)
				return JUserPtr<T>{};

			if (ptr == nullptr)
				return JUserPtr<T>{};

			return ptr->GetTypeInfo().GetInstanceUserPtr<T>(ptr->GetGuid());
		}
		template<typename T, typename U>
		static JUserPtr<T> GetUserPtr(U* ptr)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return JUserPtr<T>{};

			if (!std::is_base_of_v<JTypeBase, T> && !std::is_base_of_v<JTypeBase, U>)
				return JUserPtr<T>{};

			if (ptr == nullptr)
				return JUserPtr<T>{};

			return ptr->GetTypeInfo().GetInstanceUserPtr<T>(ptr->GetGuid());
		}
		template<typename T>
		static JUserPtr<T> GetUserPtr(const size_t typeGuid, const size_t objGuid)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return JUserPtr<T>{};

			if (!std::is_base_of_v<JTypeBase, T>)
				return JUserPtr<T>{};

			return _JReflectionInfo::Instance().GetTypeInfo(typeGuid)->GetInstanceUserPtr<T>(objGuid);
		}
		template<typename T>
		static JUserPtr<T> GetUserPtr(const std::string& typeFullName, const size_t objGuid)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return JUserPtr<T>{};

			if (!std::is_base_of_v<JTypeBase, T>)
				return JUserPtr<T>{};

			return _JReflectionInfo::Instance().GetTypeInfo(typeFullName)->GetInstanceUserPtr<T>(objGuid);
		}
		template<typename T>
		static JUserPtr<T> GetUserPtr(const JTypeInstanceSearchHint& hint)
		{
			if (!hint.isValid)
				return JUserPtr<T>{};

			return _JReflectionInfo::Instance().GetTypeInfo(hint.typeGuid)->GetInstanceUserPtr<T>(hint.objectGuid);
		}
		static JUserPtr<JTypeBase> SearchUserPtr(JTypeInfo& baseTypeInfo, const size_t objGuid)
		{
			std::vector<JTypeInfo*> derivedInfoVec = _JReflectionInfo::Instance().GetDerivedTypeInfo(baseTypeInfo);
			for (const auto& data : derivedInfoVec)
			{
				auto userPtr = data->GetInstanceUserPtr(objGuid);
				if (userPtr.IsValid())
					return userPtr;
			}
			return JUserPtr<JTypeBase>{};
		}
		static JUserPtr<JTypeBase> SearchUserPtr(const std::string& baseTypeFullName, const size_t objGuid)
		{
			return SearchUserPtr(*_JReflectionInfo::Instance().GetTypeInfo(baseTypeFullName), objGuid);
		}
		template<typename  T>
		static JUserPtr<T> SearchUserPtr(JTypeInfo& baseTypeInfo, const size_t objGuid)
		{
			return ConvertChildUserPtr<T>(SearchUserPtr(baseTypeInfo, objGuid));
		}
		template<typename  T>
		static JUserPtr<T> SearchUserPtr(const std::string& baseTypeFullName, const size_t objGuid)
		{
			return ConvertChildUserPtr<T>(SearchUserPtr(baseTypeFullName, objGuid));
		}

		static JWeakPtr<JTypeBase> GetWeakPtr(const size_t typeGuid, const size_t objGuid)
		{
			return _JReflectionInfo::Instance().GetTypeInfo(typeGuid)->GetInstanceWeakPtr(objGuid);
		}
		static JWeakPtr<JTypeBase> GetWeakPtr(const std::string& typeFullName, const size_t objGuid)
		{
			return _JReflectionInfo::Instance().GetTypeInfo(typeFullName)->GetInstanceWeakPtr(objGuid);
		}
		static JWeakPtr<JTypeBase> GetWeakPtr(const JTypeInstanceSearchHint& hint)
		{
			if (!hint.isValid)
				return JWeakPtr<JTypeBase>{};

			return _JReflectionInfo::Instance().GetTypeInfo(hint.typeGuid)->GetInstanceWeakPtr(hint.objectGuid);
		}
		template<typename T>
		static JWeakPtr<T> GetWeakPtr(const size_t objGuid)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return JWeakPtr<T>{};

			if (!std::is_base_of_v<JTypeBase, T>)
				return JWeakPtr<T>{};

			return T::StaticTypeInfo().GetInstanceWeakPtr<T>(objGuid);
		}
		template<typename T>
		static JWeakPtr<T> GetWeakPtr(T* ptr)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return JWeakPtr<T>{};

			if (!std::is_base_of_v<JTypeBase, T>)
				return JWeakPtr<T>{};

			if (ptr == nullptr)
				return JWeakPtr<T>{};

			return ptr->GetTypeInfo().GetInstanceWeakPtr<T>(ptr->GetGuid());
		}
		template<typename T, typename U>
		static JWeakPtr<T> GetWeakPtr(U* ptr)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return JWeakPtr<T>{};

			if (!std::is_base_of_v<JTypeBase, T> && !std::is_base_of_v<JTypeBase, U>)
				return JWeakPtr<T>{};

			if (ptr == nullptr)
				return JWeakPtr<T>{};

			return ptr->GetTypeInfo().GetInstanceWeakPtr<T>(ptr->GetGuid());
		}
		template<typename T>
		static JWeakPtr<T> GetWeakPtr(const size_t typeGuid, const size_t objGuid)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return JWeakPtr<T>{};

			if (!std::is_base_of_v<JTypeBase, T>)
				return JWeakPtr<T>{};

			return _JReflectionInfo::Instance().GetTypeInfo(typeGuid)->GetInstanceWeakPtr<T>(objGuid);
		}
		template<typename T>
		static JWeakPtr<T> GetWeakPtr(const std::string& typeFullName, const size_t objGuid)
		{
			if constexpr (!JTypeInfoDetermine<T>::value)
				return JWeakPtr<T>{};

			if (!std::is_base_of_v<JTypeBase, T>)
				return JWeakPtr<T>{};

			return _JReflectionInfo::Instance().GetTypeInfo(typeFullName)->GetInstanceWeakPtr<T>(objGuid);
		}
		template<typename T>
		static JWeakPtr<T> GetWeakPtr(const JTypeInstanceSearchHint& hint)
		{
			if (!hint.isValid)
				return JWeakPtr<T>{};

			return _JReflectionInfo::Instance().GetTypeInfo(hint.typeGuid)->GetInstanceWeakPtr<T>(hint.objectGuid);
		}
		static JWeakPtr<JTypeBase> SearchWeakPtr(JTypeInfo& baseTypeInfo, const size_t objGuid)
		{
			std::vector<JTypeInfo*> derivedInfoVec = _JReflectionInfo::Instance().GetDerivedTypeInfo(baseTypeInfo);
			for (const auto& data : derivedInfoVec)
			{
				auto weakPtr = data->GetInstanceWeakPtr(objGuid);
				if (weakPtr.IsValid())
					return weakPtr;
			}
			return JWeakPtr<JTypeBase>{};
		}
		static JWeakPtr<JTypeBase> SearchWeakPtr(const std::string& baseTypeFullName, const size_t objGuid)
		{
			return SearchUserPtr(*_JReflectionInfo::Instance().GetTypeInfo(baseTypeFullName), objGuid);
		}


		template<typename enumType>
		std::vector<enumType>GetEnumElementVec()
		{
			JEnumInfo* jEnumInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(RemoveAll_T<enumType>).name());
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
		template<typename enumType>
		std::vector<int>GetEnumElementValueVec()
		{
			JEnumInfo* jEnumInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(RemoveAll_T<enumType>).name());
			if (jEnumInfo != nullptr)
				return jEnumInfo->GetEnumElementVec();
			else
				return std::vector<int>{};
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