#include"JAnimationFSMstate.h" 
#include"JAnimationFSMtransition.h" 
#include"JAnimationTime.h"   

namespace JinEngine
{
	namespace Core
	{
		JAnimationFSMstate::JAnimationFSMstate(const std::string& name, const size_t guid)
			:JFSMstate(name, guid)
		{}
		JAnimationFSMstate::~JAnimationFSMstate() {}
		std::vector<JAnimationFSMtransition*>::const_iterator JAnimationFSMstate::GetTransitionVectorHandle(_Out_ uint& transitionCount)
		{
			transitionCount = (uint)transitionCash.size();
			return transitionCash.cbegin();
		}
		JFSMtransition* JAnimationFSMstate::AddTransition(std::unique_ptr<JFSMtransition> newTransition)noexcept
		{
			JFSMtransition* cash = JFSMstate::AddTransition(std::move(newTransition));
			if (cash != nullptr)
			{
				transitionCash.push_back(dynamic_cast<JAnimationFSMtransition*>(cash));
				return cash;
			}
			else
				return cash;
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
	}
}