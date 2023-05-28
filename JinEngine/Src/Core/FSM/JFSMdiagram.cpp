#include"JFSMdiagram.h"
#include"JFSMdiagramPrivate.h"
#include"JFSMstate.h"  
#include"JFSMstatePrivate.h"  
#include"JFSMtransition.h"  
#include"JFSMownerInterface.h"
#include"JFSMparameterValueType.h"
#include"JFSMparameterStorageAccess.h"   
#include"../Guid/GuidCreator.h"
#include"../Reflection/JTypeImplBase.h"
#include"../../Utility/JCommonUtility.h"
#include<memory>
#include<string>
#include<unordered_map>

namespace JinEngine
{
	namespace Core
	{
		namespace
		{
			using _StateUpdateInterface = JFSMstatePrivate::UpdateInterface;
		}
		namespace
		{
			static JFSMdiagramPrivate dPrivate;
			_StateUpdateInterface* StateUpdateInterface(const JUserPtr<JFSMstate>& state)
			{
				return &static_cast<_StateUpdateInterface&>(static_cast<JFSMstatePrivate&>(state->PrivateInterface()).GetUpdateInterface());
			}
		}
 
		class JFSMdiagram::JFSMdiagramImpl : public JTypeImplBase,
			public JFSMparameterStorageUserInterface
		{
			REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JFSMdiagramImpl)
		public:
			JWeakPtr<JFSMdiagram> thisPointer = nullptr;
		public:
			//vector + unorered map => 64bit overhead
			JFSMdiagramOwnerInterface* ownerInterface;
			std::vector<JUserPtr<JFSMstate>> stateVec; 
			std::unordered_map<size_t, JUserPtr<JFSMstate>> stateMap;
			JUserPtr<JFSMstate> initState = nullptr;
			size_t nowStateGuid;
		public:
			JFSMdiagramImpl(const InitData& initData, JFSMdiagram* thisDiagramRaw)
				:ownerInterface(initData.ownerInterface)
			{}
			~JFSMdiagramImpl(){}
		public:
			JUserPtr<JFSMstate> GetNowState()const noexcept
			{
				auto data = stateMap.find(nowStateGuid);
				if (data != stateMap.end())
					return data->second;
				else
					return nullptr;
			}
			JUserPtr<JFSMstate> GetState(const size_t guid)const noexcept
			{
				auto data = stateMap.find(guid);
				if (data != stateMap.end())
					return data->second;
				else
					return nullptr;
			}
			JUserPtr<JFSMstate> GetStateByIndex(const uint index)const noexcept
			{
				if (index < stateVec.size())
					return stateVec[index];
				else
					return nullptr;
			}
			std::vector<JUserPtr<JFSMparameter>> GetStorageParameter()const noexcept
			{
				return ownerInterface->GetParameterStorageUser()->GetParameterVec();
			}
		public:
			bool CanUseParameter(const size_t paramGuid)const noexcept
			{
				return ownerInterface->GetParameterStorageUser()->GetParameter(paramGuid) != nullptr;
			}
		public:
			void Initialize()noexcept
			{
				const uint stateVecSize = (uint)stateVec.size();
				for (uint i = 0; i < stateVecSize; ++i)
					StateUpdateInterface(stateVec[i])->Initialize(stateVec[i]);
			}
			void Clear()noexcept
			{
				std::vector<JUserPtr<JFSMstate>> copy = stateVec;
				for(auto& data : copy)
					JFSMinterface::BeginDestroy(data.Get()); 
				initState = nullptr;
				stateMap.clear();
				stateVec.clear();
			}
		public:
			void NotifyRemoveParameter(const size_t guid)noexcept
			{
				const uint stateVecCount = (uint)stateVec.size();
				for (uint i = 0; i < stateVecCount; ++i)
					JFSMstatePrivate::ParameterInterface::RemoveParameter(stateVec[i], guid); 
			}
		public:
			bool RegisterInstance()noexcept
			{
				return ownerInterface->RegisterDiagram(thisPointer);
			}
			bool DeRegisterInstance()noexcept
			{
				return ownerInterface->DeRegisterDiagram(thisPointer);
			}
		public:
			bool AddState(const JUserPtr<JFSMstate>& newState)noexcept
			{
				if (newState == nullptr)
					return false;

				const uint stateVecSize = (uint)stateVec.size();
				if (stateVecSize >= GetMaxStateCapacity())
					return false;

				if (stateVec.size() == 0)
				{
					initState = newState;
					nowStateGuid = newState->GetGuid();
				}
				newState->SetName(JCUtil::MakeUniqueName(stateVec, newState->GetName()));
				stateMap.emplace(newState->GetGuid(), newState);
				stateVec.push_back(newState);

				return true;
			}
			bool RemoveState(const JUserPtr<JFSMstate>& state)noexcept
			{
				if (state == nullptr)
					return false;

				const size_t guid = state->GetGuid();
				if (stateMap.find(guid) != stateMap.end())
				{
					stateMap.erase(guid);
					int index = JCUtil::GetTypeIndex(stateVec, guid);
					if (index != -1)
					{ 
						stateVec.erase(stateVec.begin() + index);
						const uint stateCount = (uint)stateVec.size();
						for (uint i = 0; i < stateCount; ++i)
							JFSMinterface::BeginDestroy(stateVec[i]->GetTransitionByOutGuid(state->GetGuid()).Get());

						if (guid == nowStateGuid)
						{
							nowStateGuid = 0;
							if (stateVec.size() > 0)
								nowStateGuid = stateVec[0]->GetGuid();
						}
						return true;
					}
					else
						return false;
				}
				else
					return false;
			}
		public:
			void RegisterThisPointer(JFSMdiagram* fsmDiagram)
			{
				thisPointer = GetWeakPtr(fsmDiagram);
			}
			void RegisterPostCreation()
			{
				ownerInterface->GetParameterStorageUser()->AddUser(this, thisPointer->GetGuid());
			}
			void DeRegisterPreDestruction()
			{
				ownerInterface->GetParameterStorageUser()->RemoveUser(this, thisPointer->GetGuid());
			}
			static void RegisterTypeData()
			{
				JIdentifier::RegisterPrivateInterface(JFSMdiagram::StaticTypeInfo(), dPrivate);
				IMPL_REALLOC_BIND(JFSMdiagram::JFSMdiagramImpl, thisPointer)
			}
		};

		JFSMdiagram::InitData::InitData(JFSMdiagramOwnerInterface* ownerInterface)
			:JFSMinterface::InitData(JFSMdiagram::StaticTypeInfo()), ownerInterface(ownerInterface)
		{}
		JFSMdiagram::InitData::InitData(const std::wstring& name, const size_t guid, JFSMdiagramOwnerInterface* ownerInterface)
			: JFSMinterface::InitData(JFSMdiagram::StaticTypeInfo(), name, guid), ownerInterface(ownerInterface)
		{}
		JFSMdiagram::InitData::InitData(const JTypeInfo& initTypeInfo, JFSMdiagramOwnerInterface* ownerInterface)
			:JFSMinterface::InitData(initTypeInfo), ownerInterface(ownerInterface)
		{}
		JFSMdiagram::InitData::InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid, JFSMdiagramOwnerInterface* ownerInterface)
			: JFSMinterface::InitData(initTypeInfo, name, guid), ownerInterface(ownerInterface)
		{}
		bool JFSMdiagram::InitData::IsValidData()const noexcept
		{
			return JFSMinterface::InitData::IsValidData() && ownerInterface != nullptr;
		}
		 
		JIdentifierPrivate& JFSMdiagram::PrivateInterface()const noexcept
		{
			return dPrivate;
		}
		J_FSM_OBJECT_TYPE JFSMdiagram::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::DIAGRAM;
		} 
		uint JFSMdiagram::GetStateCount()const noexcept
		{
			return (uint)impl->stateVec.size();
		}
		JUserPtr<JFSMstate> JFSMdiagram::GetNowState()const noexcept
		{
			return impl->GetNowState();
		}
		JUserPtr<JFSMstate> JFSMdiagram::GetState(const size_t guid)const noexcept
		{
			return impl->GetState(guid);
		}
		JUserPtr<JFSMstate> JFSMdiagram::GetStateByIndex(const uint index)const noexcept
		{
			return impl->GetStateByIndex(index);
		}
		std::vector<JUserPtr<JFSMstate>> JFSMdiagram::GetStateVec()noexcept
		{
			return impl->stateVec;
		}
		bool JFSMdiagram::CanUseParameter(const size_t paramGuid)const noexcept
		{
			return impl->CanUseParameter(paramGuid);
		}
		bool JFSMdiagram::CanCreateState()const noexcept
		{
			return GetStateCount() < GetMaxStateCapacity();
		}  
		void JFSMdiagram::Initialize()noexcept
		{
			impl->Initialize();
		}
		void JFSMdiagram::Clear()noexcept
		{
			impl->Clear();
		}
		JFSMdiagram::JFSMdiagram(const InitData& initData)
			:JFSMinterface(initData),impl(std::make_unique<JFSMdiagramImpl>(initData, this))
		{}
		JFSMdiagram::~JFSMdiagram()
		{
			impl.reset();
		}

		using CreateInstanceInterface = JFSMdiagramPrivate::CreateInstanceInterface;
		using DestroyInstanceInterface = JFSMdiagramPrivate::DestroyInstanceInterface;
		using OwnTypeInterface = JFSMdiagramPrivate::OwnTypeInterface;
		using OwnerTypeInterface = JFSMdiagramPrivate::OwnerTypeInterface;
		using ParamInterface = JFSMdiagramPrivate::ParamInterface;

		JOwnerPtr<JIdentifier> CreateInstanceInterface::Create(JDITypeDataBase* initData)
		{
			return JPtrUtil::MakeOwnerPtr<JFSMdiagram>(*static_cast<JFSMdiagram::InitData*>(initData));
		}
		void CreateInstanceInterface::Initialize(JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept
		{
			JIdentifierPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
			JFSMdiagram* diagram = static_cast<JFSMdiagram*>(createdPtr);
			diagram->impl->RegisterThisPointer(diagram);
			diagram->impl->RegisterPostCreation();
			diagram->impl->Initialize();
		}
		void CreateInstanceInterface::RegisterCash(JIdentifier* createdPtr)noexcept
		{
			static_cast<JFSMdiagram*>(createdPtr)->impl->RegisterInstance();
		}
		bool CreateInstanceInterface::CanCreateInstance(JDITypeDataBase* initData)const noexcept
		{
			const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JFSMdiagram::InitData::StaticTypeInfo());
			return isValidPtr && initData->IsValidData();
		}
 
		void DestroyInstanceInterface::Clear(JIdentifier* ptr, const bool isForced)
		{
			JFSMinterfacePrivate::DestroyInstanceInterface::Clear(ptr, isForced);
			JFSMdiagram* dPtr = static_cast<JFSMdiagram*>(ptr); 
			dPtr->impl->DeRegisterPreDestruction();
			dPtr->impl->Clear();
		}
		void DestroyInstanceInterface::DeRegisterCash(JIdentifier* ptr)noexcept
		{
			static_cast<JFSMdiagram*>(ptr)->impl->DeRegisterInstance();
		}

		bool OwnTypeInterface::AddState(const JUserPtr<JFSMstate>& state)noexcept
		{
			return state->GetOwner()->impl->AddState(state);
		}
		bool OwnTypeInterface::RemoveState(const JUserPtr<JFSMstate>& state)noexcept
		{
			return state->GetOwner()->impl->RemoveState(state);
		}

		void OwnerTypeInterface::SetOwnerPointer(const JUserPtr<JFSMdiagram>& digram, JFSMdiagramOwnerInterface* ownInterface)noexcept
		{
			digram->impl->ownerInterface = ownInterface;
		}

		std::vector<JUserPtr<JFSMparameter>> ParamInterface::GetStorageParameter(const JUserPtr<JFSMdiagram>& diagram)noexcept
		{
			return diagram->impl->GetStorageParameter();
		}

		JIdentifierPrivate::CreateInstanceInterface& JFSMdiagramPrivate::GetCreateInstanceInterface()const noexcept
		{
			static CreateInstanceInterface pI;
			return pI;
		}
		JIdentifierPrivate::DestroyInstanceInterface& JFSMdiagramPrivate::GetDestroyInstanceInterface()const noexcept
		{
			static DestroyInstanceInterface pI;
			return pI;
		}
	}
}
