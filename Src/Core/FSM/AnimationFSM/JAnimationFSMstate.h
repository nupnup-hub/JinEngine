#pragma once
#include"../JFSMstate.h"
#include"JAnimationStateType.h"  

namespace JinEngine
{
	class JSkeletonAsset;
	struct EditorDiagramNode;
	namespace Core
	{
		struct JAnimationShareData;
		struct JAnimationTime;
		class JAnimationFSMtransition;
		class JAnimationFSMstate : public JFSMstate
		{
		protected:
			std::unique_ptr<EditorDiagramNode> editorCoord;
			std::vector<JAnimationFSMtransition*> transitionCash;
		public:
			JAnimationFSMstate();
			~JAnimationFSMstate();
			virtual J_ANIMATION_STATE_TYPE GetStateType()const noexcept = 0;
			virtual void Enter(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const float timeOffset)noexcept = 0;
			virtual void Update(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const uint updateNumber)noexcept = 0;
			virtual void Close(JAnimationShareData& animationShareData)noexcept = 0;
			virtual void GetRegisteredSkeleton(std::vector<JSkeletonAsset*>& skeletonVec)noexcept = 0;

			EditorDiagramNode* GetEditorDiagramNode()noexcept;
			std::vector<JAnimationFSMtransition*>::const_iterator GetTransitionVectorHandle(_Out_ uint& transitionCount);

			JFSMtransition* AddTransition(std::unique_ptr<JFSMtransition> newTransition)noexcept override;
			bool EraseTransition(const size_t outputId)noexcept override;
			JAnimationFSMtransition* FindNextStateTransition(JAnimationTime& animationTime)noexcept;
		};
	}
}