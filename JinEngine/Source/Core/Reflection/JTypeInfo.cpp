#include"JTypeInfo.h"    
#include"JPropertyInfo.h"    
#include"JMethodInfo.h"   
#include"JTypeBase.h"   
#include"../Utility/JCommonUtility.h"    

namespace JinEngine
{
	namespace Core
	{   
		JTypeInfo::InterfaceTypeInfo::InterfaceTypeInfo(Core::JTypeInfo& interfaceType, const ConvertInterfacePtr convertPtr)
			:interfaceType(interfaceType), convertPtr(convertPtr)
		{}
		JTypeInfo::ImplTypeInfo::ImplTypeInfo(Core::JTypeInfo& implType, const ConvertImplBasePtr convertPtr)
			:implType(implType), convertPtr(convertPtr)
		{}

		std::string JTypeInfo::Name()const noexcept
		{ 
			return name;
		}
		std::string JTypeInfo::NameWithOutModifier()const noexcept
		{
			std::string withOutName = name;
			if (withOutName[0] == 'J')
				withOutName = withOutName.substr(1);
			
			int index = (int)withOutName.find("Interface");
			if (index != std::string::npos)
				return withOutName.substr(0, index);
			else
				return withOutName;
		}
		std::string JTypeInfo::FullName()const noexcept
		{
			return fullName;
		} 
		size_t JTypeInfo::TypeGuid()const noexcept
		{
			return hashCode;
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
		JTypeInfoGuiOption* JTypeInfo::GetOption()noexcept
		{  
			return &option;
		}
		uint JTypeInfo::GetInstanceCount()const noexcept
		{
			return instanceData != nullptr ? (uint)instanceData->classInstanceVec.size() : 0;
		}
		int JTypeInfo::GetInstanceIndex(IdentifierType iden)const noexcept
		{
			return instanceData != nullptr ? JCUtil::GetTypeIndex(instanceData->classInstanceVec, iden) : -1;
 		}
		JTypeBase* JTypeInfo::GetInstanceRawPtr(IdentifierType iden)const noexcept
		{
			if (instanceData == nullptr)
				return nullptr;

			auto data = instanceData->classInstanceMap.find(iden);
			return data != instanceData->classInstanceMap.end() ? data->second.Get() : nullptr;
		}
		JUserPtr<JTypeBase> JTypeInfo::GetInstanceUserPtr(IdentifierType iden)const noexcept
		{
			if (instanceData == nullptr)
				return JUserPtr<JTypeBase>{};

			auto data = instanceData->classInstanceMap.find(iden);
			return data != instanceData->classInstanceMap.end() ? JUserPtr<JTypeBase>{data->second } : JUserPtr<JTypeBase>{};
		}
		JWeakPtr<JTypeBase> JTypeInfo::GetInstanceWeakPtr(IdentifierType iden)const noexcept
		{
			if (instanceData == nullptr)
				return JWeakPtr<JTypeBase>{};

			auto data = instanceData->classInstanceMap.find(iden);
			return data != instanceData->classInstanceMap.end() ? JWeakPtr<JTypeBase>{data->second } : JWeakPtr<JTypeBase>{};
		}
		TypeInstanceVector JTypeInfo::GetInstanceRawPtrVec()const noexcept
		{
			if (instanceData == nullptr)
				return TypeInstanceVector{};

			return instanceData->classInstanceVec;
		}
		std::vector<JTypeInfo*> JTypeInfo::GetChildInfo()const noexcept
		{
			return _JReflectionInfo::Instance().GetDerivedTypeInfo(*this, false);
		}
	 	JAllocationInterface* JTypeInfo::GetAllocationInterface()const noexcept
		{ 
			return allocationInterface.get();
		}
		JAllocationInfo JTypeInfo::GetAllocInfo()const noexcept
		{
			if (allocationInterface != nullptr)
				return allocationInterface->GetInformation();
			else
			{
				if (instanceData != nullptr)
				{
					JAllocationInfo info;
					info.committedBlockCount = instanceData->classInstanceVec.size();
					info.oriBlockSize = dataSize;
					info.allocBlockSize = dataSize;
					return info;
				}
				else
					return JAllocationInfo{};		
			}
		}
		JTypeInfo* JTypeInfo::GetInterfaceTypeInfo()const noexcept
		{
			return HasInterfaceTypeInfo() ? &interfaceTypeInfo->interfaceType : nullptr;
		}
		JTypeInfo* JTypeInfo::GetImplTypeInfo()const noexcept
		{
			return HasImplTypeInfo() ? &implTypeInfo->implType : nullptr;
		}
		bool JTypeInfo::SetAllocationCreator(std::unique_ptr <JTypeAllocationCreatorInterface>&& newCreator)noexcept
		{  
			if (extraInitInfo == nullptr || !extraInitInfo->canUseAlloc || newCreator == nullptr)
				return false;

			extraInitInfo->allocInitInfo->creator = std::move(newCreator);
			return true;
		}
		bool JTypeInfo::SetAllocationOption(std::unique_ptr<JAllocationDesc>&& newOption)noexcept
		{ 
			if (extraInitInfo == nullptr || !extraInitInfo->canUseAlloc || newOption == nullptr)
				return false;

			extraInitInfo->allocInitInfo->option = std::move(newOption);
			return true;
		}
		bool JTypeInfo::SetDestructionInfo(std::unique_ptr<JLazyDestructionInfo>&& newDesInfo)noexcept
		{
			if (extraInitInfo == nullptr || !extraInitInfo->canUseLazy || newDesInfo == nullptr)
				return false;

			extraInitInfo->lazyDestructionInfo = std::move(newDesInfo);
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
		bool JTypeInfo::CanUseLazyDestruction()const noexcept
		{
			return lazyDestruction != nullptr;
		}
		bool JTypeInfo::HasInterfaceTypeInfo()const noexcept
		{
			return interfaceTypeInfo != nullptr;
		}
		bool JTypeInfo::HasImplTypeInfo()const noexcept
		{
			return implTypeInfo != nullptr;
		}
		JTypeBase* JTypeInfo::ConvertInterfaceBase(JTypeImplBase* implBase)const noexcept
		{
			if (!HasInterfaceTypeInfo())
				return nullptr;

			return interfaceTypeInfo->convertPtr(implBase);
		}
		JTypeImplBase* JTypeInfo::ConvertImplBase(JTypeBase* tBase)const noexcept
		{
			if (!HasImplTypeInfo())
				return nullptr;

			return implTypeInfo->convertPtr(tBase);
		}
		JTypeInfoGuiOption* JTypeInfo::TryGetImplOption()noexcept
		{
			return HasImplTypeInfo() ? GetImplTypeInfo()->GetOption() : GetOption();
		}
		void JTypeInfo::TryLazyDestruction(JTypeBase* tBase)noexcept
		{
			if (!CanUseLazyDestruction())
				return;
			lazyDestruction->AddUser(tBase);
		}
		void JTypeInfo::TryCancelLazyDestruction(JTypeBase* tBase)noexcept
		{
			if (!CanUseLazyDestruction())
				return;

			lazyDestruction->RemoveUser(tBase);
		}
		void JTypeInfo::UpdateLazyDestruction(const float timeOffset)noexcept
		{
			if (!CanUseLazyDestruction())
				return;

			lazyDestruction->Update(timeOffset);
		}
		bool JTypeInfo::AddInstance(IdentifierType iden, JOwnerPtr<JTypeBase>&& ptr)noexcept
		{
			if (instanceData == nullptr)
				return false; 

			if (instanceData->classInstanceMap.find(iden) == instanceData->classInstanceMap.end())
			{
				if (ptr->GetTypeInfo().IsChildOf(JTypeBase::StaticTypeInfo()))
				{ 
					auto destructionPtr = [](void* ptr)
					{  
						delete static_cast<JTypeBase*>(ptr);
						//JTypeBase::operator delete(ptr);
					};

					ptr.SetDestructionPtr(destructionPtr);
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
		/*
		JOwnerPtr<JTypeBase> JTypeInfo::ReleaseInstance(IdentifierType iden)noexcept
		{
			if (instanceData == nullptr)
				return JOwnerPtr<JTypeBase>{};

			const uint instanceCount = (uint)instanceData->classInstanceVec.size();
			for (uint i = 0; i < instanceCount; ++i)
			{
				if (instanceData->classInstanceVec[i]->GetGuid() == iden)
				{
					instanceData->classInstanceVec.erase(instanceData->classInstanceVec.begin() + i);
					JOwnerPtr<JTypeBase> owner = std::move(instanceData->classInstanceMap.find(iden)->second);
					instanceData->classInstanceMap.erase(iden);
					return owner;
				}
			}
			return JOwnerPtr<JTypeBase>{};
		}
		*/
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
			for (const auto& data : option.widgetHandleVec)
			{
				const uint widgetCount = data->GetWidgetInfoCount();
				for (uint i = 0; i < widgetCount; ++i)
					data->GetWidgetInfo(i)->TryReSettingExtraUserInfo();
			}
			callOncePtr();
		}
		void JTypeInfo::RegisterEngineDefaultAllocationOption()
		{
			if (allocationInterface != nullptr || extraInitInfo == nullptr || extraInitInfo->allocInitInfo == nullptr)
				return;
			   
			extraInitInfo->allocInitInfo->option = std::make_unique<JAllocationDesc>();  
			extraInitInfo->allocInitInfo->option->dataSize = dataSize;  
		}
		void JTypeInfo::RegisterAllocation()
		{ 
			if (isAbstractType || extraInitInfo == nullptr || !extraInitInfo->canUseAlloc)
				return;

			if (extraInitInfo->allocInitInfo == nullptr)
				extraInitInfo->allocInitInfo = std::make_unique<AllocationInitInfo>();

			if (extraInitInfo->allocInitInfo->option == nullptr)
				RegisterEngineDefaultAllocationOption();
			 
			bool useDefaultAllocation = extraInitInfo->allocInitInfo->option->allocationType == J_ALLOCATION_TYPE::DEFAULT;
			if (allocationInterface != nullptr || useDefaultAllocation)
				return;

			if (extraInitInfo->allocInitInfo->creator == nullptr)
				extraInitInfo->allocInitInfo->creator = std::make_unique<JTypeAllocationCreator>();

			//if (extraInitInfo->allocInitInfo->option->canReAlloc)
			//	extraInitInfo->allocInitInfo->option->canReAlloc = instanceData != nullptr;

			if (extraInitInfo->allocInitInfo->option->canReAlloc && extraInitInfo->allocInitInfo->option->notifyReAllocB == nullptr)
			{
				using NotifyReAllocPtr = JAllocationDesc::NotifyReAllocF::Ptr;
				using NotifyReAllocF = JAllocationDesc::NotifyReAllocF::Functor;
				using NotifyReAllocB = JAllocationDesc::NotifyReAllocB;
				using ReceiverPtr = JAllocationDesc::ReceiverPtr;
				using ReAllocatedPtr = JAllocationDesc::ReAllocatedPtr;
				using MemIndex = JAllocationDesc::MemIndex;

				NotifyReAllocPtr notifyPtr = [](ReceiverPtr receiver, ReAllocatedPtr movedPtr, MemIndex index)
				{
					auto typeInfo = static_cast<JTypeInfo*>(receiver); 
					auto iden = static_cast<JTypeBase*>(movedPtr);
					auto& ownerPtr = typeInfo->instanceData->classInstanceMap.find(iden->GetGuid())->second;
					ownerPtr.SetValidPointer(iden);

					int vecIndex = JCUtil::GetTypeIndex(typeInfo->instanceData->classInstanceVec, ownerPtr->GetGuid());
					typeInfo->instanceData->classInstanceVec[vecIndex] = ownerPtr.Get();
				};
				auto reAllocF = std::make_unique<NotifyReAllocF>(notifyPtr);
				extraInitInfo->allocInitInfo->option->notifyReAllocB = UniqueBind(std::move(reAllocF), static_cast<ReceiverPtr>(this), empty, empty);
			}

			if (extraInitInfo->allocInitInfo->option->dataCount == 0)
				extraInitInfo->allocInitInfo->option->dataCount = JAllocationDesc::initDataCount;
			if (extraInitInfo->allocInitInfo->option->dataSize < dataSize)
				extraInitInfo->allocInitInfo->option->dataSize = dataSize;

			//Debug
			extraInitInfo->allocInitInfo->option->name = Name();
			allocationInterface = extraInitInfo->allocInitInfo->creator->CreateAlloc(extraInitInfo->allocInitInfo->option.get());
			allocationInterface->Initialize(std::move(*extraInitInfo->allocInitInfo->option));
		}
		void JTypeInfo::DeRegisterAllocation()
		{
			if (allocationInterface != nullptr)
				allocationInterface->Release();
			allocationInterface = nullptr; 
		}
		void JTypeInfo::RegisterLazyDestructionInfo()
		{
			if (isAbstractType || extraInitInfo == nullptr || !extraInitInfo->canUseLazy)
				return;
			 
			if (extraInitInfo->lazyDestructionInfo  == nullptr|| extraInitInfo->lazyDestructionInfo->executeDestroy == nullptr)
				return;
	 
			if (lazyDestruction != nullptr)
				return;
			 
			lazyDestruction = std::make_unique<JLazyDestruction>(std::move(extraInitInfo->lazyDestructionInfo));
		}
		void JTypeInfo::RegisterInterfaceTypeInfo(std::unique_ptr<InterfaceTypeInfo>&& interfaceTypeInfo)
		{
			JTypeInfo::interfaceTypeInfo = std::move(interfaceTypeInfo); 
		}
		void JTypeInfo::RegisterImplTypeInfo(std::unique_ptr<ImplTypeInfo>&& implTypeInfo)
		{ 
			JTypeInfo::implTypeInfo = std::move(implTypeInfo);
		}
		void JTypeInfo::EndRegister()
		{
			extraInitInfo.reset();
		}
		JTypeInstanceSearchHint::JTypeInstanceSearchHint()
			:typeGuid(0), objectGuid(0), isValid(false), hasImplType(false)
		{}
		JTypeInstanceSearchHint::JTypeInstanceSearchHint(const JTypeInfo& info, const size_t guid)
			: typeGuid(info.TypeGuid()), objectGuid(guid), isValid(true), hasImplType(info.HasImplTypeInfo())
		{}
		JTypeInstanceSearchHint::JTypeInstanceSearchHint(JUserPtr<JTypeBase> iden)
			: typeGuid(iden.IsValid() ? iden->GetTypeInfo().TypeGuid() : 0),
			objectGuid(iden.IsValid() ? iden->GetGuid() : 0),
			isValid(iden.IsValid() ? true : false), 
			hasImplType(iden.IsValid() ? iden->GetTypeInfo().HasImplTypeInfo() : false)
		{}
		JTypeInstanceSearchHint::~JTypeInstanceSearchHint()
		{  
		}
	}
}