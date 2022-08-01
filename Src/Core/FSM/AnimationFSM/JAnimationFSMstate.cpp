#include"JAnimationFSMstate.h" 
#include"JAnimationFSMtransition.h" 
#include"JAnimationTime.h"  
#include"../../../Window/Editor/Diagram/EditorDiagramNode.h"

namespace JinEngine
{
	namespace Core
	{
		JAnimationFSMstate::JAnimationFSMstate()
		{
			editorCoord = std::make_unique<EditorDiagramNode>();
		}
		JAnimationFSMstate::~JAnimationFSMstate() {}
		EditorDiagramNode* JAnimationFSMstate::GetEditorDiagramNode()noexcept
		{
			return editorCoord.get();
		}
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
		bool JAnimationFSMstate::EraseTransition(const size_t outputId)noexcept
		{
			if (JFSMstate::EraseTransition(outputId))
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