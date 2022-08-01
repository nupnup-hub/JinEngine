#include"JTypeInfo.h"  
#include"JPropertyInfo.h"    
#include"JMethodInfo.h"

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