#pragma once
#include"../JComponent.h"  

namespace JinEngine
{  
	namespace Core
	{
		class JFSMparameter;
	}
	class JSkeletonAsset; 
	class JAnimationController; 
	class JAnimatorPrivate;
	class JAnimator final : public JComponent
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JAnimator)
	public: 
		class InitData final: public JComponent::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(JGameObject* owner);
			InitData(const size_t guid, const J_OBJECT_FLAG flag, JGameObject* owner);
		};
	private:
		friend class JAnimatorPrivate;
		class JAnimatorImpl;
	private:
		std::unique_ptr<JAnimatorImpl> impl;
	public:
		Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final;
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR;
		}
		Core::JUserPtr<JSkeletonAsset>GetSkeletonAsset()const noexcept;
		Core::JUserPtr<JAnimationController> GetAnimatorController()const noexcept;
	public:
		void SetSkeletonAsset(Core::JUserPtr<JSkeletonAsset> newSkeletonAsset)noexcept;
		void SetAnimatorController(Core::JUserPtr<JAnimationController> newAnimationController)noexcept;
		void SetParameterValue(Core::JFSMparameter* param, const float value)noexcept; 
	public:
		bool IsAvailableOverlap()const noexcept final; 
		bool PassDefectInspection()const noexcept final;  
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		JAnimator(const InitData& initData);
		~JAnimator();
	};
}
