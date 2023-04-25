#include"JFSMtransition.h"
#include"JFSMtransitionPrivate.h"
#include"JFSMstatePrivate.h"
#include"JFSMstate.h"
#include"JFSMcondition.h"    
#include"../Identity/JIdenCreator.h"
#include"../Identity/JIdentifierImplBase.h"
#include"../../Utility/JCommonUtility.h" 

namespace JinEngine
{
	namespace Core
	{
		namespace
		{ 
			static constexpr float errorOnValue = -100000;
			static JFSMtransitionPrivate tPrivate;
		}

		class JFSMtransition::JFSMtransitionImpl : public JIdentifierImplBase
		{
			REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JFSMtransitionImpl) 
		public:
			JFSMstate* owner;
			JFSMstate* outState;
			std::vector<JFSMcondition*>conditionVec;
		public:
			JFSMtransitionImpl(const InitData& initData, JFSMtransition* thisTrans)
				:owner(initData.ownerState.Get()),
				outState(initData.outState.Get())
			{}
			~JFSMtransitionImpl() {}
		public:
			float GetConditionOnValue(const uint index)const noexcept
			{
				if (index >= conditionVec.size())
					return errorOnValue;

				return conditionVec[index]->GetOnValue();
			}
			JFSMcondition* GetConditionByIndex(const uint index)const noexcept
			{
				if (index >= conditionVec.size())
					return nullptr;

				return conditionVec[index];
			}
			REGISTER_METHOD(GetConditionVec)
			std::vector<JFSMcondition*> GetConditionVec()const noexcept
			{
				return conditionVec;
			}
		public:
			REGISTER_METHOD(SetConditionVec)
			void SetConditionVec(std::vector<JFSMcondition*> vec)noexcept
			{
				conditionVec = vec;
			}
			REGISTER_METHOD_GUI_WIDGET(Condition, GetConditionVec, SetConditionVec, GUI_LIST(J_GUI_LIST_TYPE::DYNAMIC, true, CreateCondition))
		public:
			bool HasSatisfiedCondition()const noexcept
			{
				const uint condVecSize = (uint)conditionVec.size();

				if (condVecSize == 0)
					return true;

				for (uint index = 0; index < condVecSize; ++index)
				{
					if (conditionVec[index]->IsSatisfied())
						return true;
				}
				return false;
			}
		public:
			static JIdentifier* CreateCondition(JIdentifier* iden)noexcept
			{
				if (!iden->GetTypeInfo().IsChildOf<JFSMtransition>())
					return nullptr;

				return JICI::Create<JFSMcondition>(GetUserPtr<JFSMtransition>(iden));
			}
		public:
			bool RemoveParameter(const size_t guid)noexcept
			{
				const uint conditionVecSize = (uint)conditionVec.size();
				for (uint index = 0; index < conditionVecSize; ++index)
				{
					if (conditionVec[index]->HasSameParameter(guid))
						conditionVec[index]->SetParameter(nullptr);
				}
				return true;
			}
		public:
			static bool RegisterInstance(JFSMtransition* trans)noexcept
			{
				return JFSMstatePrivate::OwnTypeInterface::AddTransition(trans);
			}
			static bool DeRegisterInstance(JFSMtransition* trans)noexcept
			{
				return JFSMstatePrivate::OwnTypeInterface::RemoveTransition(trans);
			}
		public:
			bool AddCondition(JFSMcondition* newCondition)noexcept
			{
				JFSMcondition* res = nullptr;
				if (newCondition != nullptr)
				{
					newCondition->SetName(JCUtil::MakeUniqueName(conditionVec, newCondition->GetName()));
					conditionVec.push_back(newCondition);
					return true;
				}
				else
					return false;
			}
			bool RemoveCondition(JFSMcondition* condition)noexcept
			{
				const size_t guid = condition->GetGuid();
				const uint conditionVecSize = (uint)conditionVec.size();
				for (uint i = 0; i < conditionVecSize; ++i)
				{
					if (conditionVec[i]->GetGuid() == guid)
					{
						conditionVec.erase(conditionVec.begin() + i);
						break;
					}
				}
				return true;
			}
		public:
			void Initialize()noexcept
			{
				const uint conditionVecSize = (uint)conditionVec.size();
				for (uint i = 0; i < conditionVecSize; ++i)
					conditionVec[i]->SetOnValue(0);
			}
			void Clear()
			{
				std::vector<JFSMcondition*> copy = conditionVec;
				for (auto& data : copy)
					JFSMinterface::BeginDestroy(data);
				conditionVec.clear();
			}
		public:
			static void RegisterCallOnce()
			{
				Core::JIdentifier::RegisterPrivateInterface(JFSMtransition::StaticTypeInfo(), tPrivate);
			}
		};

		JFSMtransition::InitData::InitData(JUserPtr<JFSMstate> ownerState,JUserPtr<JFSMstate> outState)
			:JFSMinterface::InitData(JFSMtransition::StaticTypeInfo()), ownerState(ownerState), outState(outState)
		{}
		JFSMtransition::InitData::InitData(const std::wstring& name,
			const size_t guid,
			JUserPtr<JFSMstate> ownerState,
			JUserPtr<JFSMstate> outState)
			: JFSMinterface::InitData(JFSMtransition::StaticTypeInfo(), name, guid), ownerState(ownerState), outState(outState)
		{}
		JFSMtransition::InitData::InitData(const JTypeInfo& initTypeInfo, 
			JUserPtr<JFSMstate> ownerState,
			JUserPtr<JFSMstate> outState)
			:JFSMinterface::InitData(initTypeInfo), ownerState(ownerState), outState(outState)
		{}
		JFSMtransition::InitData::InitData(const JTypeInfo& initTypeInfo,
			const std::wstring& name, 
			const size_t guid,
			JUserPtr<JFSMstate> ownerState,
			JUserPtr<JFSMstate> outState)
			: JFSMinterface::InitData(initTypeInfo, name, guid), ownerState(ownerState), outState(outState)
		{}
		bool JFSMtransition::InitData::IsValidData()const noexcept
		{
			return JFSMinterface::InitData::IsValidData() && ownerState != nullptr && outState != nullptr;
		}
 
		Core::JIdentifierPrivate& JFSMtransition::GetPrivateInterface()const noexcept
		{
			return tPrivate;
		}
		J_FSM_OBJECT_TYPE JFSMtransition::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::TRANSITION;
		}
		JFSMstate* JFSMtransition::GetOwner()const noexcept
		{
			return impl->owner;
		}
		JFSMstate* JFSMtransition::GetOutState()const noexcept
		{
			return impl->outState;
		}
		uint JFSMtransition::GetConditioCount()const noexcept
		{
			return (uint)impl->conditionVec.size();
		}
		float JFSMtransition::GetConditionOnValue(const uint index)const noexcept
		{
			return impl->GetConditionOnValue(index);
		}
		size_t JFSMtransition::GetOutputStateGuid()const noexcept
		{
			return impl->outState->GetGuid();
		}
		JFSMcondition* JFSMtransition::GetConditionByIndex(const uint index)const noexcept
		{
			return impl->GetConditionByIndex(index);
		}
		std::vector<JFSMcondition*> JFSMtransition::GetConditionVec()const noexcept
		{
			return impl->GetConditionVec();
		}
		void JFSMtransition::SetConditionVec(std::vector<JFSMcondition*> vec)noexcept
		{
			impl->SetConditionVec(vec);
		}
		bool JFSMtransition::HasSatisfiedCondition()const noexcept
		{
			return impl->HasSatisfiedCondition();
		}		 
		JIdentifier* JFSMtransition::CreateCondition(JIdentifier* iden)noexcept
		{
			return JFSMtransition::JFSMtransitionImpl::CreateCondition(iden);
		}
		void JFSMtransition::Initialize()noexcept
		{
			impl->Initialize();
		}
		JFSMtransition::JFSMtransition(const InitData& initData)
			:JFSMinterface(initData), impl(std::make_unique<JFSMtransitionImpl>(initData, this))
		{}
		JFSMtransition::~JFSMtransition()
		{
			impl.reset();
		}

		using CreateInstanceInterface = JFSMtransitionPrivate::CreateInstanceInterface;
		using DestroyInstanceInterface = JFSMtransitionPrivate::DestroyInstanceInterface;
		using OwnTypeInterface = JFSMtransitionPrivate::OwnTypeInterface;
		using UpdateInterface = JFSMtransitionPrivate::UpdateInterface;
		using ParameterInterface = JFSMtransitionPrivate::ParameterInterface;

		Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
		{
			return Core::JPtrUtil::MakeOwnerPtr<JFSMtransition>(*static_cast<JFSMtransition::InitData*>(initData.get()));
		}
		bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
		{
			const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JFSMtransition::InitData::StaticTypeInfo());
			return isValidPtr && initData->IsValidData();
		}
		void CreateInstanceInterface::RegisterCash(Core::JIdentifier* createdPtr)noexcept
		{
			JFSMtransition::JFSMtransitionImpl::RegisterInstance(static_cast<JFSMtransition*>(createdPtr));
		}

		void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
		{
			static_cast<JFSMtransition*>(ptr)->impl->Clear();
		}
		void DestroyInstanceInterface::DeRegisterCash(JIdentifier* ptr)noexcept
		{
			JFSMtransition::JFSMtransitionImpl::DeRegisterInstance(static_cast<JFSMtransition*>(ptr));
		}

		bool OwnTypeInterface::AddCondition(JFSMcondition* cond)noexcept
		{
			return cond->GetOwner()->impl->AddCondition(cond);
		}
		bool OwnTypeInterface::RemoveCondition(JFSMcondition* cond)noexcept
		{
			return cond->GetOwner()->impl->RemoveCondition(cond);
		}

		void UpdateInterface::Initialize(JFSMtransition* trans)noexcept
		{
			trans->impl->Initialize();
		}

		void ParameterInterface::RemoveParameter(JFSMtransition* trans, const size_t guid)
		{
			trans->impl->RemoveParameter(guid);
		}

		Core::JIdentifierPrivate::CreateInstanceInterface& JFSMtransitionPrivate::GetCreateInstanceInterface()const noexcept
		{
			static CreateInstanceInterface pI;
			return pI;
		}
		Core::JIdentifierPrivate::DestroyInstanceInterface& JFSMtransitionPrivate::GetDestroyInstanceInterface()const noexcept
		{
			static DestroyInstanceInterface pI;
			return pI;
		}
		JFSMtransitionPrivate::UpdateInterface& JFSMtransitionPrivate::GetUpdateInterface() const noexcept
		{
			static UpdateInterface pI;
			return pI;
		}
	}
}