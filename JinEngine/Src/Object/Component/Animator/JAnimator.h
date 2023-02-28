#pragma once
#include"JAnimatorInterface.h"
#include"../../Resource/Skeleton/JSkeletonAsset.h"
#include"../../Resource/AnimationController/JAnimationController.h"
#include<memory>
#include<vector>
#include<DirectXMath.h>

namespace JinEngine
{  
	class JSkeletonAsset; 
	class JAnimationController; 

	namespace Core
	{
		class JAnimationUpdateData;
	}
	class JAnimator final : public JAnimatorInterface
	{
		REGISTER_CLASS(JAnimator)
	private: 
		REGISTER_PROPERTY_EX(skeletonAsset, GetSkeletonAsset, SetSkeletonAsset, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::NONE, false))
		JSkeletonAsset* skeletonAsset = nullptr;
		REGISTER_PROPERTY_EX(animationController, GetAnimatorController, SetAnimatorController, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::NONE, false))
		JAnimationController* animationController = nullptr;
		Core::JGameTimer* userTimer = nullptr;
		std::unique_ptr<Core::JAnimationUpdateData> animationUpdateData;
	private:
		bool reqSettingAniData = false;
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
		void SetParameterValue(const std::wstring& paramName, const float value)noexcept;
	public:
		bool IsAvailableOverlap()const noexcept final; 
		bool PassDefectInspection()const noexcept final; 
	public:
		JAnimatorFrameUpdateTriggerInterface* UpdateTriggerInterface();
	private:
		void OnAnimationUpdate(Core::JGameTimer* sceneTimer)noexcept final;
		void OffAnimationUpdate()noexcept final;
		void SettingAnimationUpdateData()noexcept;
		void ClearAnimationUpdateData()noexcept;
	private:
		bool CanUpdateAnimation()const noexcept;
	private:
		void DoCopy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private: 
		void UpdateFrame(Graphic::JAnimationConstants& constant) final;
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