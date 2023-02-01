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
		class JAnimationFSMstateClip final : public JAnimationFSMstate, public JResourceUserInterface
		{
			REGISTER_CLASS(JAnimationFSMstateClip)
		private:
			JAnimationClip* clip = nullptr;
		public:
			void Initialize()noexcept final;
			J_ANIMATION_STATE_TYPE GetStateType()const noexcept final;
			void Enter(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const float timeOffset)noexcept override;
			void Update(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, const uint updateNumber)noexcept override;
			void Close(JAnimationShareData& animationShareData)noexcept override;
			void GetRegisteredSkeleton(std::vector<JSkeletonAsset*>& skeletonVec)noexcept override;
			void SetClip(JAnimationClip* newClip)noexcept;
		protected:
			void Clear()noexcept final;
		private:
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
		private:
			J_FILE_IO_RESULT StoreData(std::wofstream& stream)final;
			J_FILE_IO_RESULT LoadData(std::wifstream& stream, JFSMconditionStorageUserAccess& iConditionUser)final;
		private:
			static void RegisterJFunc();
		private:
			JAnimationFSMstateClip(const JFSMstateInitData& initData);
			~JAnimationFSMstateClip();
			JAnimationFSMstateClip(const JAnimationFSMstateClip& rhs) = delete;
			JAnimationFSMstateClip& operator=(const JAnimationFSMstateClip& rhs) = delete;
		};
	}
}