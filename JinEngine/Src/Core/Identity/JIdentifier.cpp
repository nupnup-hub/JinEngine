#include"JIdentifier.h"
#include"JIdentifierPrivate.h" 
#include"JIdentifierImplBase.h" 
#include"../Guid/GuidCreator.h"
#include"../../Utility/JCommonUtility.h"

#define INVALID_NAME L""

namespace JinEngine
{
	namespace Core
	{
		namespace
		{
			static std::unordered_map<size_t, JIdentifierPrivate&> pMap;
		}
		class JIdentifier::JIdentifierImpl : public JIdentifierImplBase
		{
			REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JIdentifierImpl)
		public:
			std::wstring name;
			size_t guid;
		public:
			//Caution
			//Impl생성자에서 interface class 참조시 interface class가 함수내에서 impl을 참조할 경우 error
			//impl이 아직 생성되지 않았으므로
			JIdentifierImpl(const InitData& initData)
				:name(initData.name), guid(initData.guid)
			{}
		public:
			static bool BeginDestroy(JIdentifier* ptr, const bool isForced)
			{
				if (ptr == nullptr)
					return false;

				auto& destroyInterface = ptr->GetPrivateInterface().GetDestroyInstanceInterface();
				if (destroyInterface.CanDestroyInstancce(ptr, isForced))
				{
					destroyInterface.PrepareDestroyInstance(ptr);
					destroyInterface.SetInvalidInstance(ptr);
					destroyInterface.DeRegisterCash(ptr);
					destroyInterface.Clear(ptr, isForced);
					return destroyInterface.RemoveInstance(ptr);
				}
				else
					return false;
			}
		};

		JIdentifier::InitData::InitData(const JTypeInfo& initTypeInfo)
			:initTypeInfo(initTypeInfo), name(GetDefaultName(initTypeInfo)), guid(MakeGuid())
		{}
		JIdentifier::InitData::InitData(const JTypeInfo& initTypeInfo, const size_t guid)
			: initTypeInfo(initTypeInfo), name(GetDefaultName(initTypeInfo)), guid(guid)
		{}
		JIdentifier::InitData::InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid)
			: initTypeInfo(initTypeInfo), name(name), guid(guid)
		{}
		const JTypeInfo& JIdentifier::InitData::InitDataTypeInfo()const noexcept
		{
			return initTypeInfo;
		}
		bool JIdentifier::InitData::IsValidData()const noexcept
		{
			//isn't overlapped guid
			return name != INVALID_NAME && Core::GetRawPtr(initTypeInfo.TypeGuid(), guid) == nullptr;
		}

		std::wstring JIdentifier::GetName() const noexcept
		{
			return impl->name;
		}
		std::wstring JIdentifier::GetNameWithType()const noexcept
		{
			return L"Type: " + JCUtil::U8StrToWstr(GetTypeInfo().NameWithOutModifier()) + L"\nName: " + GetName();
		}
		size_t JIdentifier::GetGuid()const noexcept
		{
			return impl->guid;
		}
		std::wstring JIdentifier::GetDefaultName(const JTypeInfo& info)noexcept
		{
			return L"New" + std::wstring(JCUtil::StrToWstr(info.NameWithOutModifier())).substr(1);
		}
		JIdentifierPrivate* JIdentifier::GetPrivateInterface(const size_t typeGuid)noexcept
		{
			auto pIData = pMap.find(typeGuid);
			if (pIData != pMap.end())
				return &pIData->second;
			else
				return nullptr;
		}

		void JIdentifier::SetName(const std::wstring& newName)noexcept
		{
			if (!newName.empty())
				impl->name = newName;
		}
		bool JIdentifier::BeginCopy(JIdentifier* from, JIdentifier* to)
		{
			if (from == nullptr)
				return false;

			auto& copyInterface = from->GetPrivateInterface().GetCreateInstanceInterface();
			if (!copyInterface.CanCopy(from, to))
				return false;

			return copyInterface.Copy(from, to);
		}
		bool JIdentifier::BeginDestroy(JIdentifier* ptr)
		{
			return JIdentifierImpl::BeginDestroy(ptr, false);
		}
		bool JIdentifier::BeginForcedDestroy(JIdentifier* ptr)
		{
			return JIdentifierImpl::BeginDestroy(ptr, true);
		}
		void JIdentifier::RegisterPrivateInterface(const JTypeInfo& info, JIdentifierPrivate& p)
		{
			pMap.emplace(info.TypeGuid(), p);
		}
		JIdentifier::JIdentifier(const InitData& initData)
			:impl(std::make_unique<JIdentifierImpl>(initData))
		{}
		JIdentifier::~JIdentifier()
		{
			impl.reset();
		}

		using CreateInstanceInterface = JIdentifierPrivate::CreateInstanceInterface;
		using DestroyInstanceInterface = JIdentifierPrivate::DestroyInstanceInterface; 
		using ReleaseInterface = JIdentifierPrivate::ReleaseInterface;

		JIdentifier* CreateInstanceInterface::BeginCreate(std::unique_ptr<JDITypeDataBase>&& initData, JIdentifierPrivate* pInterface)noexcept
		{
			if (initData == nullptr || !initData->GetTypeInfo().IsChildOf(JIdentifier::InitData::StaticTypeInfo()))
				return nullptr;
			 
			auto& createInterface = pInterface->GetCreateInstanceInterface();
			if (createInterface.CanCreateInstance(initData.get()))
			{
				createInterface.PrepareCreateInstance();
				auto ownerPtr = createInterface.Create(std::move(initData));
				auto rawPtr = ownerPtr.Get(); 
				createInterface.RegisterCash(rawPtr);
				createInterface.SetValidInstance(rawPtr);
				createInterface.AddInstance(std::move(ownerPtr));
				return rawPtr;
			}
			else
				return nullptr;
		}
		JIdentifier* CreateInstanceInterface::BeginCreateAndCopy(std::unique_ptr<JDITypeDataBase>&& initData, JIdentifier* from)noexcept
		{
			if (initData == nullptr || !initData->IsValidData() || from == nullptr)
				return nullptr;

			JIdentifier::InitData* idenInitData = static_cast<JIdentifier::InitData*>(initData.get());
			JIdentifierPrivate* fromPI = &from->GetPrivateInterface();
			JIdentifierPrivate* toPI = JIdentifier::GetPrivateInterface(idenInitData->GetTypeInfo().TypeGuid());
			if (fromPI != toPI)
				return nullptr;

			JIdentifier* to = static_cast<JIdentifier*>(BeginCreate(std::move(initData), toPI));
			if (!JIdentifier::BeginCopy(from, to))
			{
				if (to != nullptr)
					JIdentifier::BeginForcedDestroy(to);
				return nullptr;
			}
			return to;
		}

		bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
		{
			return initData != nullptr && initData->IsValidData();
		}
		void CreateInstanceInterface::PrepareCreateInstance()noexcept {}
		void CreateInstanceInterface::SetValidInstance(JIdentifier* createdPtr)noexcept {}
		void CreateInstanceInterface::RegisterCash(JIdentifier* createdPtr)noexcept {}
		bool CreateInstanceInterface::AddInstance(Core::JOwnerPtr<JIdentifier>&& createdOwner)noexcept
		{
			JIdentifier* rawPtr = createdOwner.Get(); 
			return rawPtr->GetTypeInfo().AddInstance(rawPtr->GetGuid(), std::move(createdOwner));
		}
		bool CreateInstanceInterface::CanCopy(JIdentifier* from, JIdentifier* to)noexcept
		{
			return from != nullptr && to != nullptr && from->GetTypeInfo().IsA(to->GetTypeInfo());
		}
		bool DestroyInstanceInterface::CanDestroyInstancce(JIdentifier* ptr, const bool isForced)const noexcept
		{
			return ptr != nullptr;
		} 

		void DestroyInstanceInterface::PrepareDestroyInstance(JIdentifier* ptr)noexcept {}
		void DestroyInstanceInterface::DeRegisterCash(JIdentifier* ptr)noexcept {}
		void DestroyInstanceInterface::SetInvalidInstance(JIdentifier* ptr)noexcept{}
		bool DestroyInstanceInterface::RemoveInstance(JIdentifier* ptr)noexcept
		{
			return ptr->GetTypeInfo().RemoveInstance(ptr->GetGuid());
		}
		/*
			template<typename T, std::enable_if_t<std::is_base_of_v<JIdentifier, T>, int> = 0>
			static Core::JOwnerPtr<JIdentifier> ReleaseInstance(const size_t guid)noexcept
			{
				auto ownerPtr = T::StaticTypeInfo().ReleaseInstance(guid);
				if (ownerPtr.IsValid())
					ownerPtr->DeRegisterInstance();
				return ownerPtr;
			}
			static Core::JOwnerPtr<JIdentifier> ReleaseInstance(Core::JIdentifier* rawPtr)noexcept
			{
				if (rawPtr != nullptr)
				{
					auto ownerPtr = rawPtr->GetTypeInfo().ReleaseInstance(rawPtr->GetGuid());
					if (ownerPtr.IsValid())
						ownerPtr->DeRegisterInstance();
					return ownerPtr;
				}
				else
					return  Core::JOwnerPtr<JIdentifier>{};
			}
		*/
		Core::JOwnerPtr<JIdentifier> ReleaseInterface::ReleaseInstance(JIdentifier* ptr)
		{
			if (ptr == nullptr)
				return Core::JOwnerPtr<JIdentifier>{};

			auto& dI = ptr->GetPrivateInterface().GetDestroyInstanceInterface();
			dI.SetInvalidInstance(ptr);
			dI.DeRegisterCash(ptr);
			return ptr->GetTypeInfo().ReleaseInstance(ptr->GetGuid());
		}
		bool ReleaseInterface::RestoreInstance(Core::JOwnerPtr<JIdentifier>&& instance)
		{
			if (instance == nullptr)
				return false;

			auto rawPtr = instance.Get();
			auto& cI = rawPtr->GetPrivateInterface().GetCreateInstanceInterface();
			cI.RegisterCash(rawPtr);
			cI.SetValidInstance(rawPtr);
			return rawPtr->GetTypeInfo().AddInstance(rawPtr->GetGuid(), std::move(instance));
		}
	}
}