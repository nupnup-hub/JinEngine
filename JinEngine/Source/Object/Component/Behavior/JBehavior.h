/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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