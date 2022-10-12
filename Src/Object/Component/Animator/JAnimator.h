#pragma once
#include"JAnimatorInterface.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationTime.h"
#include<memory>
#include<vector>
#include<DirectXMath.h>

namespace JinEngine
{  
	class JSkeletonAsset; 
	class JAnimationController; 

	class JAnimator final : public JAnimatorInterface
	{
		REGISTER_CLASS(JAnimator)
	private: 
		JSkeletonAsset* skeletonAsset;
		JAnimationController* animationController;
		std::vector<Core::JAnimationTime>animationTimes; 
	public:
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR;
		}
	public:
		JSkeletonAsset* GetSkeletonAsset()noexcept;
		JAnimationController* GetAnimatorController()const noexcept; 

		void SetSkeletonAsset(JSkeletonAsset* newSkeletonAsset)noexcept;
		void SetAnimatorController(JAnimationController* newAnimationController)noexcept;

		bool IsAvailableOverlap()const noexcept final; 
		bool PassDefectInspection()const noexcept final; 

		void OnAnimation()noexcept;
	private:
		void DoCopy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private: 
		bool UpdateFrame(Graphic::JAnimationConstants& constant) final;
	private:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
	private:
		Core::J_FILE_IO_RESULT CallStoreComponent(std::wofstream& stream)final;
		static Core::J_FILE_IO_RESULT StoreObject(std::wofstream& stream, JAnimator* animator);
		static JAnimator* LoadObject(std::wifstream& stream, JGameObject* owner);
		static void RegisterJFunc();
	private:
		JAnimator(const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
		~JAnimator();
	};
}