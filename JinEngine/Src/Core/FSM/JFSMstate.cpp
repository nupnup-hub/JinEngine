#include"JFSMstate.h"
#include"JFSMstatePrivate.h" 
#include"JFSMtransition.h" 
#include"JFSMtransitionPrivate.h" 
#include"JFSMdiagram.h"
#include"JFSMdiagramPrivate.h"
#include"../Guid/GuIdCreator.h"
#include"../Identity/JIdentifierImplBase.h"
#include"../../Utility/JCommonUtility.h"

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
			_TransitionUpdateInterface* TransitionUpdateInterface(JFSMtransition* trans)
			{
				return &static_cast<_TransitionUpdateInterface&>(static_cast<JFSMtransitionPrivate&>(trans->GetPrivateInterface()).GetUpdateInterface());
			}
		}
 
		class JFSMstate::JFSMstateImpl : public JIdentifierImplBase
		{
			REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JFSMstateImpl) 
		public:
			JFSMdiagram* owner;
			std::vector<JFSMtransition*> transitionVec;
			bool decidedNextState = false;
		public:
			JFSMstateImpl(const InitData& initData, JFSMstate* thisState)
				:owner(initData.ownerDiagram.Get())
			{}
			~JFSMstateImpl(){}
		public:
			JFSMtransition* GetTransition(const size_t guid)noexcept
			{
				int index = JCUtil::GetJIdenIndex(transitionVec, guid);
				if (index != -1)
					return transitionVec[index];
				else
					return nullptr;
			}
			JFSMtransition* GetTransitionByOutGuid(const size_t outputGuid)noexcept
			{
				const uint transitionSize = (uint)transitionVec.size();
				for (uint i = 0; i < transitionSize; ++i)
				{
					if (transitionVec[i]->GetOutputStateGuid() == outputGuid)
						return transitionVec[i];
				}
				return nullptr;
			}
			JFSMtransition* GetTransitionByIndex(uint index)noexcept
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
				std::vector<JFSMtransition*> copy = transitionVec; 
				for (auto& data : copy)
					JFSMinterface::BeginDestroy(data); 
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
			static bool RegisterInstance(JFSMstate* state)noexcept
			{
				return JFSMdiagramPrivate::OwnTypeInterface::AddState(state);
			}
			static bool DeRegisterInstance(JFSMstate* state)noexcept
			{
				return JFSMdiagramPrivate::OwnTypeInterface::RemoveState(state);
			}
		public:
			bool AddTransition(JFSMtransition* newTransition)noexcept
			{
				JFSMtransition* res = nullptr;
				if (newTransition != nullptr && owner->GetState(newTransition->GetOutputStateGuid()))
				{
					newTransition->SetName(JCUtil::MakeUniqueName(transitionVec, newTransition->GetName()));
					transitionVec.push_back(newTransition);
					return true;
				}
				else
					return false;
			}
			bool RemoveTransition(JFSMtransition* transition)noexcept
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
			static void RegisterCallOnce()
			{
				Core::JIdentifier::RegisterPrivateInterface(JFSMstate::StaticTypeInfo(), sPrivate);
			}
		};

		JFSMstate::InitData::InitData(JUserPtr<JFSMdiagram> ownerDiagram)
			:JFSMinterface::InitData(JFSMstate::StaticTypeInfo()), ownerDiagram(ownerDiagram)
		{}
		JFSMstate::InitData::InitData(const std::wstring& name, const size_t guid, JUserPtr<JFSMdiagram> ownerDiagram)
			: JFSMinterface::InitData(JFSMstate::StaticTypeInfo(), name, guid), ownerDiagram(ownerDiagram)
		{}
		JFSMstate::InitData::InitData(const JTypeInfo& initTypeInfo, JUserPtr<JFSMdiagram> ownerDiagram)
			:JFSMinterface::InitData(initTypeInfo), ownerDiagram(ownerDiagram)
		{}
		JFSMstate::InitData::InitData(const JTypeInfo& initTypeInfo, const std::wstring& name, const size_t guid, JUserPtr<JFSMdiagram> ownerDiagram)
			: JFSMinterface::InitData(initTypeInfo, name, guid), ownerDiagram(ownerDiagram)
		{}
		bool JFSMstate::InitData::IsValidData()const noexcept
		{
			return JFSMinterface::InitData::IsValidData() && ownerDiagram != nullptr;
		}
 
		Core::JIdentifierPrivate& JFSMstate::GetPrivateInterface()const noexcept
		{
			return sPrivate;
		}
		J_FSM_OBJECT_TYPE JFSMstate::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::STATE;
		}
		JFSMdiagram* JFSMstate::GetOwner()const noexcept
		{
			return impl->owner;
		}
		uint JFSMstate::GetTransitionCount()const noexcept
		{
			return (uint)impl->transitionVec.size();
		}
		JFSMtransition* JFSMstate::GetTransition(const size_t guid)noexcept
		{
			return impl->GetTransition(guid);
		}
		JFSMtransition* JFSMstate::GetTransitionByOutGuid(const size_t outputGuid)noexcept
		{
			return impl->GetTransitionByOutGuid(outputGuid);
		}
		JFSMtransition* JFSMstate::GetTransitionByIndex(uint index)noexcept
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

		Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
		{
			return Core::JPtrUtil::MakeOwnerPtr<JFSMstate>(*static_cast<JFSMstate::InitData*>(initData.get()));
		}
		bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
		{
			const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JFSMstate::InitData::StaticTypeInfo());
			return isValidPtr && initData->IsValidData();
		}
		void CreateInstanceInterface::RegisterCash(Core::JIdentifier* createdPtr)noexcept
		{
			JFSMstate::JFSMstateImpl::RegisterInstance(static_cast<JFSMstate*>(createdPtr));
		}

		void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
		{ 
			static_cast<JFSMstate*>(ptr)->impl->Clear();
		}
		void DestroyInstanceInterface::DeRegisterCash(JIdentifier* ptr)noexcept
		{
			JFSMstate::JFSMstateImpl::DeRegisterInstance(static_cast<JFSMstate*>(ptr));
		}

		bool OwnTypeInterface::AddTransition(JFSMtransition* transition)noexcept
		{
			return transition->GetOwner()->impl->AddTransition(transition);
		}
		bool OwnTypeInterface::RemoveTransition(JFSMtransition* transition)noexcept
		{
			return transition->GetOwner()->impl->RemoveTransition(transition);
		}

		void UpdateInterface::Initialize(JFSMstate* state)noexcept
		{
			state->impl->Initialize();
		}

		void ParameterInterface::RemoveParameter(JFSMstate* state, const size_t guid)
		{
			state->impl->RemoveParameter(guid);
		}

		Core::JIdentifierPrivate::CreateInstanceInterface& JFSMstatePrivate::GetCreateInstanceInterface()const noexcept
		{
			static CreateInstanceInterface pI;
			return pI;
		}
		Core::JIdentifierPrivate::DestroyInstanceInterface& JFSMstatePrivate::GetDestroyInstanceInterface()const noexcept
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