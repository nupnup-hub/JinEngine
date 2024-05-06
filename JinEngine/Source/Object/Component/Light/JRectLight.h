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
#include"JLight.h"
namespace JinEngine
{
	class JTexture;
	class JRectLight : public JLight
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JRectLight)
	public:
		class InitData final : public JLight::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(const JUserPtr<JGameObject>& owner);
			InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner);
		};
	private:
		friend class JRectLightPrivate;
		class JRectLightImpl;
	private:
		std::unique_ptr<JRectLightImpl> impl;
	public:
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		const Graphic::JGraphicResourceUserInterface GraphicResourceUserInterface()const noexcept final;
		const Graphic::JCullingUserInterface CullingUserInterface()const noexcept final;
		J_LIGHT_TYPE GetLightType()const noexcept final;
		J_SHADOW_MAP_TYPE GetShadowMapType()const noexcept final;
		JVector2F GetAreaSize()const noexcept;
		float GetMinPower()const noexcept final;
		float GetMaxPower()const noexcept final;
		float GetFrustumNear()const noexcept final;
		float GetFrustumFar()const noexcept final; 
		float GetRange()const noexcept;
		float GetBarndoorLength()const noexcept;
		float GetBarndoorAngle()const noexcept;
		DirectX::BoundingBox GetBBox()const noexcept final;
		JUserPtr<JMeshGeometry> GetMesh()const noexcept final;
		DirectX::XMMATRIX GetMeshWorldM(const bool restrictScaledZ = false)const noexcept final;
		DirectX::XMMATRIX GetMeshWorldMRectrictScaleZ()const noexcept;
		JVector3F GetDirection()const noexcept;
		JUserPtr<JTexture> GetSourceTexture()const noexcept;
	public:
		void SetShadow(const bool value)noexcept final;
		void SetShadowResolution(const J_SHADOW_RESOLUTION sQuality)noexcept final;
		void SetAllowDisplayShadowMap(const bool value)noexcept final;
		void SetAreaSize(const JVector2F newSize)noexcept;
		//void SetTwoSide(const bool value)noexcept; 
		void SetRange(const float range)noexcept;
		void SetBarndoorLength(const float newLength)noexcept;
		void SetBarndoorAngle(const float newAngle)noexcept;
		void SetSourceTexture(const JUserPtr<JTexture>& srcTexture)noexcept;
	public:
		bool IsFrameDirted()const noexcept final;
		bool IsCsmActivated()const noexcept;
		//bool IsTwoSide()const noexcept;
		bool CanAllocateCsm()const noexcept;
		bool PassDefectInspection()const noexcept final;
		bool AllowFrustumCulling()const noexcept final;
		bool AllowHzbOcclusionCulling()const noexcept final;
		bool AllowHdOcclusionCulling()const noexcept final;
		bool AllowDisplayOccCullingDepthMap()const noexcept final; 
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		JRectLight(const InitData& initData);
		~JRectLight();
	};
}