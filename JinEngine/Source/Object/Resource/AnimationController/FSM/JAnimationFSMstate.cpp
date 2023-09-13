#include"JAnimationFSMstate.h" 
#include"JAnimationFSMstatePrivate.h"
#include"JAnimationFSMtransition.h" 
#include"JAnimationFSMtransitionPrivate.h" 
#include"JAnimationTime.h"    
#include"JAnimationUpdateData.h"  
#include"../../../../Core/Fsm/JFSMcondition.h"  
#include"../../../../Core/Fsm/JFSMparameter.h"  
#include"../../../../Core/Identity/JIdenCreator.h"
#include"../../../../Core/Reflection/JTypeImplBase.h"
#include"../../../../Core/File/JFileIOHelper.h"
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

	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadAssetCommonData(std::wifstream& stream, const JUserPtr<JAnimationFSMstate>& state)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		uint transitionCount = 0;
		JFileIOHelper::LoadAtomicData(stream, transitionCount);

		for (uint i = 0; i < transitionCount; ++i)
		{
			std::wstring tName;
			size_t guid;
			Core::J_FSM_OBJECT_TYPE oType;
			size_t outputGuid;
			size_t outputTypeGuid;
			JFileIOHelper::LoadFsmObjectIden(stream, tName, guid, oType);
			JFileIOHelper::LoadAtomicData(stream, outputGuid);
			JFileIOHelper::LoadAtomicData(stream, outputTypeGuid);

			auto outuserPtr = Core::GetUserPtr<JAnimationFSMstate>(outputTypeGuid, outputGuid);
			if (outuserPtr != nullptr)
				JICI::Create< JAnimationFSMtransition>(tName, guid, state, outuserPtr);
		}
		for (uint i = 0; i < transitionCount; ++i)
			TransitionIOInterface(state->GetTransitionByIndex(i))->LoadAssetData(stream, state->GetTransitionByIndex(i));
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetCommonData(std::wofstream& stream, const JUserPtr<JAnimationFSMstate>& state)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JFileIOHelper::StoreAtomicData(stream, L"TransitionCount:", state->GetTransitionCount());
		const uint transitionCount = state->GetTransitionCount();

		for (uint i = 0; i < transitionCount; ++i)
		{
			JUserPtr<JAnimationFSMtransition> nowTran = state->GetTransitionByIndex(i);
			JFileIOHelper::StoreFsmObjectIden(stream, nowTran.Get());
			JFileIOHelper::StoreAtomicData(stream, Core::JFileConstant::StreamHasObjGuidSymbol(), nowTran->GetOutputStateGuid());
			JFileIOHelper::StoreAtomicData(stream, L"OutTypeGuid", nowTran->GetOutState()->GetTypeInfo().TypeGuid());
		}
		for (uint i = 0; i < transitionCount; ++i)
			TransitionIOInterface(state->GetTransitionByIndex(i))->StoreAssetData(stream, state->GetTransitionByIndex(i));
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
