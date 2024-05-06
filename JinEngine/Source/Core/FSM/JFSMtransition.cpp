/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JFSMtransition.h"
#include"JFSMtransitionPrivate.h"
#include"JFSMstatePrivate.h"
#include"JFSMstate.h" 
#include"JFSMcondition.h"    
#include"../Identity/JIdenCreator.h"
#include"../Reflection/JTypeImplBase.h"
#include"../Utility/JCommonUtility.h" 

namespace JinEngine
{
	namespace Core
	{
		namespace
		{ 
			static constexpr float errorOnValue = -100000;
			static JFSMtransitionPrivate tPrivate;
		}

		class JFSMtransition::JFSMtransitionImpl : public JTypeImplBase
		{
			REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JFSMtransitionImpl) 
		public:
			JWeakPtr<JFSMtransition> thisPointer;
		public:
			JUserPtr<JFSMstate> inState;
			JUserPtr<JFSMstate> outState;
			std::vector<JUserPtr<JFSMcondition>>conditionVec;
		public:
			JFSMtransitionImpl(const InitData& initData, JFSMtransition* thisTransRaaw)
				:inState(initData.inState), outState(initData.outState)
			{}
			~JFSMtransitionImpl() {}
		public:
			float GetConditionOnValue(const uint index)const noexcept
			{
				if (index >= conditionVec.size())
					return errorOnValue;

				return conditionVec[index]->GetOnValue();
			}
			JUserPtr<JFSMcondition> GetConditionByIndex(const uint index)const noexcept
			{
				if (index >= conditionVec.size())
					return nullptr;

				return conditionVec[index];
			} 
			std::vector<JUserPtr<JFSMcondition>> GetConditionVec()const noexcept
			{ 
				return conditionVec;
			}
		public: 
			void SetConditionVec(std::vector<JUserPtr<JFSMcondition>> vec)noexcept
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
			static JUserPtr<JIdentifier> CreateCondition(JUserPtr<JIdentifier> trans)noexcept
			{
				if (!trans->GetTypeInfo().IsChildOf<JFSMtransition>())
					return nullptr;

				return JICI::Create<JFSMcondition>(Core::ConvertChildUserPtr< JFSMtransition>(std::move(trans)));
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
			bool RegisterInstance()noexcept
			{
				return JFSMstatePrivate::OwnTypeInterface::AddTransition(thisPointer);
			}
			bool DeRegisterInstance()noexcept
			{
				return JFSMstatePrivate::OwnTypeInterface::RemoveTransition(thisPointer);
			}
		public:
			bool AddCondition(const JUserPtr<JFSMcondition>& newCondition)noexcept
			{ 
				if (newCondition != nullptr)
				{
					newCondition->SetName(JCUtil::MakeUniqueName(conditionVec, newCondition->GetName()));
					conditionVec.push_back(newCondition);
					return true;
				}
				else
					return false;
			}
			bool RemoveCondition(const JUserPtr<JFSMcondition>& condition)noexcept
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
				std::vector<JUserPtr<JFSMcondition>> copy = conditionVec;
				for (auto& data : copy)
					JFSMinterface::BeginDestroy(data.Get());
				conditionVec.clear();
			}
		public:
			void RegisterThisPointer(JFSMtransition* trans)
			{
				thisPointer = GetWeakPtr(trans);
			}
			static void RegisterTypeData()
			{
				JIdentifier::RegisterPrivateInterface(JFSMtransition::StaticTypeInfo(), tPrivate);
				IMPL_REALLOC_BIND(JFSMtransition::JFSMtransitionImpl, thisPointer)
			}
		};

		JFSMtransition::InitData::InitData(const JUserPtr<JFSMstate>& inState, const JUserPtr<JFSMstate>& outState)
			:JFSMinterface::InitData(JFSMtransition::StaticTypeInfo()), inState(inState), outState(outState)
		{}
		JFSMtransition::InitData::InitData(const std::wstring& name,
			const size_t guid,
			const JUserPtr<JFSMstate>& inState,
			const JUserPtr<JFSMstate>& outState)
			: JFSMinterface::InitData(JFSMtransition::StaticTypeInfo(), name, guid), inState(inState), outState(outState)
		{}
		JFSMtransition::InitData::InitData(const JTypeInfo& initTypeInfo, 
			const JUserPtr<JFSMstate>& inState,
			const JUserPtr<JFSMstate>& outState)
			:JFSMinterface::InitData(initTypeInfo), inState(inState), outState(outState)
		{}
		JFSMtransition::InitData::InitData(const JTypeInfo& initTypeInfo,
			const std::wstring& name, 
			const size_t guid,
			const JUserPtr<JFSMstate>& inState,
			const JUserPtr<JFSMstate>& outState)
			: JFSMinterface::InitData(initTypeInfo, name, guid), inState(inState), outState(outState)
		{}
		bool JFSMtransition::InitData::IsValidData()const noexcept
		{
			return JFSMinterface::InitData::IsValidData() && inState != nullptr && outState != nullptr;
		}
 
		JIdentifierPrivate& JFSMtransition::PrivateInterface()const noexcept
		{
			return tPrivate;
		}
		J_FSM_OBJECT_TYPE JFSMtransition::GetFSMobjType()const noexcept
		{
			return J_FSM_OBJECT_TYPE::TRANSITION;
		}
		JUserPtr<JFSMstate> JFSMtransition::GetInState()const noexcept
		{
			return impl->inState;
		}
		JUserPtr<JFSMstate> JFSMtransition::GetOutState()const noexcept
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
		JUserPtr<JFSMcondition> JFSMtransition::GetConditionByIndex(const uint index)const noexcept
		{
			return impl->GetConditionByIndex(index);
		}
		std::vector<JUserPtr<JFSMcondition>> JFSMtransition::GetConditionVec()const noexcept
		{
			return impl->GetConditionVec();
		}
		void JFSMtransition::SetConditionVec(std::vector<JUserPtr<JFSMcondition>> vec)noexcept
		{
			impl->SetConditionVec(vec);
		}
		bool JFSMtransition::HasSatisfiedCondition()const noexcept
		{
			return impl->HasSatisfiedCondition();
		}		 
		JUserPtr<JIdentifier> JFSMtransition::CreateCondition(const JUserPtr<JIdentifier>& iden)noexcept
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

		JOwnerPtr<JIdentifier> CreateInstanceInterface::Create(JDITypeDataBase* initData)
		{
			return JPtrUtil::MakeOwnerPtr<JFSMtransition>(*static_cast<JFSMtransition::InitData*>(initData));
		}
		void CreateInstanceInterface::Initialize(JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept
		{
			JFSMinterfacePrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
			JFSMtransition* trans = static_cast<JFSMtransition*>(createdPtr);
			trans->impl->RegisterThisPointer(trans);
			trans->impl->Initialize();
		}
		void CreateInstanceInterface::RegisterCash(JIdentifier* createdPtr)noexcept
		{
			static_cast<JFSMtransition*>(createdPtr)->impl->RegisterInstance();
		}
		bool CreateInstanceInterface::CanCreateInstance(JDITypeDataBase* initData)const noexcept
		{
			const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JFSMtransition::InitData::StaticTypeInfo());
			return isValidPtr && initData->IsValidData();
		}

		void DestroyInstanceInterface::Clear(JIdentifier* ptr, const bool isForced)
		{
			static_cast<JFSMtransition*>(ptr)->impl->Clear();
			JFSMinterfacePrivate::DestroyInstanceInterface::Clear(ptr, isForced);
		}
		void DestroyInstanceInterface::DeRegisterCash(JIdentifier* ptr)noexcept
		{
			static_cast<JFSMtransition*>(ptr)->impl->DeRegisterInstance();
		}

		bool OwnTypeInterface::AddCondition(const JUserPtr<JFSMcondition>& cond)noexcept
		{
			return cond->GetOwner()->impl->AddCondition(cond);
		}
		bool OwnTypeInterface::RemoveCondition(const JUserPtr<JFSMcondition>& cond)noexcept
		{
			return cond->GetOwner()->impl->RemoveCondition(cond);
		}

		void UpdateInterface::Initialize(const JUserPtr<JFSMtransition>& trans)noexcept
		{
			trans->impl->Initialize();
		}

		void ParameterInterface::RemoveParameter(const JUserPtr<JFSMtransition>& trans, const size_t guid)
		{
			trans->impl->RemoveParameter(guid);
		}

		JIdentifierPrivate::CreateInstanceInterface& JFSMtransitionPrivate::GetCreateInstanceInterface()const noexcept
		{
			static CreateInstanceInterface pI;
			return pI;
		}
		JIdentifierPrivate::DestroyInstanceInterface& JFSMtransitionPrivate::GetDestroyInstanceInterface()const noexcept
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