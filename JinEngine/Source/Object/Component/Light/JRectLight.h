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
		float GetFrustumNear()const noexcept final;
		float GetFrustumFar()const noexcept final;
		float GetPower()const noexcept;
		float GetRange()const noexcept;
		DirectX::BoundingBox GetBBox()const noexcept final;
		JVector3F GetDirection()const noexcept;
		JUserPtr<JTexture> GetSourceTexture()const noexcept;
	public:
		void SetShadow(const bool value)noexcept final;
		void SetShadowResolution(const J_SHADOW_RESOLUTION sQuality)noexcept final;
		void SetAllowDisplayShadowMap(const bool value)noexcept final;
		void SetAreaSize(const JVector2F newSize)noexcept;
		void SetTwoSide(const bool value)noexcept;
		void SetPower(const float power)noexcept;
		void SetRange(const float range)noexcept;
		void SetSourceTexture(const JUserPtr<JTexture>& srcTexture)noexcept;
	public:
		bool IsFrameDirted()const noexcept final;
		bool IsCsmActivated()const noexcept;
		bool IsTwoSide()const noexcept;
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