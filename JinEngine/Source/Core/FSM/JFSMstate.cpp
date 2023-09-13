#include"JFSMstate.h"
#include"JFSMstatePrivate.h" 
#include"JFSMtransition.h" 
#include"JFSMtransitionPrivate.h" 
#include"JFSMdiagram.h"
#include"JFSMdiagramPrivate.h"
#include"../Guid/JGuIdCreator.h"
#include"../Reflection/JTypeImplBase.h"
#include"../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{
		namespace
		{  
			using _TransitionUpdateInterface = JFSMtransitionPrivate::UpdateInterface;
		}
		namespace
		{
			static JFSMstatePrivate sPrivate;
			_TransitionUpdateInterface* TransitionUpdateInterface(const JUserPtr<JFSMtransition>& trans)
			{
				return &static_cast<_TransitionUpdateInterface&>(static_cast<JFSMtransitionPrivate&>(trans->PrivateInterface()).GetUpdateInterface());
			}
		}
 
		class JFSMstate::JFSMstateImpl : public JTypeImplBase
		{
			REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JFSMstateImpl) 
		public:
			JWeakPtr<JFSMstate> thisPointer;
		public:
			JUserPtr<JFSMdiagram> owner;
			std::vector<JUserPtr<JFSMtransition>> transitionVec;
			bool decidedNextState = false;
		public:
			JFSMstateImpl(const InitData& initData, JFSMstate* thisStateRaw)
				:owner(initData.ownerDiagram)
			{}
			~JFSMstateImpl(){}
		public:
			JUserPtr<JFSMtransition> GetTransition(const size_t guid)noexcept
			{
				int index = JCUtil::GetTypeIndex(transitionVec, guid);
				if (index != -1)
					return transitionVec[index];
				else
					return nullptr;
			}
			JUserPtr<JFSMtransition> GetTransitionByOutGuid(const size_t outputGuid)noexcept
			{
				const uint transitionSize = (uint)transitionVec.size();
				for (uint i = 0; i < transitionSize; ++i)
				{
					if (transitionVec[i]->GetOutputStateGuid() == outputGuid)
						return transitionVec[i];
				}
				return nullptr;
			}
			JUserPtr<JFSMtransition> GetTransitionByIndex(uint index)noexcept
			{
				if (transitionVec.size() <= index)
					return nullptr;
				else
					return transitionVec[index];
			}
		public:
			void Initialize()noexcept
			{
				decidedNextState = false;
				const uint transitionSize = (uint)transitionVec.size();

				for (uint i = 0; i < transitionSize; ++i)
					TransitionUpdateInterface(transitionVec[i])->Initialize(transitionVec[i]); 
			}
			void Clear()noexcept
			{
				std::vector<JUserPtr<JFSMtransition>> copy = transitionVec;
				for (auto& data : copy)
					JFSMinterface::BeginDestroy(data.Get()); 
				transitionVec.clear();
			}
		public:
			void EnterState()noexcept
			{
				decidedNextState = false;
			}
		public:
			bool RemoveParameter(const size_t guid)noexcept
			{
				const uint transitionSize = (uint)transitionVec.size();
				for (uint i = 0; i < transitionSize; ++i)
					JFSMtransitionPrivate::ParameterInterface::RemoveParameter(transitionVec[i], guid); 
				return true;
			}
		public:
			bool RegisterInstance()noexcept
			{
				return JFSMdiagramPrivate::OwnTypeInterface::AddState(thisPointer);
			}
			bool DeRegisterInstance()noexcept
			{
				return JFSMdiagramPrivate::OwnTypeInterface::RemoveState(thisPointer);
			}
		public:
			bool AddTransition(const JUserPtr<JFSMtransition>& newTransition)noexcept
			{ 
				if (newTransition != nullptr && owner->GetState(newTransition->GetOutputStateGuid()).IsValid())
				{
					newTransition->SetName(JCUtil::MakeUniqueName(transitionVec, newTransition->GetName()));
					transitionVec.push_back(newTransition);
					return true;
				}
				else
					return false;
			}
			bool RemoveTransition(const JUserPtr<JFSMtransition>& transition)noexcept
			{
				const uint transitionSize = (uint)transitionVec.size();
				for (uint index = 0; index < transitionSize; ++index)
				{
					if (transitionVec[index]->GetGuid() == transition->GetGuid())
					{
						transitionVec.erase(transitionVec.begin() + index);
						return true;
					}
				}
				return false;
			}
		public:
			void RegisterThisPointer(JFSMstate* fsmState)
			{
				thisPointer = GetWeakPtr(fsmState);
			}
			static void RegisterTypeData()
			{
				JIdentifier::RegisterPrivateInterface(JFSMstate::StaticTypeInfo(), sPrivate);
				IMPL_REALLOC_BIND(JFSMstate::JFSMstateImpl, thisPointer)
			}
		};

		JFSMstate::InitData::InitData(const JUserPtr<JFSMdiagram>& ownerDiagram)
			:JFSMinterface::InitData(JFSMstate::StaticTypeInfo()), ownerDiagram(ownerDiagram)
		{}
		JFSMstate::InitData::InitData(const std::wstring& name, const size_t guid, const JUserPtr<JFSMdiagram>& ownerDiagram)
			: JFSMinterface::InitData(JFSMstate::StaticTypeInfo(), name, guid), ownerDiagram(ownerDiagram)
		{}
		JFSMstate::InitData::InitData(const JTypeInfo& initTypeInfo, const JUserPtr<JFSMdiagram>& ownerDiagram)
			:JFSMinterface::InitData(initTypeInfo), ownerDiagram(ownerDiagram)
		{}
		JFSMstate::InitData::InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid, const JUserPtr<JFSMdiagram>& ownerDiagram)
			: JFSMinterface::InitData(initTypeInfo, name, guid), ownerDiagram(ownerDiagram)
		{}
		bool JFSMstate::InitData::IsValidData()const noexcept
		{
			return JFSMinterface::InitData::IsValidData() && ownerDiagram != nullptr;
		}
 
		JIdentifierPrivate& JFSMstate::PrivateInterface()const noexcept
		{
			return sPrivate;
		}
		J_FSM_OBJECT_TYPE JFSMstate::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::STATE;
		}
		JUserPtr<JFSMdiagram> JFSMstate::GetOwner()const noexcept
		{
			return impl->owner;
		}
		uint JFSMstate::GetTransitionCount()const noexcept
		{
			return (uint)impl->transitionVec.size();
		}
		JUserPtr<JFSMtransition> JFSMstate::GetTransition(const size_t guid)noexcept
		{
			return impl->GetTransition(guid);
		}
		JUserPtr<JFSMtransition> JFSMstate::GetTransitionByOutGuid(const size_t outputGuid)noexcept
		{
			return impl->GetTransitionByOutGuid(outputGuid);
		}
		JUserPtr<JFSMtransition> JFSMstate::GetTransitionByIndex(uint index)noexcept
		{
			return impl->GetTransitionByIndex(index);
		}
		void JFSMstate::Initialize()noexcept
		{
			impl->Initialize();
		}
		void JFSMstate::Clear()noexcept
		{
			impl->Clear();
		}
		void JFSMstate::EnterState()noexcept
		{
			impl->EnterState();
		}
 
		JFSMstate::JFSMstate(const InitData& initData)
			:JFSMinterface(initData), impl(std::make_unique<JFSMstateImpl>(initData, this))
		{}
		JFSMstate::~JFSMstate()
		{
			impl.reset();
		}

		using CreateInstanceInterface = JFSMstatePrivate::CreateInstanceInterface;
		using DestroyInstanceInterface = JFSMstatePrivate::DestroyInstanceInterface;
		using OwnTypeInterface = JFSMstatePrivate::OwnTypeInterface;
		using UpdateInterface = JFSMstatePrivate::UpdateInterface;
		using ParameterInterface = JFSMstatePrivate::ParameterInterface;

		JOwnerPtr<JIdentifier> CreateInstanceInterface::Create(JDITypeDataBase* initData)
		{
			return JPtrUtil::MakeOwnerPtr<JFSMstate>(*static_cast<JFSMstate::InitData*>(initData));
		}
		void CreateInstanceInterface::Initialize(JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept
		{
			JIdentifierPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
			JFSMstate* state = static_cast<JFSMstate*>(createdPtr);
			state->impl->RegisterThisPointer(state);
			state->impl->Initialize();
		}
		void CreateInstanceInterface::RegisterCash(JIdentifier* createdPtr)noexcept
		{
			static_cast<JFSMstate*>(createdPtr)->impl->RegisterInstance();
		}
		bool CreateInstanceInterface::CanCreateInstance(JDITypeDataBase* initData)const noexcept
		{
			const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JFSMstate::InitData::StaticTypeInfo());
			return isValidPtr && initData->IsValidData();
		}

		void DestroyInstanceInterface::Clear(JIdentifier* ptr, const bool isForced)
		{
			static_cast<JFSMstate*>(ptr)->impl->Clear();
			JFSMinterfacePrivate::DestroyInstanceInterface::Clear(ptr, isForced);
		}
		void DestroyInstanceInterface::DeRegisterCash(JIdentifier* ptr)noexcept
		{
			static_cast<JFSMstate*>(ptr)->impl->DeRegisterInstance();
		}

		bool OwnTypeInterface::AddTransition(const JUserPtr<JFSMtransition>& transition)noexcept
		{
			return transition->GetInState()->impl->AddTransition(transition);
		}
		bool OwnTypeInterface::RemoveTransition(const JUserPtr<JFSMtransition>& transition)noexcept
		{
			return transition->GetInState()->impl->RemoveTransition(transition);
		}

		void UpdateInterface::Initialize(const JUserPtr<JFSMstate>& state)noexcept
		{
			state->impl->Initialize();
		}

		void ParameterInterface::RemoveParameter(const JUserPtr<JFSMstate>& state, const size_t guid)
		{
			state->impl->RemoveParameter(guid);
		}

		JIdentifierPrivate::CreateInstanceInterface& JFSMstatePrivate::GetCreateInstanceInterface()const noexcept
		{
			static CreateInstanceInterface pI;
			return pI;
		}
		JIdentifierPrivate::DestroyInstanceInterface& JFSMstatePrivate::GetDestroyInstanceInterface()const noexcept
		{
			static DestroyInstanceInterface pI;
			return pI;
		}
		JFSMstatePrivate::UpdateInterface& JFSMstatePrivate::GetUpdateInterface() const noexcept
		{
			static UpdateInterface pI;
			return pI;
		}
	}
}