#pragma once
#include"JAnimatorInterface.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationTime.h"
#include<memory>
#include<vector>
#include<DirectXMath.h>

namespace JinEngine
{  
	class JSkeletonAsset;
	class JAnimationClip;
	class JAnimationController; 

	class JAnimator : public JAnimatorInterface
	{
		REGISTER_CLASS(JAnimator)
	private: 
		JSkeletonAsset* skeletonAsset;
		JAnimationController* animationController;
		std::vector<Core::JAnimationTime>animationTimes; 
	public:
		void OnAnimation()noexcept;
		JAnimationController* GetAnimatorController()const noexcept; 
		JSkeletonAsset* GetSkeletonAsset()noexcept;

		void SetAnimatorController(JAnimationController* animationController)noexcept;
		void SetSkeletonAsset(JSkeletonAsset* newSkeletonAsset)noexcept;

		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static J_COMPONENT_TYPE GetStaticComponentType()noexcept;
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final; 
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private: 
		bool UpdateFrame(Graphic::JAnimationConstants& constant) final;
	private:
		Core::J_FILE_IO_RESULT CallStoreComponent(std::wofstream& stream)final;
		static Core::J_FILE_IO_RESULT StoreObject(std::wofstream& stream, JAnimator* animator);
		static JAnimator* LoadObject(std::wifstream& stream, JGameObject* owner);
		static void RegisterFunc();
	private:
		JAnimator(const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner);
		~JAnimator();
	};
}