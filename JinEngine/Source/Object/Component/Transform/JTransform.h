#pragma once 
#include"../JComponent.h"   
#include"../../../Core/Math/JVector.h"
#include"../../../Core/Math/JMatrix.h"

namespace JinEngine
{
	class JTransform final : public JComponent
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JTransform)
	public: 
		class InitData final : public JComponent::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(const JUserPtr<JGameObject>& owner);
			InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner);
		};
	private:
		friend class JTransformPrivate;
		class JTransformImpl;
	private:
		std::unique_ptr<JTransformImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_TRANSFORM;
		}
		JVector3<float> GetPosition()const noexcept;
		//Euler
		JVector3<float> GetRotation()const noexcept;  
		JVector4<float> GetQuaternion()const noexcept;
		JVector3<float> GetScale()const noexcept;
		JVector3<float> GetWorldPosition()const noexcept;
		JVector3<float> GetWorldRotation()const noexcept;
		JVector4<float> GetWorldQuaternion()const noexcept;
		JMatrix4x4 GetWorldMatrix()const noexcept;  
		JVector3<float> GetRight()const noexcept;
		JVector3<float> GetUp()const noexcept;
		JVector3<float> GetFront()const noexcept;
		JVector3<float> GetWorldRight()const noexcept;
		JVector3<float> GetWorldUp()const noexcept;
		JVector3<float> GetWorldFront()const noexcept;
		float GetDistance(const JUserPtr<JTransform>& t)const noexcept;
		void GetWorldPQS(_Inout_ JVector3<float>& p, _Inout_ JVector4<float>& q, _Inout_ JVector3<float>& s);
	public:
		void SetTransform(const JMatrix4x4& transform)noexcept; 
		void SetTransform(const JVector3<float>& position, const JVector3<float>& rotation, const JVector3<float>& scale)noexcept;
		void SetTransform(const JVector3<float>& position, const JVector4<float>& quaternion, const JVector3<float>& scale)noexcept;
		void SetPosition(const JVector3<float>& value)noexcept;
		//Euler
		void SetRotation(const JVector3<float>& value)noexcept;
		void SetRotation(const JVector4<float>& q)noexcept;
		void SetScale(const JVector3<float>& value)noexcept;
	public:
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final;
	public:
		/**
		* 현재 transform right, up, dir를 축으로하는 변환행렬 생성
		*/  
		void CalTransformMatrix(_Out_ JMatrix4x4& m);
		static void CalTransformMatrix(_Out_ JMatrix4x4& m,
			const JUserPtr<JTransform>& t,
			const JVector3<float>& position,
			const JVector3<float>& right,
			const JVector3<float>& up,
			const JVector3<float>& front);
	public:
		void LookAt(const JVector3<float>& target, const JVector3<float>& worldUp = JVector3<float>(0, 1, 0))noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		JTransform(const InitData& initData);
		~JTransform();
	};
}
 