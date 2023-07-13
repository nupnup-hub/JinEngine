#pragma once
#include"../JComponent.h" 
#include<DirectXMath.h> 

namespace JinEngine
{ 
	class JGameObject;
	class JBehaviorPrivate;
	class JBehavior : public JComponent
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JBehavior)
	public: 
		class InitData : public JComponent::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(const Core::JTypeInfo& typeInfo, const JUserPtr<JGameObject>& owner);
			InitData(const Core::JTypeInfo& typeInfo, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner);
		};
	public:
		struct DerivedTypeData
		{
		public:
			using CreatePtr = JOwnerPtr<Core::JIdentifier>(*)(Core::JDITypeDataBase* initData);
		public:
			CreatePtr createPtr;
		};
	private:
		friend class JBehaviorPrivate;
		class JBehaviorImpl;
	private:
		std::unique_ptr<JBehaviorImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
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
	protected:
		virtual void NotifyActivate();
		virtual void NotifyDeActivate();
	protected:
		virtual void Initialize();
		virtual void Clear();
		virtual bool Copy(JUserPtr<Core::JIdentifier> to);			//call after JBehavior copied
	protected:
		virtual void Update() = 0;
	protected: 
		static void RegisterDerivedData(const Core::JTypeInfo& info, const DerivedTypeData& derivedData);
	protected:
		JBehavior(const InitData& initData);
		~JBehavior();
	};
}