#include"JTypeInfo.h"  
#include"JPropertyInfo.h"    
#include"JMethodInfo.h"
#include"../../Object/JObject.h"

namespace JinEngine
{
	namespace Core
	{
		std::string JTypeInfo::Name()const noexcept
		{
			return name;
		}
		std::string JTypeInfo::FullName()const noexcept
		{
			return fullName;
		}
		const PropertyMap* JTypeInfo::GetPropertyMap()const noexcept
		{
			return memberData != nullptr ? &memberData->propertyInfo : nullptr;
		}
		const MethodMap* JTypeInfo::GetMethodMap()const noexcept
		{
			return memberData != nullptr ? &memberData->methodInfo : nullptr;
		}
		bool JTypeInfo::IsA(const JTypeInfo& tar)const noexcept
		{
			if (this == &tar)
				return true;

			return hashCode == tar.hashCode;
		}
		bool JTypeInfo::IsChildOf(const JTypeInfo& parentCandidate)const noexcept
		{
			if (IsA(parentCandidate))
				return true;

			for (const JTypeInfo* p = parent; p != nullptr; p = p->parent)
			{
				if (p->IsA(parentCandidate))
					return true;
			}
			return false;
		}
		JTypeInstance* JTypeInfo::GetInstanceRawPtr(IdentifierType iden)noexcept
		{
			if (instanceData == nullptr)
				return nullptr;

			auto data = instanceData->classInstanceMap.find(iden);
			return data != instanceData->classInstanceMap.end() ? data->second.Get() : nullptr;
		}
		JUserPtr<JTypeInstance> JTypeInfo::GetInstanceUserPtr(IdentifierType iden)noexcept
		{
			if (instanceData == nullptr)
				return JUserPtr<JTypeInstance>{};

			auto data = instanceData->classInstanceMap.find(iden);
			return data != instanceData->classInstanceMap.end() ? JUserPtr<JTypeInstance>{data->second } : JUserPtr<JTypeInstance>{};
		}
		bool JTypeInfo::AddInstance(IdentifierType iden, JOwnerPtr<JTypeInstance> ptr)noexcept
		{
			if (instanceData == nullptr)
				return false; 

			if (instanceData->classInstanceMap.find(iden) == instanceData->classInstanceMap.end())
			{
				instanceData->classInstanceVec.push_back(ptr.Get());
				instanceData->classInstanceMap.emplace(iden, ptr);
				return true;
			}
			else
				return false;
		}
		bool JTypeInfo::RemoveInstance(IdentifierType iden)noexcept
		{
			if (instanceData == nullptr)
				return false;
			 
			const uint instanceCount = (uint)instanceData->classInstanceVec.size();
			for (uint i = 0; i < instanceCount; ++i)
			{
				if (instanceData->classInstanceVec[i]->GetGuid() == iden)
				{
					instanceData->classInstanceVec.erase(instanceData->classInstanceVec.begin() + i);
					instanceData->classInstanceMap.erase(iden);
					return true;
				}
			}
			return false;
		}
		bool JTypeInfo::AddPropertyInfo(JPropertyInfo* newProperty)
		{
			if (memberData == nullptr)
				memberData = std::make_unique< JTypeMemberData>();

			assert(newProperty != nullptr);
			if (memberData->propertyInfo.find(newProperty->name) == memberData->propertyInfo.end())
			{
				memberData->propertyInfo.emplace(newProperty->name, newProperty);
				return true;
			}
			else
				return false;
		}
		bool JTypeInfo::AddMethodInfo(JMethodInfo* newMethod)
		{
			if (memberData == nullptr)
				memberData = std::make_unique< JTypeMemberData>();

			assert(newMethod != nullptr);
			if (memberData->methodInfo.find(newMethod->identificationName) == memberData->methodInfo.end())
			{
				memberData->methodInfo.emplace(newMethod->identificationName, newMethod);
				return true;
			}
			else
				return false;
		}
	}
}