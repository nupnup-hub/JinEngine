#include"JAnimationFSMstate.h" 
#include"JAnimationFSMtransition.h" 
#include"JAnimationTime.h"   
#include<fstream>

namespace JinEngine
{
	namespace Core
	{
		JAnimationFSMstate::JAnimationFSMstate(const std::wstring& name, const size_t guid)
			:JFSMstate(name, guid)
		{}
		JAnimationFSMstate::~JAnimationFSMstate() {}
		std::vector<JAnimationFSMtransition*>& JAnimationFSMstate::GetTransitionVector()noexcept
		{
			return transitionCash;
		}
		JAnimationFSMtransition* JAnimationFSMstate::AddTransition(std::unique_ptr<JFSMtransition> newTransition)noexcept
		{
			JFSMtransition* ret = JFSMstate::AddTransition(std::move(newTransition));
			if (ret != nullptr)
			{
				transitionCash.push_back(static_cast<JAnimationFSMtransition*>(ret));
				return static_cast<JAnimationFSMtransition*>(ret);
			}
			else
				return nullptr;
		}
		bool JAnimationFSMstate::RemoveTransition(const size_t outputStateGuid)noexcept
		{
			if (JFSMstate::RemoveTransition(outputStateGuid))
			{
				uint cashSize = (uint)transitionCash.size();
				for (uint i = 0; i < cashSize; ++i)
				{
					if (transitionCash[i] == nullptr)
						transitionCash.erase(transitionCash.begin() + i);
				}
				return true;
			}
			return false;
		}
		JAnimationFSMtransition* JAnimationFSMstate::FindNextStateTransition(JAnimationTime& animationTime)noexcept
		{
			uint transitionSize = (uint)transitionCash.size();
			bool hasTransition = false;
			uint index = 0;
			for (index; index < transitionSize; ++index)
			{
				if (transitionCash[index]->IsSatisfiedOption(animationTime.normalizedTime) &&
					transitionCash[index]->HasSatisfiedCondition())
				{
					hasTransition = true;
					return transitionCash[index];
				}
			}
			return nullptr;
		}
		J_FILE_IO_RESULT JAnimationFSMstate::StoreIdentifierData(std::wofstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
			 			
			return JFSMIdentifier::StoreIdentifierData(stream, *this);
		}
		J_FILE_IO_RESULT JAnimationFSMstate::StoreContentsData(std::wofstream& stream)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			const uint transitionCount = (uint)transitionCash.size();
			stream << transitionCount << '\n';
			for (uint i = 0; i < transitionCount; ++i)
				transitionCash[i]->StoreData(stream);

			return J_FILE_IO_RESULT::SUCCESS;
		}
		J_FILE_IO_RESULT JAnimationFSMstate::LoadContentsData(std::wifstream& stream, JFSMLoadGuidMap& guidMap, IJFSMconditionStorageUser& iFSMconditionStorage)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			uint transitionCount = 0;
			stream >> transitionCount; 

			for (uint i = 0; i < transitionCount; ++i)
			{
				std::unique_ptr<JAnimationFSMtransition> newTransition = JAnimationFSMtransition::LoadData(stream, guidMap, iFSMconditionStorage);
				if(newTransition != nullptr)
					AddTransition(std::move(newTransition));
			}
			return J_FILE_IO_RESULT::SUCCESS;
		}
	}
}