#pragma once 
#include"JAnimationFSMstate.h"
#include"JAnimationTime.h"
#include"../../../Object/Resource/JResourceUserInterface.h"

namespace JinEngine
{
	class JAnimationClip;
	namespace Core
	{
		struct JAnimationTime;
		class JAnimationFSMstateClip : public JAnimationFSMstate, public JResourceUserInterface
		{
		private:
			JAnimationClip* clip;
		public:
			JAnimationFSMstateClip(const std::string& name, const size_t guid);
		public:
			void Initialize()noexcept final;
			J_ANIMATION_STATE_TYPE GetStateType()const noexcept final;
			void Enter(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const float timeOffset)noexcept override;
			void Update(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const uint updateNumber)noexcept override;
			void Close(JAnimationShareData& animationShareData)noexcept override;
			void GetRegisteredSkeleton(std::vector<JSkeletonAsset*>& skeletonVec)noexcept override;
			void SetClip(JAnimationClip* clip)noexcept;
			void Clear()noexcept final;
		private:
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
		};
	}
}