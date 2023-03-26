#include"JReflectionInfo.h"
#include"JTypeInfo.h"
#include"JEnumInfo.h"
#include"../JDataType.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Object/Component/JComponent.h"
#include"../../Object/Resource/JResourceObject.h"
#include"../../Utility/JCommonUtility.h"
#include<windows.h>
#include<set>

namespace JinEngine
{
	namespace Core
	{
		void JReflectionInfoImpl::AddType(JTypeInfo* newType)
		{
			assert(newType != nullptr);
			if (GetTypeInfo(newType->TypeGuid()) != nullptr)
				return;

			jType.typeVec.push_back(newType);
			jType.typeNameMap.emplace(newType->TypeGuid(), newType); 
		}
		void JReflectionInfoImpl::AddEnum(JEnumInfo* newEnum)
		{
			assert(newEnum != nullptr);
			if (GetEnumInfo(newEnum->EnumGuid()) != nullptr)
				return;

			jEnum.enumVec.push_back(newEnum);
			jEnum.enumNameMap.emplace(newEnum->EnumGuid(), newEnum);
		}
		JTypeInfo* JReflectionInfoImpl::GetTypeInfo(const std::string& fullname)const noexcept
		{
			return GetTypeInfo(std::hash<std::string>{}(fullname));
		} 
		JTypeInfo* JReflectionInfoImpl::GetTypeInfo(const size_t typeGuid)const noexcept
		{
			auto data = jType.typeNameMap.find(typeGuid);
			return data != jType.typeNameMap.end() ? data->second : nullptr;
		}
		JEnumInfo* JReflectionInfoImpl::GetEnumInfo(const std::string& fullname)const noexcept
		{
			return GetEnumInfo(std::hash<std::string>{}(fullname));
		}
		JEnumInfo* JReflectionInfoImpl::GetEnumInfo(const size_t enumGuid)const noexcept
		{
			auto data = jEnum.enumNameMap.find(enumGuid);
			return data != jEnum.enumNameMap.end() ? data->second : nullptr;
		}
		std::vector<JTypeInfo*> JReflectionInfoImpl::GetAllTypeInfo()const noexcept
		{
			return jType.typeVec;
		}
		std::vector<JTypeInfo*> JReflectionInfoImpl::GetDerivedTypeInfo(const JTypeInfo& baseType)const noexcept
		{
			std::vector<JTypeInfo*> res;
			const uint typeCount = (uint)jType.typeVec.size();
			for (uint i = 0; i < typeCount; ++i)
			{
				if (jType.typeVec[i]->IsA(baseType))
					continue;

				if (jType.typeVec[i]->IsChildOf(baseType))
					res.push_back(jType.typeVec[i]);
			}
			return res;
		}
		void JReflectionInfoImpl::SearchInstance()
		{
			const uint typeCount = (uint)jType.typeVec.size();
			for (uint i = 0; i < typeCount; ++i)
			{ 
				if (jType.typeVec[i]->instanceData != nullptr && jType.typeVec[i]->instanceData->classInstanceVec.size() > 0)
				{
					MessageBox(0, std::to_wstring(jType.typeVec[i]->instanceData->classInstanceVec.size()).c_str(), JCUtil::StrToWstr(jType.typeVec[i]->Name()).c_str(), 0);
				}
			}
		}
		void JReflectionInfoImpl::Initialize()
		{
			static bool callOnce = false;
			if (callOnce)
				return;

			std::vector<JTypeInfo*> typeVec = GetAllTypeInfo(); 
			for (auto& data : typeVec)
				data->ExecuteTypeCallOnece();
			 
			std::set<size_t> allocatedType;
			JGameObject::StaticTypeInfo().RegisterAllocation();
			allocatedType.emplace(JGameObject::StaticTypeInfo().TypeGuid());

			auto componentTypeVec = GetDerivedTypeInfo(JComponent::StaticTypeInfo());
			for (auto& data : componentTypeVec)
			{
				if (!data->IsAbstractType())
				{
					data->RegisterAllocation();
					allocatedType.emplace(data->TypeGuid());
				}
			}

			auto rTypeHintVec = JResourceObject::GetRTypeHintVec(J_RESOURCE_ALIGN_TYPE::DEPENDENCY);
			for (auto& hint : rTypeHintVec)
			{ 
				Core::JTypeInfo& typeInfo = JResourceObject::CallGetTypeInfo(hint.thisType);
				auto derivedVec = GetDerivedTypeInfo(typeInfo);
				for (auto& data : derivedVec)
				{
					if (!data->IsAbstractType())
					{
						data->RegisterAllocation();
						allocatedType.emplace(data->TypeGuid());
					}
				}
			}

			auto restTypeVec = GetDerivedTypeInfo(JIdentifier::StaticTypeInfo());
			for (auto& data : restTypeVec)
			{
				if (!data->IsAbstractType() && allocatedType.find(data->TypeGuid()) == allocatedType.end())
				{
					data->RegisterAllocation();
					allocatedType.emplace(data->TypeGuid());
				}
			}
			callOnce = true;
		}
		void JReflectionInfoImpl::Clear()
		{
			auto typeVec = GetDerivedTypeInfo(JIdentifier::StaticTypeInfo());
			for (auto& data : typeVec)
				data->DeRegisterAllocation();
		}
	}
}