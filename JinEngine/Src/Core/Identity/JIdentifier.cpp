#include"JIdentifier.h"
#include"JIdentifierPrivate.h"  
#include"../Reflection/JTypeImplBase.h"
#include"../Reflection/JTypeBasePrivate.h"
#include"../Guid/GuidCreator.h"
#include"../../Utility/JCommonUtility.h"

#define INVALID_NAME L""

//Debug
#include<fstream>
#include"../../Object/Component/RenderItem/JRenderItem.h"
#include"../../Object/Component/Transform/JTransform.h"

namespace JinEngine
{
	namespace Core
	{
		namespace
		{
			using InstanceInterface = JTypeBasePrivate::InstanceInterface;
		}
		namespace
		{
			static std::unordered_map<size_t, JIdentifierPrivate&> pMap;
		}
		//Caution
		//fixed variable name 
		//thisPointer and impl
		//it is used by macro
		class JIdentifier::JIdentifierImpl : public JTypeImplBase
		{
			REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JIdentifierImpl)
		public:
			JWeakPtr<JIdentifier> thisPointer;
		public:
			std::wstring name;  
			bool isReleased = false;
		public:
			//Caution
			//Impl생성자에서 interface class 참조시 interface class가 함수내에서 impl을 참조할 경우 error
			//impl이 아직 생성되지 않았으므로
			JIdentifierImpl(const InitData& initData)
				:name(initData.name)
			{}
		public:
			void SetReleasedTrigger(const bool value)noexcept
			{
				isReleased = value;
			}
		public:
			bool IsReleased()const noexcept
			{
				return isReleased;
			}
		public:
			static bool BeginDestroy(JIdentifier* ptr, const bool isForced)
			{
				if (ptr == nullptr)
					return false;

				auto& destroyInterface = ptr->PrivateInterface().GetDestroyInstanceInterface();
				if (destroyInterface.CanDestroyInstancce(ptr, isForced))
				{   
					if (ptr->impl->IsReleased())
					{
						//	already execute
						//	destroyInterface.SetInvalidInstance(ptr);
						//	destroyInterface.DeRegisterCash(ptr);
						//	InstanceInterface::RemoveInstance(ptr);
						destroyInterface.Clear(ptr, isForced);
						ptr->impl->SetReleasedTrigger(false);
						return true;
					}
					else
					{
						destroyInterface.SetInvalidInstance(ptr);
						destroyInterface.DeRegisterCash(ptr);
						destroyInterface.Clear(ptr, isForced);
						return InstanceInterface::RemoveInstance(ptr);
					}
				}
				else
					return false;
			}
		public:
			void RegisterThisPointer(JIdentifier* iden)
			{
				thisPointer = Core::GetWeakPtr(iden);
			}
			static void RegisterTypeData()
			{
				IMPL_REALLOC_BIND(JIdentifier::JIdentifierImpl, thisPointer)
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
		std::wstring JIdentifier::GetDefaultName(const JTypeInfo& info)noexcept
		{ 
			return L"New" + std::wstring(JCUtil::StrToWstr(info.NameWithOutModifier())).substr(1);
		}
		JIdentifierPrivate* JIdentifier::PrivateInterface(const size_t typeGuid)noexcept
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
		bool JIdentifier::BeginCopy(JUserPtr<JIdentifier> from, JUserPtr<JIdentifier> to)
		{
			if (from == nullptr)
				return false;

			auto& copyInterface = from->PrivateInterface().GetCreateInstanceInterface();
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
			:JTypeBase(initData.guid), impl(std::make_unique<JIdentifierImpl>(initData))
		{ 
		}
		JIdentifier::~JIdentifier()
		{
			impl.reset();
		}

		using CreateInstanceInterface = JIdentifierPrivate::CreateInstanceInterface;
		using DestroyInstanceInterface = JIdentifierPrivate::DestroyInstanceInterface; 
		using ReleaseInterface = JIdentifierPrivate::ReleaseInterface;

		JUserPtr<JIdentifier> CreateInstanceInterface::BeginCreate(std::unique_ptr<JDITypeDataBase>&& initData, JIdentifierPrivate* pInterface)noexcept
		{
			if (initData == nullptr || !initData->GetTypeInfo().IsChildOf(JIdentifier::InitData::StaticTypeInfo()))
				return nullptr;
			 
			auto& createInterface = pInterface->GetCreateInstanceInterface();
			if (createInterface.CanCreateInstance(initData.get()))
			{ 
				auto ownerPtr = createInterface.Create(initData.get());
				auto rawPtr = ownerPtr.Get();
				InstanceInterface::AddInstance(std::move(ownerPtr));
				createInterface.Initialize(rawPtr, initData.get());
				createInterface.RegisterCash(rawPtr);
				createInterface.SetValidInstance(rawPtr);
				return Core::GetUserPtr(rawPtr);
			}
			else
				return nullptr;
		}
		JUserPtr<JIdentifier> CreateInstanceInterface::BeginCreateAndCopy(std::unique_ptr<JDITypeDataBase>&& initData, JUserPtr<JIdentifier> from)noexcept
		{
			if (initData == nullptr || !initData->IsValidData() || from == nullptr)
				return nullptr;

			JIdentifier::InitData* idenInitData = static_cast<JIdentifier::InitData*>(initData.get());
			JIdentifierPrivate* fromPI = &from->PrivateInterface();
			JIdentifierPrivate* toPI = JIdentifier::PrivateInterface(idenInitData->GetTypeInfo().TypeGuid());
			if (fromPI != toPI)
				return nullptr;

			JUserPtr<JIdentifier> to = BeginCreate(std::move(initData), toPI);
			if (!JIdentifier::BeginCopy(from, to))
			{
				if (to != nullptr)
					JIdentifier::BeginForcedDestroy(to.Get());
				return nullptr;
			}
			return to;
		}

		bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
		{
			return initData != nullptr && initData->IsValidData();
		} 
		void CreateInstanceInterface::Initialize(JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept
		{ 
			createdPtr->impl->RegisterThisPointer(createdPtr);
		} 
		void CreateInstanceInterface::RegisterCash(JIdentifier* createdPtr)noexcept {}
		void CreateInstanceInterface::SetValidInstance(JIdentifier* createdPtr)noexcept {}
		bool CreateInstanceInterface::CanCopy(JUserPtr<JIdentifier> from, JUserPtr<JIdentifier> to)noexcept
		{
			return from != nullptr && to != nullptr && from->GetTypeInfo().IsA(to->GetTypeInfo());
		}
		bool DestroyInstanceInterface::CanDestroyInstancce(JIdentifier* ptr, const bool isForced)const noexcept
		{
			return ptr != nullptr;
		} 
		 
		void DestroyInstanceInterface::Clear(JIdentifier* ptr, const bool isForced){}
		void DestroyInstanceInterface::DeRegisterCash(JIdentifier* ptr)noexcept {}
		void DestroyInstanceInterface::SetInvalidInstance(JIdentifier* ptr)noexcept{}
	 
		JOwnerPtr<JTypeBase> ReleaseInterface::ReleaseInstance(JIdentifier* ptr)
		{
			if (ptr == nullptr)
				return JOwnerPtr<JIdentifier>{};

			auto& dI = ptr->PrivateInterface().GetDestroyInstanceInterface();
			dI.SetInvalidInstance(ptr);
			dI.DeRegisterCash(ptr);
			ptr->impl->SetReleasedTrigger(true);
			return InstanceInterface::ReleaseInstance(ptr);
		}
		bool ReleaseInterface::RestoreInstance(JOwnerPtr<JTypeBase>&& instance)
		{
			if (instance == nullptr)
				return false;

			if (!instance->GetTypeInfo().IsChildOf<JIdentifier>())
				return false;

			auto rawPtr = static_cast<JIdentifier*>(instance.Get());
			auto& cI = rawPtr->PrivateInterface().GetCreateInstanceInterface();
			bool res = InstanceInterface::AddInstance(std::move(instance));
			if (res)
			{
				cI.RegisterCash(rawPtr);
				cI.SetValidInstance(rawPtr);
				rawPtr->impl->SetReleasedTrigger(false);
			}
			return res;
		}
	}
}