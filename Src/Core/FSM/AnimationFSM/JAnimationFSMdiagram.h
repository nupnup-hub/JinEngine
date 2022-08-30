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
		struct JFSMLoadGuidMap;
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
			JAnimationFSMdiagram(const std::wstring& name, const size_t guid, IJFSMconditionStorageUser* conditionStorage);
			~JAnimationFSMdiagram();
		public:
			void Initialize(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset)noexcept;
			void Enter(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset);
			void Update(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept;
		public:
			bool HasAnimationData()noexcept;
			bool HasState()noexcept;

			JAnimationFSMstate* GetState(const size_t stateGuid)noexcept;
			std::vector<JAnimationFSMstate*>& GetStateVec()noexcept;
			 
			void SetAnimationClip(const size_t stateGuid, JAnimationClip* clip)noexcept;
			void SetTransitionCondition(const size_t inputStateGuid, const size_t outputStateGuid, const size_t conditionGuid, const uint conditionIndex)noexcept;
			void SetTransitionCondtionOnValue(const size_t inputStateGuid, const size_t outputStateGuid, const uint conditionIndex, const float value)noexcept;

			JAnimationFSMstate* CreateAnimationClipState(const std::wstring& name)noexcept;
			JAnimationFSMtransition* CreateAnimationTransition(const size_t inputStateGuid, const size_t outputStateGuid)noexcept;

			bool DestroyAnimationState(const size_t stateGuid)noexcept;
			bool DestroyAnimationTransition(const size_t inputStateGuid, const size_t outputStateGuid)noexcept;
			void Clear()noexcept;
		private:
			void StuffFinalTransform(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, Graphic::JAnimationConstants& animationConstatns)noexcept;
			void CrossFading(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, Graphic::JAnimationConstants& animationConstatns)noexcept;
			void PreprocessSkeletonBindPose(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset)noexcept;
		public:
			J_FILE_IO_RESULT StoreIdentifierData(std::wofstream& stream);
			J_FILE_IO_RESULT StoreContentsData(std::wofstream& stream);
			static std::unique_ptr<JAnimationFSMdiagram> LoadIdentifierData(std::wifstream& stream, JFSMLoadGuidMap& guidMap, IJFSMconditionStorageUser* conditionStorage);
			J_FILE_IO_RESULT LoadContentsData(std::wifstream& stream, JFSMLoadGuidMap& guidMap);
		};
	}
}