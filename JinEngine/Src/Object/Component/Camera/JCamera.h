#pragma once
#include"JCameraState.h"
#include"../JComponent.h" 
#include"../../../Graphic/Upload/Frameresource/JFrameUpdateUserAccess.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceUserAccess.h"
#include"../../../Graphic/Culling/JCullingUserAccess.h"
#include<DirectXCollision.h>

namespace JinEngine
{
	class JTransform; 
	class JCameraPrivate;
	class JCamera final : public JComponent,
		public Graphic::JFrameUpdateUserAccessInterface,
		public Graphic::JGraphicMultiResourceUserAccess,
		public Graphic::JCullingUserAccess
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JCamera)
	public: 
		class InitData final : public JComponent::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(const JUserPtr<JGameObject>& owner);
			InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner);
		};
	private:
		friend class JCameraPrivate;
		class JCameraImpl;
	private:
		std::unique_ptr<JCameraImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		Graphic::JFrameUpdateUserAccess FrameUserInterface() noexcept final;
		const Graphic::JGraphicMultiResourceUserInterface GraphicResourceUserInterface()const noexcept final;
		const Graphic::JCullingUserInterface CullingUserInterface()const noexcept final;
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA;
		} 	  
		JUserPtr<JTransform> GetTransform()noexcept;
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
		void SetNear(const float value)noexcept;
		void SetFar(const float value) noexcept;
		void SetFov(const float value) noexcept;
		void SetFovDegree(const float value) noexcept;
		void SetViewSize(const int width, const int height) noexcept;
		void SetOrthoCamera(const bool value)noexcept;
		void SetAllowDisplayDepthMap(const bool value)noexcept;
		void SetAllowDisplayDebug(const bool value)noexcept;
		void SetAllowFrustumCulling(const bool value)noexcept;
		void SetAllowHzbOcclusionCulling(const bool value)noexcept;
		void SetAllowDisplayOccCullingDepthMap(const bool value)noexcept;
		void SetCameraState(const J_CAMERA_STATE state)noexcept;
	public: 
		bool IsOrthoCamera()const noexcept;
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final; 
		bool AllowDisplayDepthMap()const noexcept;
		bool AllowDisplayDebug()const noexcept;
		bool AllowFrustumCulling()const noexcept;
		bool AllowHzbOcclusionCulling()const noexcept;
		bool AllowDisplayOccCullingDepthMap()const noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final; 
	private:
		JCamera(const InitData& initData);
		~JCamera();
	};
}