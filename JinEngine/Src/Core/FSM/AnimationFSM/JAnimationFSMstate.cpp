#include"JAnimationFSMstate.h" 
#include"JAnimationFSMstatePrivate.h"
#include"JAnimationFSMtransition.h" 
#include"JAnimationFSMtransitionPrivate.h" 
#include"JAnimationTime.h"    
#include"JAnimationUpdateData.h"  
#include"../JFSMcondition.h"  
#include"../JFSMparameter.h"  
#include"../../Identity/JIdenCreator.h"
#include"../../Identity/JIdentifierImplBase.h"
#include"../../File/JFileIOHelper.h"
#include"../../File/JFileConstant.h" 
#include"../../Guid/GuidCreator.h" 
#include<fstream>

namespace JinEngine
{
	namespace Core
	{
		namespace
		{ 
			using _TransitionIOInterface = JAnimationFSMtransitionPrivate::AssetDataIOInterface;
			using _TransitionUpdateInterface = JAnimationFSMtransitionPrivate::UpdateInterface;
		}
		namespace
		{ 
			_TransitionIOInterface* TransitionIOInterface(JAnimationFSMtransition* trans)
			{
				return &static_cast<_TransitionIOInterface&>(static_cast<JAnimationFSMtransitionPrivate&>(trans->GetPrivateInterface()).GetAssetDataIOInterface());
			}
			_TransitionUpdateInterface* TransitionUpdateInterface(JAnimationFSMtransition* trans)
			{
				return &static_cast<_TransitionUpdateInterface&>(static_cast<JAnimationFSMtransitionPrivate&>(trans->GetPrivateInterface()).GetUpdateInterface());
			}
		}
		 
		class JAnimationFSMstate::JAnimationFSMstateImpl : public JIdentifierImplBase
		{
			REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JAnimationFSMstateImpl) 
		public:
			JVector2<float> pos;		//for editor node
		public:
			JAnimationFSMstateImpl(const InitData& initData, JAnimationFSMstate* thisState)
			{}
			~JAnimationFSMstateImpl()
			{}
		public:
			JVector2<float> GetPos()const noexcept
			{
				return pos;
			}
			void SetPos(const JVector2<float>& newPos)
			{
				pos = newPos;
			}
		};
		using UniqueAniFSM = std::unique_ptr<JAnimationFSMtransition>;
 
		JAnimationFSMtransition* JAnimationFSMstate::GetTransitionByIndex(uint index)noexcept
		{
			JFSMtransition* trans = JFSMstate::GetTransitionByIndex(index);
			if (trans != nullptr)
				return static_cast<JAnimationFSMtransition*>(trans);
			else
				return nullptr;
		}
		JAnimationFSMtransition* JAnimationFSMstate::FindNextStateTransition(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
		{
			JAnimationTime& animationTime = updateData->diagramData[layerNumber].animationTimes[updateNumber];
			const uint transitionCount = GetTransitionCount();
			bool hasTransition = false; 
			for (uint index = 0; index < transitionCount; ++index)
			{
				JAnimationFSMtransition* nowTransition = static_cast<JAnimationFSMtransition*>(GetTransitionByIndex(index));
				if (nowTransition->IsSatisfiedOption(animationTime.normalizedTime))
				{
					const uint conditionCount = nowTransition->GetConditioCount();
					for (uint j = 0; j < conditionCount; ++j)
					{
						JFSMcondition* cond = nowTransition->GetConditionByIndex(j);
						const float nowValue = updateData->GetParameterValue(cond->GetParameter()->GetGuid());
						if (nowValue == cond->GetOnValue())
							return nowTransition;
					}
				}
			}
			return nullptr;
		} 
		JAnimationFSMstate::JAnimationFSMstate(const InitData& initData)
			:JFSMstate(initData), impl(std::make_unique<JAnimationFSMstateImpl>(initData, this))
		{}
		JAnimationFSMstate::~JAnimationFSMstate() {}

		using AssetDataIOInterface = JAnimationFSMstatePrivate::AssetDataIOInterface;
		using EditorInterface = JAnimationFSMstatePrivate::EditorInterface;

		J_FILE_IO_RESULT AssetDataIOInterface::LoadAssetCommonData(std::wifstream& stream, JAnimationFSMstate* state)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			uint transitionCount = 0;
			JFileIOHelper::LoadAtomicData(stream, transitionCount);

			for (uint i = 0; i < transitionCount; ++i)
			{
				std::wstring tName;
				size_t guid;
				J_FSM_OBJECT_TYPE oType;
				size_t outputGuid;
				size_t outputTypeGuid;
				JFileIOHelper::LoadFsmObjectIden(stream, tName, guid, oType);
				JFileIOHelper::LoadAtomicData(stream, outputGuid);
				JFileIOHelper::LoadAtomicData(stream, outputTypeGuid);

				auto outuserPtr = Core::GetUserPtr<JAnimationFSMstate>(outputTypeGuid, outputGuid);
				if(outuserPtr != nullptr)
					JICI::Create< JAnimationFSMtransition>(tName, guid, Core::GetUserPtr(state), outuserPtr);
			}
			for (uint i = 0; i < transitionCount; ++i)
				TransitionIOInterface(state->GetTransitionByIndex(i))->LoadAssetData(stream, state->GetTransitionByIndex(i));
			return J_FILE_IO_RESULT::SUCCESS;
		}
		J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetCommonData(std::wofstream& stream, JAnimationFSMstate* state)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			JFileIOHelper::StoreAtomicData(stream, L"TransitionCount:", state->GetTransitionCount());
			const uint transitionCount = state->GetTransitionCount();

			for (uint i = 0; i < transitionCount; ++i)
			{
				JAnimationFSMtransition* nowTran = state->GetTransitionByIndex(i);
				JFileIOHelper::StoreFsmObjectIden(stream, nowTran);
				JFileIOHelper::StoreAtomicData(stream, JFileConstant::StreamHasObjGuidSymbol(), nowTran->GetOutputStateGuid());
				JFileIOHelper::StoreAtomicData(stream, L"OutTypeGuid", nowTran->GetOutState()->GetTypeInfo().TypeGuid());
			}
			for (uint i = 0; i < transitionCount; ++i)
				TransitionIOInterface(state->GetTransitionByIndex(i))->StoreAssetData(stream, state->GetTransitionByIndex(i));
			return J_FILE_IO_RESULT::SUCCESS;
		}

		JVector2<float> EditorInterface::GetPos(JAnimationFSMstate* state) noexcept
		{
			return state->impl->GetPos();
		}
		void EditorInterface::SetPos(JAnimationFSMstate* state, const JVector2<float>& newPos)noexcept
		{
			state->impl->SetPos(newPos);
		}		 
	}
}