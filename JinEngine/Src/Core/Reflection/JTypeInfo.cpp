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
			if (name[0] == 'J')
				return name.substr(1);
			else
				return name;
		}
		std::string JTypeInfo::FullName()const noexcept
		{
			return fullName;
		} 
		size_t JTypeInfo::TypeGuid()const noexcept
		{
			return std::hash<std::string>{}(fullName);
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
		JTypeInfoGuiOption* JTypeInfo::GetOption() noexcept
		{  
			return &option;
		}
		uint JTypeInfo::GetInstanceCount()const noexcept
		{
			return instanceData != nullptr ? (uint)instanceData->classInstanceVec.size() : 0;
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
	 	JAllocationInterface* JTypeInfo::GetAllocationInterface()noexcept
		{ 
			return allocationInterface.get();
		}
		bool JTypeInfo::SetAllocationCreator(std::unique_ptr <JTypeAllocationCreatorInterface>&& newCreator)noexcept
		{
			if (isRegisteredAllocation || newCreator == nullptr)
				return false;

			allocationCreator = std::move(newCreator);
			return true;
		}
		bool JTypeInfo::SetAllocationOption(std::unique_ptr<JTypeAllocationOption>&& newOption)noexcept
		{
			if (isRegisteredAllocation || allocationInterface != nullptr)
				return false;

			allocationOption = std::move(newOption);
			return true;
		}
		bool JTypeInfo::IsAbstractType()const noexcept
		{
			return isAbstractType;
		}
		bool JTypeInfo::IsLeafType()const noexcept
		{ 
			return isLeafType;
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
		void JTypeInfo::ExecuteTypeCallOnece()
		{
			callOncePtr();
		}
		void JTypeInfo::RegisterEngineDefaultAllocationOption()
		{
			if (allocationInterface != nullptr || allocationOption != nullptr)
				return;

			if (isRegisteredAllocation)
				return;

			J_ALLOCATION_TYPE type;
			uint blockCount = 10000;
			uint reservePageCount;
			size_t pageSize;
			JAllocationInterface::CalculatePageFitAllocationData(dataSize, blockCount, pageSize, reservePageCount);
 
			if (dataSize >= 1 << 20)
				allocationOption = std::make_unique<JTypeAllocationOption>(J_ALLOCATION_TYPE::HEAP, dataSize, blockCount);
			else
			{
				//virtual test
				allocationOption = std::make_unique<JTypeAllocationOption>(J_ALLOCATION_TYPE::HEAP, dataSize, blockCount);
				//allocationOption = std::make_unique<JTypeAllocationOption>(J_ALLOCATION_TYPE::HEAP, dataSize, blockCount);
			}		 
		}
		void JTypeInfo::RegisterAllocation()
		{ 
			if (!isRegisteredAllocation)
				RegisterEngineDefaultAllocationOption();

			isRegisteredAllocation = true;
			bool useDefaultAllocation = allocationOption->allocationType == J_ALLOCATION_TYPE::DEFAULT;
			if (allocationInterface != nullptr || useDefaultAllocation)
				return;

			allocationInterface = allocationCreator->CreateAlloc(allocationOption.get());
			allocationInterface->Initialize(allocationOption->allocDataCount, allocationOption->dataSize);
		}
		void JTypeInfo::DeRegisterAllocation()
		{
			if (allocationInterface != nullptr)
				allocationInterface->Release();
			allocationInterface = nullptr;
			isRegisteredAllocation = false;
		}
		JTypeInstanceSearchHint::JTypeInstanceSearchHint()
			:typeGuid(0), objectGuid(0), isValid(false)
		{}
		JTypeInstanceSearchHint::JTypeInstanceSearchHint(const JTypeInfo& info, const size_t guid)
			: typeGuid(info.TypeGuid()), objectGuid(guid), isValid(true)
		{}
		JTypeInstanceSearchHint::JTypeInstanceSearchHint(Core::JUserPtr<JIdentifier> iden)
			: typeGuid(iden.IsValid() ? iden->GetTypeInfo().TypeGuid() : 0),
			objectGuid(iden.IsValid() ? iden->GetGuid() : 0),
			isValid(iden.IsValid() ? true : false)
		{}
	}
}