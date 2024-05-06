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
#include"JCameraState.h"
#include"../JComponent.h" 
#include"../../../Graphic/Frameresource/JFrameUpdateUserAccess.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceUserAccess.h"
#include"../../../Graphic/Culling/JCullingUserAccess.h" 
#include"../../../Graphic/Image/JImageProcessingDesc.h"
#include"../../../Core/Math/JMatrix.h"
#include<DirectXCollision.h>

namespace JinEngine
{
	class JTransform; 
	class JCameraPrivate;
	 
	class JCamera final : public JComponent,
		public Graphic::JFrameUpdateUserAccess,
		public Graphic::JGraphicResourceUserAccess,
		public Graphic::JCullingUserAccess
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JCamera)
	public: 
		class InitData final : public JComponent::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			JVector2F rtSizeRate = JVector2F::One();
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
		const Graphic::JGraphicResourceUserInterface GraphicResourceUserInterface()const noexcept final;
		const Graphic::JCullingUserInterface CullingUserInterface()const noexcept final;
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_CAMERA;
		} 	  
		JUserPtr<JTransform> GetTransform()noexcept;
		DirectX::XMMATRIX GetView()const noexcept;
		JMatrix4x4 GetView4x4()const noexcept;
		DirectX::XMMATRIX GetInvView()const noexcept; 
		DirectX::XMMATRIX GetProj()const noexcept;  
		JMatrix4x4 GetProj4x4()const noexcept; 
		DirectX::XMMATRIX GetPreViewProj()const noexcept;
		void GetUvToView(JVector2F& a, JVector2F& b)const noexcept;
		/*
		* @return world bounding frustum
		*/
		DirectX::BoundingFrustum GetBoundingFrustum()const noexcept; 
		/*
		* @return local bounding frustum
		*/
		DirectX::BoundingFrustum GetLocalBoundingFrustum()const noexcept;
		float GetNear()const noexcept;
		float GetFar()const noexcept;
		float GetFovX()const noexcept;
		float GetFovXDegree()const noexcept;
		float GetFovY()const noexcept;
		float GetFovYDegree()const noexcept;
		float GetAspect()const noexcept;
		float GetOrthoViewWidth()const noexcept;
		float GetOrthoViewHeight()const noexcept;
		float GetNearViewWidth()const noexcept;
		float GetNearViewHeight()const noexcept;
		float GetFarViewWidth()const noexcept;
		float GetFarViewHeight()const noexcept;
		J_CAMERA_STATE GetCameraState()const noexcept; 
		JVector2F GetRenderTargetSize()const noexcept;
		JVector2F GetRenderTargetRate()const noexcept;
		Graphic::JSsaoDesc GetSsaoDesc()const noexcept; 
	public:
		void SetNear(const float value)noexcept;
		void SetFar(const float value) noexcept;
		void SetFov(const float value) noexcept;
		void SetFovDegree(const float value) noexcept;
		void SetAspect(const float value) noexcept;
		void SetOrthoViewWidth(const float value) noexcept;		// for Ortho
		void SetOrthoViewHeight(const float value) noexcept;	// for Ortho
		void SetOrthoViewSize(const float width, const float height) noexcept;	// for Ortho
		void SetOrthoCamera(const bool value)noexcept;
		void SetAllowDisplayRenderResult(const bool value)noexcept;
		void SetAllowDisplayDebugObject(const bool value)noexcept;
		void SetAllowFrustumCulling(const bool value)noexcept;
		void SetAllowHzbOcclusionCulling(const bool value)noexcept;
		void SetAllowHdOcclusionCulling(const bool value)noexcept;
		void SetAllowDisplayOccCullingDepthMap(const bool value)noexcept;
		void SetAllowSsao(const bool value)noexcept;
		void SetCameraState(const J_CAMERA_STATE state)noexcept;
		void SetRenderTargetRate(const JVector2F rate)noexcept;		//default 1,1 = client window size
		void SetSsaoDesc(const Graphic::JSsaoDesc& desc)noexcept;
	public: 
		bool IsFrameDirted()const noexcept;
		bool IsOrthoCamera()const noexcept; 
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final; 
		bool AllowDisplayRenderResult()const noexcept;
		bool AllowDisplayDebugObject()const noexcept;
		bool AllowFrustumCulling()const noexcept final;
		bool AllowHzbOcclusionCulling()const noexcept final;
		bool AllowHdOcclusionCulling()const noexcept final;
		bool AllowDisplayOccCullingDepthMap()const noexcept final;
		bool AllowLightCulling()const noexcept final;
		bool AllowDisplayLightCullingDebug()const noexcept;
		bool AllowSsao()const noexcept;
		bool AllowPostProcess()const noexcept;
		bool AllowRaytracingGI()const noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final; 
	private:
		JCamera(const InitData& initData);
		~JCamera();
	};
}