#include"JAnimationFSMstateClip.h" 
#include"JAnimationTime.h"
#include"JAnimationShareData.h"
#include"JAnimationFSMtransition.h"
#include"../../GameTimer/JGameTimer.h"
#include"../../../Object/Resource/AnimationClip/JAnimationClip.h"
#include"../../../Object/Resource/Skeleton/JSkeletonAsset.h"

namespace JinEngine
{
	namespace Core
	{
		JAnimationFSMstateClip::JAnimationFSMstateClip()
			:JAnimationFSMstate()
		{

		}
		void JAnimationFSMstateClip::Initialize()noexcept
		{
			JFSMstate::Initialize();
		}
		J_ANIMATION_STATE_TYPE JAnimationFSMstateClip::GetStateType()const noexcept
		{
			return J_ANIMATION_STATE_TYPE::CLIP;
		}
		void JAnimationFSMstateClip::Enter(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const float timeOffset)noexcept
		{
			clip->ClipEnter(animationTime, animationShareData, srcSkeletonAsset, JGameTimer::Instance().TotalTime(), timeOffset);
		}
		void JAnimationFSMstateClip::Update(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const uint updateNumber)noexcept
		{
			animationShareData.ClearSkeletonBlendRate(updateNumber);
			if (clip != nullptr)
			{
				clip->Update(animationTime,
					animationShareData,
					srcSkeletonAsset,
					animationShareData.localTransform[updateNumber],
					JGameTimer::Instance().TotalTime(),
					JGameTimer::Instance().DeltaTime());

				size_t clipGuid = clip->GetClipSkeletonAsset()->GetGuid();
				animationShareData.skeletonBlendRate[updateNumber][clipGuid] = 1;
				animationShareData.lastState[updateNumber] = GetStateType();
			}
			else
				animationShareData.StuffIdentity(updateNumber);
		}
		void JAnimationFSMstateClip::Close(JAnimationShareData& animationShareData)noexcept
		{
			clip->ClipClose();
		}
		void JAnimationFSMstateClip::GetRegisteredSkeleton(std::vector<JSkeletonAsset*>& skeletonVec)noexcept
		{
			if (clip != nullptr)
				skeletonVec.push_back(clip->GetClipSkeletonAsset());
		}
		void JAnimationFSMstateClip::SetClip(JAnimationClip* clip)noexcept
		{
			JAnimationFSMstateClip::clip = clip;
		}
		void JAnimationFSMstateClip::Clear()noexcept
		{
			JFSMstate::Clear();
			if (clip != nullptr)
				clip->OffReference();
		}
	}
}