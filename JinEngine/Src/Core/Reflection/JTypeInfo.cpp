#include"JTypeInfo.h"    
#include"JPropertyInfo.h"    
#include"JMethodInfo.h"
#include"../Pointer/JOwnerPtr.h"
#include"../../Object/JObject.h"

namespace JinEngine
{
	namespace Core
	{
		std::string JTypeInfo::Name()const noexcept
		{
			return name;
		}
		std::string JTypeInfo::NameWithOutPrefix()const noexcept
		{
			return name.substr(1);
		}
		std::string JTypeInfo::FullName()const noexcept
		{
			return fullName;
		}
		const PropertyVec JTypeInfo::GetPropertyVec()const noexcept
		{ 
			return memberData != nullptr ? memberData->propertyInfoVec : PropertyVec{};
		}
		const MethodVec JTypeInfo::GetMethodVec()const noexcept
		{
			return memberData != nullptr ? memberData->methodInfoVec : MethodVec{};
		}
		JTypeInfo* JTypeInfo::GetParent()const noexcept
		{
			return parent;
		}
		JPropertyInfo* JTypeInfo::GetProperty(const std::string& name)const noexcept
		{
			if (memberData != nullptr)
			{
				auto data = memberData->propertyInfoMap.find(name);
				return data != memberData->propertyInfoMap.end() ? data->second : nullptr;
			}  
			else
				return nullptr;
		}
		JMethodInfo* JTypeInfo::GetMethod(const std::string& name)const noexcept
		{
			if (memberData != nullptr)
			{
				auto data = memberData->methodInfoMap.find(name);
				return data != memberData->methodInfoMap.end() ? data->second : nullptr;
			}
			else
				return nullptr;
		}
		JTypeInfoOption* JTypeInfo::GetOption() noexcept
		{
			return &option;
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
				if (ptr->GetTypeInfo().IsChildOf(JIdentifier::StaticTypeInfo()))
				{ 
					instanceData->classInstanceVec.push_back(ptr.Get());
					instanceData->classInstanceMap.emplace(iden, std::move(ptr));
					return true;
				}
				else
					return false;
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
		JOwnerPtr<JTypeInstance> JTypeInfo::ReleaseInstance(IdentifierType iden)noexcept
		{
			if (instanceData == nullptr)
				return JOwnerPtr<JTypeInstance>{};

			const uint instanceCount = (uint)instanceData->classInstanceVec.size();
			for (uint i = 0; i < instanceCount; ++i)
			{
				if (instanceData->classInstanceVec[i]->GetGuid() == iden)
				{
					instanceData->classInstanceVec.erase(instanceData->classInstanceVec.begin() + i);
					JOwnerPtr<JTypeInstance> owner = std::move(instanceData->classInstanceMap.find(iden)->second);
					instanceData->classInstanceMap.erase(iden);
					return owner;
				}
			}
			return JOwnerPtr<JTypeInstance>{};
		}
		bool JTypeInfo::AddPropertyInfo(JPropertyInfo* newProperty)
		{
			if (memberData == nullptr)
				memberData = std::make_unique< JTypeMemberData>();

			assert(newProperty != nullptr);
			if (memberData->propertyInfoMap.find(newProperty->name) == memberData->propertyInfoMap.end())
			{
				memberData->propertyInfoVec.push_back(newProperty);
				memberData->propertyInfoMap.emplace(newProperty->name, newProperty);
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
			if (memberData->methodInfoMap.find(newMethod->identificationName) == memberData->methodInfoMap.end())
			{
				memberData->methodInfoVec.push_back(newMethod);
				memberData->methodInfoMap.emplace(newMethod->identificationName, newMethod);
				return true;
			}
			else
				return false;
		}
	}
}