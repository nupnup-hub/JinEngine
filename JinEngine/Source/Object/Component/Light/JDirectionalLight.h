#pragma once
#include"JLight.h"

namespace JinEngine
{
	class JDirectionalLightPrivate;
	class JDirectionalLight final : public JLight
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JDirectionalLight)
	public:
		class InitData final : public JLight::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(const JUserPtr<JGameObject>& owner);
			InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner);
		};
	private:
		friend class JDirectionalLightPrivate;
		class JDirectionalLightImpl;
	private:
		std::unique_ptr<JDirectionalLightImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		const Graphic::JGraphicResourceUserInterface GraphicResourceUserInterface()const noexcept final;
		const Graphic::JCullingUserInterface CullingUserInterface()const noexcept final;
		J_LIGHT_TYPE GetLightType()const noexcept final;
		J_SHADOW_MAP_TYPE GetShadowMapType()const noexcept final; 
		float GetMinPower()const noexcept;
		float GetMaxPower()const noexcept;
		float GetFrustumNear()const noexcept final;
		float GetFrustumFar()const noexcept final;
		DirectX::BoundingBox GetBBox()const noexcept final;
		JUserPtr<JMeshGeometry> GetMesh()const noexcept final;
		DirectX::XMMATRIX GetMeshWorldM(const bool restrictScaledZ = false)const noexcept final;
		uint GetCsmSplitCount()const noexcept;
		float GetCsmSplitBlendRate()const noexcept;
		float GetCsmShadowDistance()const noexcept;
		float GetCsmLevelBlendRate()const noexcept;
	public: 
		void SetShadow(const bool value)noexcept final;
		void SetShadowResolution(const J_SHADOW_RESOLUTION sQuality)noexcept final;
		void SetAllowDisplayShadowMap(const bool value)noexcept final;
		void SetCsm(const bool value)noexcept;
		void SetCsmSplitCount(const uint newCount)noexcept;
		void SetCsmSplitBlendRate(const float value)noexcept;
		void SetCsmShadowDistance(const float value)noexcept;
		void SetCsmLevelBlendRate(const float value)noexcept;
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
		JDirectionalLight(const InitData& initData);
		~JDirectionalLight();
	};
}