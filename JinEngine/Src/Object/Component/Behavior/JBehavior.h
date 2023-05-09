#pragma once
#include"../JComponent.h" 
#include<DirectXMath.h> 

namespace JinEngine
{ 
	class JGameObject;
	class JBehaviorPrivate;
	//¹Ì±¸Çö 
	class JBehavior final : public JComponent
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JBehavior)
	public: 
		class InitData final : public JComponent::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(const JUserPtr<JGameObject>& owner);
			InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner);
		};
	private:
		friend class JBehaviorPrivate;
		class JBehaviorImpl;
	private:
		std::unique_ptr<JBehaviorImpl> impl;
	public:
		Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final;
		J_COMPONENT_TYPE GetComponentType()const noexcept final;  
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::USER_DEFIENED_BEHAVIOR;
		}
	public:
		bool IsAvailableOverlap()const noexcept final; 
		bool PassDefectInspection()const noexcept final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final; 
	private:
		JBehavior(const InitData& initData);
		~JBehavior();
	};
}