#include"JAnimationFSMstate.h" 
#include"JAnimationFSMtransition.h" 
#include"JAnimationTime.h"    
#include"../../File/JFileIOHelper.h"
#include"../../File/JFileConstant.h" 
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
		JAnimationFSMtransition* JAnimationFSMstate::GetTransition(uint index)noexcept
		{
			JFSMtransition* trans = JFSMstate::GetTransition(index);
			if (trans != nullptr)
				return static_cast<JAnimationFSMtransition*>(trans);
			else
				return nullptr;
		}
		void JAnimationFSMstate::SetPos(const JVector2<float>& newPos)
		{
			pos = newPos;
		}
		JAnimationFSMtransition* JAnimationFSMstate::AddTransition(JAnimationFSMstate* outState)noexcept
		{
			if (outState != nullptr)
			{
				JFSMtransition* res = JFSMstate::AddTransition(UniqueAniFSM(new JAnimationFSMtransition(outState->GetGuid())));
				if (res != nullptr)
					return static_cast<JAnimationFSMtransition*>(res);
			}
			return nullptr;
		}
		bool JAnimationFSMstate::RemoveTransition(JAnimationFSMstate* outState)noexcept
		{
			if (outState != nullptr)
				return JFSMstate::RemoveTransition(outState->GetGuid());
			else
				return false;
		}
		JAnimationFSMtransition* JAnimationFSMstate::FindNextStateTransition(JAnimationTime& animationTime)noexcept
		{
			const uint transitionCount = GetTransitionCount();
			bool hasTransition = false; 
			for (uint index = 0; index < transitionCount; ++index)
			{
				JAnimationFSMtransition* nowTransition = static_cast<JAnimationFSMtransition*>(GetTransition(index));
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
				JAnimationFSMtransition* nowTran = GetTransition(i); 
				JFileIOHelper::StoreAtomicData(stream, JFileConstant::StreamHasObjGuidSymbol(), nowTran->GetOutputStateGuid());
				nowTran->StoreData(stream);
			}
			return J_FILE_IO_RESULT::SUCCESS;
		}
		J_FILE_IO_RESULT JAnimationFSMstate::LoadData(std::wifstream& stream, JFSMconditionStorageUserAccess& iFSMconditionStorage)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			uint transitionCount = 0; 
			JFileIOHelper::LoadAtomicData(stream, transitionCount);

			for (uint i = 0; i < transitionCount; ++i)
			{ 
				size_t outputGuid;
				JFileIOHelper::LoadAtomicData(stream, outputGuid);
				JFSMtransition* newTransition = JFSMstate::AddTransition(UniqueAniFSM(new JAnimationFSMtransition(outputGuid)));
				if (newTransition != nullptr)
					static_cast<JAnimationFSMtransition*>(newTransition)->LoadData(stream, iFSMconditionStorage);
			}
			return J_FILE_IO_RESULT::SUCCESS;
		}
		JAnimationFSMstate::JAnimationFSMstate(const JFSMstateInitData& initData)
			:JFSMstate(initData)
		{}
		JAnimationFSMstate::~JAnimationFSMstate() {}
	}
}