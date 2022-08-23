#include"JReflectionInfo.h"
#include"JTypeInfo.h"
#include"JEnumInfo.h"
#include"../JDataType.h"

namespace JinEngine
{
	namespace Core
	{
		void JReflectionImpl::AddType(JTypeInfo* newType)
		{
			assert(newType != nullptr);
			jType.typeVec.push_back(newType);
			jType.typeMap.emplace(newType->Name(), newType);
		}
		void JReflectionImpl::AddEnum(JEnumInfo* newEnum)
		{
			assert(newEnum != nullptr);
			jEnum.emplace(newEnum->FullName(), newEnum);
		}
		JTypeInfo* JReflectionImpl::GetTypeInfo(const std::string& name)
		{
			auto data = jType.typeMap.find(name);
			return data != jType.typeMap.end() ? data->second : nullptr;
		}
		JEnumInfo* JReflectionImpl::GetEnumInfo(const std::string& fullname)
		{
			auto data = jEnum.find(fullname);
			return data != jEnum.end() ? data->second : nullptr;
		}
		std::vector<JTypeInfo*> JReflectionImpl::GetDerivedTypeInfo(const JTypeInfo& baseType)
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
	}
}