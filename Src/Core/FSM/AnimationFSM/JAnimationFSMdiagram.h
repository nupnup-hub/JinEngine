#pragma once
#include"../JFSMdiagram.h" 
#include"JBlender.h" 
#include"JAnimationPostProcessing.h"
#include"JAnimationRetargeting.h" 

namespace JinEngine
{
	class JAnimationClip; 
	class JAnimationController;
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
		class JAnimationFSMdiagram final : public JFSMdiagram
		{
			REGISTER_CLASS(JAnimationFSMdiagram)
		private:
			friend class JAnimationController;
		private:  
			JAnimationFSMstate* nowState;
			JAnimationFSMstate* nextState;
			JAnimationFSMtransition* nextTransition;
			JBlender blender;
			float weight;
		public:
			void Initialize(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset)noexcept;
			void Enter(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset);
			void Update(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept;
		public:
			bool HasNowState()const noexcept;
			bool CanCreateState()const noexcept;

			JAnimationFSMstate* GetState(const size_t stateGuid)noexcept;
			JAnimationFSMstate* GetStateByIndex(const uint index)noexcept;
			const std::vector<JFSMstate*>& GetStateVec()noexcept;
			 
			void SetClip(const size_t stateGuid, JAnimationClip* clip)noexcept; 
			void Clear()noexcept;
		private:
			void StuffFinalTransform(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, Graphic::JAnimationConstants& animationConstatns)noexcept;
			void CrossFading(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, Graphic::JAnimationConstants& animationConstatns)noexcept;
			void PreprocessSkeletonBindPose(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset)noexcept;
		private:
			J_FILE_IO_RESULT StoreData(std::wofstream& stream);
			static JAnimationFSMdiagram* LoadData(std::wifstream& stream, JUserPtr<IJFSMdiagramOwner> fsmOwner);
		private:
			static void RegisterJFunc();
		private:
			JAnimationFSMdiagram(const JFSMdiagramInitData& initData);
			~JAnimationFSMdiagram();
		};
	}
}