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
			InitData(const JUserPtr<JGameObject>& owner);
			InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner);
		};
	private:
		friend class JAnimatorPrivate;
		class JAnimatorImpl;
	private:
		std::unique_ptr<JAnimatorImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR;
		}
		JUserPtr<JSkeletonAsset>GetSkeletonAsset()const noexcept;
		JUserPtr<JAnimationController> GetAnimatorController()const noexcept;
	public:
		void SetSkeletonAsset(JUserPtr<JSkeletonAsset> newSkeletonAsset)noexcept;
		void SetAnimatorController(JUserPtr<JAnimationController> newAnimationController)noexcept;
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
