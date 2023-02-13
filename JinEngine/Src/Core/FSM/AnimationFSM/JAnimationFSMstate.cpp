#include"JAnimationFSMstate.h" 
#include"JAnimationFSMtransition.h" 
#include"JAnimationTime.h"    
#include"../JFSMfactory.h"
#include"../../File/JFileIOHelper.h"
#include"../../File/JFileConstant.h" 
#include"../../Guid/GuidCreator.h"
#include<fstream>

namespace JinEngine
{
	namespace Core
	{ 
		using UniqueAniFSM = std::unique_ptr<JAnimationFSMtransition>;
		JVector2<float> JAnimationFSMstate::GetPos()const noexcept
		{
			return pos;
		}
		JAnimationFSMtransition* JAnimationFSMstate::GetTransitionByIndex(uint index)noexcept
		{
			JFSMtransition* trans = JFSMstate::GetTransitionByIndex(index);
			if (trans != nullptr)
				return static_cast<JAnimationFSMtransition*>(trans);
			else
				return nullptr;
		}
		void JAnimationFSMstate::SetPos(const JVector2<float>& newPos)
		{
			pos = newPos;
		}
		JAnimationFSMtransition* JAnimationFSMstate::FindNextStateTransition(JAnimationTime& animationTime)noexcept
		{
			const uint transitionCount = GetTransitionCount();
			bool hasTransition = false; 
			for (uint index = 0; index < transitionCount; ++index)
			{
				JAnimationFSMtransition* nowTransition = static_cast<JAnimationFSMtransition*>(GetTransitionByIndex(index));
				if (nowTransition->IsSatisfiedOption(animationTime.normalizedTime) && nowTransition->HasSatisfiedCondition())
				{
					hasTransition = true;
					return nowTransition;
				}
			}
			return nullptr;
		}
		JAnimationFSMstateStreamInteface* JAnimationFSMstate::StreamInterface()
		{
			return this;
		}
		J_FILE_IO_RESULT JAnimationFSMstate::StoreData(std::wofstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			JFileIOHelper::StoreAtomicData(stream, L"TransitionCount:", GetTransitionCount());
			const uint transitionCount = GetTransitionCount();
	 
			for (uint i = 0; i < transitionCount; ++i)
			{
				JAnimationFSMtransition* nowTran = GetTransitionByIndex(i); 
				JFileIOHelper::StoreFsmObjectIden(stream, nowTran); 
				JFileIOHelper::StoreAtomicData(stream, JFileConstant::StreamHasObjGuidSymbol(), nowTran->GetOutputStateGuid());
			}
			for (uint i = 0; i < transitionCount; ++i)
				GetTransitionByIndex(i)->StoreData(stream);
			return J_FILE_IO_RESULT::SUCCESS;
		}
		J_FILE_IO_RESULT JAnimationFSMstate::LoadData(std::wifstream& stream)
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
				JFileIOHelper::LoadFsmObjectIden(stream, tName, guid, oType); 
				JFileIOHelper::LoadAtomicData(stream, outputGuid); 
				JFFI<JAnimationFSMtransition>::Create(Core::JPtrUtil::MakeOwnerPtr<JAnimationFSMtransition::InitData>
					(tName, guid, outputGuid, Core::GetUserPtr<JFSMstate>(this)));
			}
			for (uint i = 0; i < transitionCount; ++i)
				GetTransitionByIndex(i)->LoadData(stream);
			return J_FILE_IO_RESULT::SUCCESS;
		}
		JAnimationFSMstate::JAnimationFSMstate(const JFSMstateInitData& initData)
			:JFSMstate(initData)
		{}
		JAnimationFSMstate::~JAnimationFSMstate() {}
	}
}