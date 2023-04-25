#include"JFSMdiagram.h"
#include"JFSMdiagramPrivate.h"
#include"JFSMstate.h"  
#include"JFSMstatePrivate.h"  
#include"JFSMtransition.h"  
#include"JFSMownerInterface.h"
#include"JFSMparameterValueType.h"
#include"JFSMparameterStorageAccess.h"   
#include"../Guid/GuidCreator.h"
#include"../Identity/JIdentifierImplBase.h"
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
			_StateUpdateInterface* StateUpdateInterface(JFSMstate* state)
			{
				return &static_cast<_StateUpdateInterface&>(static_cast<JFSMstatePrivate&>(state->GetPrivateInterface()).GetUpdateInterface());
			}
		}
 
		class JFSMdiagram::JFSMdiagramImpl : public JIdentifierImplBase,
			public JFSMparameterStorageUserInterface
		{
			REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JFSMdiagramImpl)
		public:
			JFSMdiagram* thisDiagram = nullptr;
		public:
			//vector + unorered map => 64bit overhead
			JFSMdiagramOwnerInterface* ownerInterface;
			std::vector<JFSMstate*> stateVec;
			std::unordered_map<size_t, JFSMstate*> stateMap;
			JFSMstate* initState = nullptr;
			size_t nowStateGuid;
		public:
			JFSMdiagramImpl(const InitData& initData, JFSMdiagram* thisDiagram)
				:ownerInterface(initData.ownerInterface), thisDiagram(thisDiagram)
			{ 
				ownerInterface->GetParameterStorageUser()->AddUser(this, thisDiagram->GetGuid());
			}
			~JFSMdiagramImpl()
			{
				ownerInterface->GetParameterStorageUser()->RemoveUser(this, thisDiagram->GetGuid());
			}
		public:
			JFSMstate* GetNowState()const noexcept
			{
				auto data = stateMap.find(nowStateGuid);
				if (data != stateMap.end())
					return data->second;
				else
					return nullptr;
			}
			JFSMstate* GetState(const size_t guid)const noexcept
			{
				auto data = stateMap.find(guid);
				if (data != stateMap.end())
					return data->second;
				else
					return nullptr;
			}
			JFSMstate* GetStateByIndex(const uint index)const noexcept
			{
				if (index < stateVec.size())
					return stateVec[index];
				else
					return nullptr;
			}
			std::vector<JFSMparameter*> GetStorageParameter()const noexcept
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
				std::vector<JFSMstate*> copy = stateVec;
				for(auto& data : copy)
					JFSMinterface::BeginDestroy(data); 
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
				return ownerInterface->RegisterDiagram(thisDiagram);
			}
			bool DeRegisterInstance()noexcept
			{
				return ownerInterface->DeRegisterDiagram(thisDiagram);
			}
		public:
			bool AddState(JFSMstate* newState)noexcept
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
			bool RemoveState(JFSMstate* state)noexcept
			{
				if (state == nullptr)
					return false;

				const size_t guid = state->GetGuid();
				if (stateMap.find(guid) != stateMap.end())
				{
					stateMap.erase(guid);
					int index = JCUtil::GetJIdenIndex(stateVec, guid);
					if (index != -1)
					{ 
						stateVec.erase(stateVec.begin() + index);
						const uint stateCount = (uint)stateVec.size();
						for (uint i = 0; i < stateCount; ++i)
							JFSMinterface::BeginDestroy(stateVec[i]->GetTransitionByOutGuid(state->GetGuid()));

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
			static void RegisterCallOnce()
			{
				Core::JIdentifier::RegisterPrivateInterface(JFSMdiagram::StaticTypeInfo(), dPrivate);
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
		 
		Core::JIdentifierPrivate& JFSMdiagram::GetPrivateInterface()const noexcept
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
		JFSMstate* JFSMdiagram::GetNowState()const noexcept
		{
			return impl->GetNowState();
		}
		JFSMstate* JFSMdiagram::GetState(const size_t guid)const noexcept
		{
			return impl->GetState(guid);
		}
		JFSMstate* JFSMdiagram::GetStateByIndex(const uint index)const noexcept
		{
			return impl->GetStateByIndex(index);
		}
		std::vector<JFSMstate*> JFSMdiagram::GetStateVec()noexcept
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
		using ParamInterface = JFSMdiagramPrivate::ParamInterface;

		Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
		{
			return Core::JPtrUtil::MakeOwnerPtr<JFSMdiagram>(*static_cast<JFSMdiagram::InitData*>(initData.get()));
		}
		bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
		{
			const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JFSMdiagram::InitData::StaticTypeInfo());
			return isValidPtr && initData->IsValidData();
		}
		void CreateInstanceInterface::RegisterCash(Core::JIdentifier* createdPtr)noexcept
		{ 
			static_cast<JFSMdiagram*>(createdPtr)->impl->RegisterInstance();
		}

		void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
		{
			JFSMdiagram* dPtr = static_cast<JFSMdiagram*>(ptr); 
			dPtr->impl->Clear();
		}
		void DestroyInstanceInterface::DeRegisterCash(JIdentifier* ptr)noexcept
		{
			static_cast<JFSMdiagram*>(ptr)->impl->DeRegisterInstance();
		}

		bool OwnTypeInterface::AddState(JFSMstate* state)noexcept
		{
			return state->GetOwner()->impl->AddState(state);
		}
		bool OwnTypeInterface::RemoveState(JFSMstate* state)noexcept
		{
			return state->GetOwner()->impl->RemoveState(state);
		}

		std::vector<JFSMparameter*> ParamInterface::GetStorageParameter(JFSMdiagram* diagram)noexcept
		{
			return diagram->impl->GetStorageParameter();
		}

		Core::JIdentifierPrivate::CreateInstanceInterface& JFSMdiagramPrivate::GetCreateInstanceInterface()const noexcept
		{
			static CreateInstanceInterface pI;
			return pI;
		}
		Core::JIdentifierPrivate::DestroyInstanceInterface& JFSMdiagramPrivate::GetDestroyInstanceInterface()const noexcept
		{
			static DestroyInstanceInterface pI;
			return pI;
		}
	}
}
