#pragma once
#include"../JFSMdiagram.h" 
#include"JBlender.h" 
#include"JAnimationPostProcessing.h"
#include"JAnimationRetargeting.h"

namespace JinEngine
{
	class JAnimationClip;
	class JSkeletonAsset;
	namespace Graphic
	{
		struct JAnimationConstants;
	}

	namespace Core
	{
		struct JAnimationShareData;
		struct JAnimationTime;
		class JAnimationFSMstate;
		class JAnimationFSMtransition;
		class JAnimationFSMdiagram : public JFSMdiagram
		{
		private: 
			std::vector<JAnimationFSMstate*> stateCash;
			JAnimationFSMstate* nowState;
			JAnimationFSMstate* nextState;
			JAnimationFSMtransition* nextTransition;
			JBlender blender;
			float weight;
		public:
			JAnimationFSMdiagram(const std::string& name, const size_t guid, IJFSMconditionStorageUser* conditionStorage);
			~JAnimationFSMdiagram();
			void Initialize(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset)noexcept;
			void Enter(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset);
			void Update(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept;
			bool HasAnimationData()noexcept;

			JAnimationFSMstate* GetState(const size_t stateGuid)noexcept;
			std::vector<JAnimationFSMstate*>& GetStateVec()noexcept;

			void SetStateName(const size_t stateGuid, const std::string& newName)noexcept;
			void SetAnimationClip(const size_t stateGuid, JAnimationClip* clip)noexcept;
			void SetTransitionCondition(const size_t inputStateGuid, const size_t outputStateGuid, const size_t conditionGuid, const uint conditionIndex)noexcept;
			void SetTransitionCondtionOnValue(const size_t inputStateGuid, const size_t outputStateGuid, const uint conditionIndex, const float value)noexcept;

			JAnimationFSMstate* CreateAnimationClipState(const std::string& name, const size_t guid)noexcept;
			JAnimationFSMtransition* CreateAnimationTransition(const size_t inputStateGuid, const size_t outputStateGuid)noexcept;

			bool DestroyAnimationState(const size_t stateGuid)noexcept;
			bool DestroyAnimationTransition(const size_t inputStateGuid, const size_t outputStateGuid)noexcept;
			void Clear()noexcept;

			bool HasState()noexcept;
			void StuffFinalTransform(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, Graphic::JAnimationConstants& animationConstatns)noexcept;
			void CrossFading(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, Graphic::JAnimationConstants& animationConstatns)noexcept;
		private:
			void PreprocessSkeletonBindPose(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset)noexcept;
		};
	}
}