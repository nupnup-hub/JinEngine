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
			jType.push_back(newType);		 
		}
		void JReflectionImpl::AddEnum(JEnumInfo* newEnum)
		{
			assert(newEnum != nullptr);
			jEnum.emplace(newEnum->FullName(), newEnum);
		}
		JTypeInfo* JReflectionImpl::GetTypeInfo(const std::string& name)
		{
			const uint typeCount = (uint)jType.size();
			for (uint i = 0; i < typeCount; ++i)
			{
				if (jType[i]->FullName() == name)
					return jType[i];
			}
			return nullptr;
		}
		JEnumInfo* JReflectionImpl::GetEnumInfo(const std::string& name)
		{
			auto data = jEnum.find(name);
			return data != jEnum.end() ? data->second : nullptr;
		}
		std::vector<JTypeInfo*> JReflectionImpl::GetDerivedTypeInfo(const JTypeInfo& baseType)
		{
			std::vector<JTypeInfo*> res;
			const uint typeCount = (uint)jType.size();
			for (uint i = 0; i < typeCount; ++i)
			{
				if (jType[i]->IsA(baseType))
					continue;

				if (jType[i]->IsChildOf(baseType))
					res.push_back(jType[i]);
			}
			return res;
		}
	}
}