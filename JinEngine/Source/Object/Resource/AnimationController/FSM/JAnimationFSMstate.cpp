#include"JAnimationFSMstate.h" 
#include"JAnimationFSMstatePrivate.h"
#include"JAnimationFSMtransition.h" 
#include"JAnimationFSMtransitionPrivate.h" 
#include"JAnimationTime.h"    
#include"JAnimationUpdateData.h"  
#include"../../../JObjectFileIOHelper.h"
#include"../../../../Core/Fsm/JFSMparameter.h"  
#include"../../../../Core/Fsm/JFSMcondition.h"  
#include"../../../../Core/Identity/JIdenCreator.h"
#include"../../../../Core/Reflection/JTypeImplBase.h"
#include"../../../../Core/File/JFileConstant.h" 
#include"../../../../Core/Guid/JGuidCreator.h" 
#include<fstream>

namespace JinEngine
{
	namespace
	{
		using _TransitionIOInterface = JAnimationFSMtransitionPrivate::AssetDataIOInterface;
		using _TransitionUpdateInterface = JAnimationFSMtransitionPrivate::UpdateInterface;
	}
	namespace
	{
		_TransitionIOInterface* TransitionIOInterface(const JUserPtr<JAnimationFSMtransition>& trans)noexcept
		{
			return &static_cast<_TransitionIOInterface&>(static_cast<JAnimationFSMtransitionPrivate&>(trans->PrivateInterface()).GetAssetDataIOInterface());
		}
		_TransitionUpdateInterface* TransitionUpdateInterface(const JUserPtr<JAnimationFSMtransition>& trans)noexcept
		{
			return &static_cast<_TransitionUpdateInterface&>(static_cast<JAnimationFSMtransitionPrivate&>(trans->PrivateInterface()).GetUpdateInterface());
		}
	}

	class JAnimationFSMstate::JAnimationFSMstateImpl : public Core::JTypeImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JAnimationFSMstateImpl)
	public:
		JWeakPtr<JAnimationFSMstate> thisPointer;
	public:
		JVector2<float> pos;		//for editor node
	public:
		JAnimationFSMstateImpl(const InitData& initData, JAnimationFSMstate* thisStateRaw)
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
	public:
		void RegisterThisPointer(JAnimationFSMstate* fsmState)
		{
			thisPointer = GetWeakPtr(fsmState);
		}
		static void RegisterTypeData()
		{
			IMPL_REALLOC_BIND(JAnimationFSMstate::JAnimationFSMstateImpl, thisPointer)
		}
	};

	JUserPtr<JAnimationFSMtransition> JAnimationFSMstate::GetTransitionByIndex(uint index)noexcept
	{
		return Core::ConvertChildUserPtr<JAnimationFSMtransition>(JFSMstate::GetTransitionByIndex(index));
	}
	JUserPtr<JAnimationFSMtransition> JAnimationFSMstate::FindNextStateTransition(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept
	{
		JAnimationTime& animationTime = updateData->diagramData[layerNumber].animationTimes[updateNumber];
		const uint transitionCount = GetTransitionCount();
		bool hasTransition = false;
		for (uint index = 0; index < transitionCount; ++index)
		{
			JUserPtr<JAnimationFSMtransition> nowTransition = Core::ConvertChildUserPtr<JAnimationFSMtransition>(GetTransitionByIndex(index));
			if (nowTransition->IsSatisfiedOption(animationTime.normalizedTime))
			{
				const uint conditionCount = nowTransition->GetConditioCount();
				for (uint j = 0; j < conditionCount; ++j)
				{
					Core::JFSMcondition* cond = nowTransition->GetConditionByIndex(j).Get();
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

	using CreateInstanceInterface = JAnimationFSMstatePrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JAnimationFSMstatePrivate::AssetDataIOInterface;
	using EditorInterface = JAnimationFSMstatePrivate::EditorInterface;

	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JFSMstatePrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JAnimationFSMstate* state = static_cast<JAnimationFSMstate*>(createdPtr);
		state->impl->RegisterThisPointer(state);
	}

	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadAssetCommonData(JFileIOTool& tool, const JUserPtr<JAnimationFSMstate>& state)
	{
		if (!tool.CanLoad())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		uint transitionCount = 0;
		JObjectFileIOHelper::LoadAtomicData(tool, transitionCount, "TransitionCount:");

		tool.PushExistStack("TransitionMetaData");
		for (uint i = 0; i < transitionCount; ++i)
		{
			std::wstring tName;
			size_t guid;
			Core::J_FSM_OBJECT_TYPE oType;
			size_t outputGuid;
			size_t outputTypeGuid;
			tool.PushExistStack();
			JObjectFileIOHelper::LoadFsmIden(tool, tName, guid, oType);
			JObjectFileIOHelper::LoadAtomicData(tool, outputGuid, Core::JFileConstant::GetHasObjGuidSymbol());
			JObjectFileIOHelper::LoadAtomicData(tool, outputTypeGuid, "OutTypeGuid");
			tool.PopStack();

			auto outuserPtr = Core::GetUserPtr<JAnimationFSMstate>(outputTypeGuid, outputGuid);
			if (outuserPtr != nullptr)
				JICI::Create< JAnimationFSMtransition>(tName, guid, state, outuserPtr);
		}
		tool.PopStack();
		tool.PushExistStack("TransitionData");
		for (uint i = 0; i < transitionCount; ++i)
		{
			tool.PushExistStack();
			TransitionIOInterface(state->GetTransitionByIndex(i))->LoadAssetData(tool, state->GetTransitionByIndex(i));
			tool.PopStack();
		}
		tool.PopStack();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetCommonData(JFileIOTool& tool, const JUserPtr<JAnimationFSMstate>& state)
	{
		if (!tool.CanStore())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JObjectFileIOHelper::StoreAtomicData(tool, state->GetTransitionCount(), "TransitionCount:");
		const uint transitionCount = state->GetTransitionCount();

		tool.PushArrayOwner("TransitionMetaData");
		for (uint i = 0; i < transitionCount; ++i)
		{
			tool.PushArrayMember();
			JUserPtr<JAnimationFSMtransition> nowTran = state->GetTransitionByIndex(i);
			JObjectFileIOHelper::StoreFsmIden(tool, nowTran.Get());
			JObjectFileIOHelper::StoreAtomicData(tool, nowTran->GetOutputStateGuid(), Core::JFileConstant::GetHasObjGuidSymbol());
			JObjectFileIOHelper::StoreAtomicData(tool, nowTran->GetOutState()->GetTypeInfo().TypeGuid(), "OutTypeGuid");
			tool.PopStack();
		}
		tool.PopStack();
		tool.PushArrayOwner("TransitionData");
		for (uint i = 0; i < transitionCount; ++i)
		{
			tool.PushArrayMember();
			TransitionIOInterface(state->GetTransitionByIndex(i))->StoreAssetData(tool, state->GetTransitionByIndex(i));
			tool.PopStack();
		}
		tool.PopStack();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	JVector2<float> EditorInterface::GetPos(const JUserPtr<JAnimationFSMstate>& state) noexcept
	{
		return state->impl->GetPos();
	}
	void EditorInterface::SetPos(const JUserPtr<JAnimationFSMstate>& state, const JVector2<float>& newPos)noexcept
	{
		state->impl->SetPos(newPos);
	}
}
