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
		class JAnimationUpdateData;
		struct JAnimationTime; 
		class JAnimationFSMstate;
		class JAnimationFSMtransition; 
		class JGameTimer;

		class JAnimationFSMdiagram final : public JFSMdiagram
		{
			REGISTER_CLASS(JAnimationFSMdiagram)
		private:
			friend class JAnimationController;
		public:
			void Initialize(JAnimationUpdateData* updateData, const uint layerNumber)noexcept;
			void Enter(JAnimationUpdateData* updateData, const uint layerNumber);
			void Update(JAnimationUpdateData* updateData, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept;
		public: 
			bool CanCreateState()const noexcept;

			JAnimationFSMstate* GetState(const size_t stateGuid)noexcept;
			JAnimationFSMstate* GetStateByIndex(const uint index)noexcept;
			JAnimationFSMtransition* GetTransition(const size_t transitionGuid)noexcept;
			const std::vector<JFSMstate*>& GetStateVec()noexcept;
			 
			void SetClip(const size_t stateGuid, JAnimationClip* clip)noexcept;
		private:
			void StuffFinalTransform(JAnimationUpdateData* updateData, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept;
			void CrossFading(JAnimationUpdateData* updateData, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept;
			void PreprocessSkeletonBindPose(JAnimationUpdateData* updateData)noexcept;
		private:
			J_FILE_IO_RESULT StoreData(std::wofstream& stream);
			static JAnimationFSMdiagram* LoadData(std::wifstream& stream, JFSMdiagramOwnerInterface* fsmOwner);
		private:
			static void RegisterJFunc();
		private:
			JAnimationFSMdiagram(const JFSMdiagramInitData& initData);
			~JAnimationFSMdiagram();
		};
	}
}