#pragma once
#include"JCameraState.h"
#include"../JComponent.h" 
#include"../../JFrameUpdateUserAccess.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceUserAccess.h"
#include<DirectXCollision.h>

namespace JinEngine
{
	class JTransform; 
	class JCameraPrivate;
	class JCamera final : public JComponent,
		public JFrameUpdateUserAccessInterface,
		public Graphic::JGraphicResourceUserAccess
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JCamera)
	public: 
		class InitData final : public JComponent::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(JGameObject* owner);
			InitData(const size_t guid, const J_OBJECT_FLAG flag, JGameObject* owner);
		};
	private:
		friend class JCameraPrivate;
		class JCameraImpl;
	private:
		std::unique_ptr<JCameraImpl> impl;
	public:
		Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final;
		JFrameUpdateUserAccess GetFrameUserInterface() noexcept final;
		const Graphic::JGraphicResourceUserInterface GraphicResourceUserInterface()const noexcept final;
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA;
		} 	  
		JTransform* GetTransform()noexcept;
		DirectX::XMMATRIX GetView()const noexcept;
		DirectX::XMMATRIX GetProj()const noexcept;
		DirectX::XMFLOAT4X4 GetView4x4f()const noexcept;
		DirectX::XMFLOAT4X4 GetProj4x4f()const noexcept;  
		DirectX::BoundingFrustum GetBoundingFrustum()const noexcept;
		float GetNear()const noexcept;
		float GetFar()const noexcept;
		float GetAspect()const noexcept;
		float GetFovY()const noexcept;
		float GetFovYDegree()const noexcept;
		float GetFovX()const noexcept;
		int GetViewWidth()const noexcept;
		int GetViewHeight()const noexcept; 
		float GetNearViewWidth()const noexcept;
		float GetNearViewHeight()const noexcept;
		float GetFarViewWidth()const noexcept;
		float GetFarViewHeight()const noexcept;
		J_CAMERA_STATE GetCameraState()const noexcept; 
	public:
		void SetNear(float value)noexcept;
		void SetFar(float value) noexcept; 
		void SetFov(float value) noexcept;
		void SetFovDegree(float value) noexcept;
		void SetViewSize(int width, int height) noexcept;
		void SetOrthoCamera(bool value)noexcept; 
		void SetMainCamera(bool value)noexcept;
	public:
		bool IsOrthoCamera()const noexcept;
		bool IsMainCamera()const noexcept;
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final; 
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final; 
	private:
		JCamera(const InitData& initData);
		~JCamera();
	};
}