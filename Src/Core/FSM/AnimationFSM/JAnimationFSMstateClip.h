#pragma once 
#include"JAnimationFSMstate.h"
#include"JAnimationTime.h"

namespace JinEngine
{
	class JAnimationClip;
	namespace Core
	{
		struct JAnimationTime;
		class JAnimationFSMstateClip : public JAnimationFSMstate
		{
		private:
			JAnimationClip* clip;
		public:
			JAnimationFSMstateClip();
			void Initialize()noexcept final;
			J_ANIMATION_STATE_TYPE GetStateType()const noexcept final;
			void Enter(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const float timeOffset)noexcept override;
			void Update(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const uint updateNumber)noexcept override;
			void Close(JAnimationShareData& animationShareData)noexcept override;
			void GetRegisteredSkeleton(std::vector<JSkeletonAsset*>& skeletonVec)noexcept override;
			void SetClip(JAnimationClip* clip)noexcept;

			void Clear()noexcept final;
		};
	}
}