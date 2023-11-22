#pragma once
#include"JLight.h"

namespace JinEngine
{
	class JSpotLightPrivate;
	class JSpotLight final : public JLight
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JSpotLight)
	public:
		class InitData final : public JLight::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(const JUserPtr<JGameObject>& owner);
			InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner);
		};
	private:
		friend class JSpotLightPrivate;
		class JSpotLightImpl;
	private:
		std::unique_ptr<JSpotLightImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		const Graphic::JGraphicResourceUserInterface GraphicResourceUserInterface()const noexcept final;
		const Graphic::JCullingUserInterface CullingUserInterface()const noexcept final;
		J_LIGHT_TYPE GetLightType()const noexcept final;
		J_SHADOW_MAP_TYPE GetShadowMapType()const noexcept final;
		float GetFrustumNear()const noexcept final;
		float GetFrustumFar()const noexcept final;
		float GetPower()const noexcept;
		float GetRange()const noexcept;
		float GetInnerConeAngle()const noexcept; 
		float GetOuterConeAngle()const noexcept;
		DirectX::BoundingBox GetBBox()const noexcept final;
		JVector3F GetDirection()const noexcept;
	public:
		void SetShadow(const bool value)noexcept final;
		void SetShadowResolution(const J_SHADOW_RESOLUTION sQuality)noexcept final;
		void SetAllowDisplayShadowMap(const bool value)noexcept final;
		void SetPower(const float power)noexcept;
		void SetRange(const float range)noexcept;
		void SetInnerConeAngle(const float spotAngle)noexcept; 
		void SetOuterConeAngle(const float spotAngle)noexcept;
	public:
		bool IsFrameDirted()const noexcept final;
		bool IsCsmActivated()const noexcept;
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
		JSpotLight(const InitData& initData);
		~JSpotLight();
	};
}