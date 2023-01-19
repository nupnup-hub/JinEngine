#include"JReflectionInfo.h"
#include"JTypeInfo.h"
#include"JEnumInfo.h"
#include"../JDataType.h"
#include"../../Utility/JCommonUtility.h"
#include<windows.h>
namespace JinEngine
{
	namespace Core
	{
		void JReflectionInfoImpl::AddType(JTypeInfo* newType)
		{
			assert(newType != nullptr);
			jType.typeVec.push_back(newType);
			jType.typeNameMap.emplace(newType->Name(), newType); 
		}
		void JReflectionInfoImpl::AddEnum(JEnumInfo* newEnum)
		{
			assert(newEnum != nullptr);
			jEnum.enumVec.push_back(newEnum);
			jEnum.enumNameMap.emplace(newEnum->FullName(), newEnum); 
		}
		JTypeInfo* JReflectionInfoImpl::GetTypeInfo(const std::string& name)const noexcept
		{
			auto data = jType.typeNameMap.find(name);
			return data != jType.typeNameMap.end() ? data->second : nullptr;
		} 
		JEnumInfo* JReflectionInfoImpl::GetEnumInfo(const std::string& fullname)const noexcept
		{
			auto data = jEnum.enumNameMap.find(fullname);
			return data != jEnum.enumNameMap.end() ? data->second : nullptr;
		}
		JTypeInfo* JReflectionInfoImpl::FindTypeInfo(const std::string& fullname)const noexcept
		{
			const uint typeCount = (uint)jType.typeVec.size();
			for (uint i = 0; i < typeCount; ++i)
			{
				if (jType.typeVec[i]->FullName() == fullname)
					return jType.typeVec[i];
			}
			return nullptr;
		}
		JEnumInfo* JReflectionInfoImpl::FindEnumInfo(const std::string& name)const noexcept
		{
			const uint enumCount = (uint)jEnum.enumVec.size();
			for (uint i = 0; i < enumCount; ++i)
			{
				if (jEnum.enumVec[i]->Name() == name)
					return jEnum.enumVec[i];
			}
			return nullptr;

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
	}
}