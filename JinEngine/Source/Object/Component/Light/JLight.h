#pragma once   
#include"../JComponent.h"
#include"../../../Graphic/Frameresource/JFrameUpdateUserAccess.h"
#include"../../../Graphic/Culling/JCullingUserAccess.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceUserAccess.h"
#include"JLightType.h" 
#include<DirectXCollision.h>

namespace JinEngine
{
	class JLight :public JComponent,
		public Graphic::JFrameUpdateUserAccess,
		public Graphic::JGraphicResourceUserAccess,
		public Graphic::JCullingUserAccess
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JLight)
	public: 
		class InitData : public JComponent::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(const Core::JTypeInfo& typeInfo, const JUserPtr<JGameObject>& owner);
			InitData(const Core::JTypeInfo& typeInfo, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner);
		}; 
	private:
		friend class JLightPrivate;
		class JLightImpl;
	private:
		std::unique_ptr<JLightImpl> impl;
	public: 
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT;
		} 
		JVector3<float> GetColor()const noexcept; 
		J_SHADOW_RESOLUTION GetShadowResolution()const noexcept;
		uint GetShadowMapSize()const noexcept;
		virtual J_LIGHT_TYPE GetLightType()const noexcept = 0;
		virtual J_SHADOW_MAP_TYPE GetShadowMapType()const noexcept = 0;
		virtual float GetNear()const noexcept = 0;
		virtual float GetFar()const noexcept = 0;
		virtual DirectX::BoundingBox GetBBox()const noexcept = 0;
	public: 
		void SetColor(const JVector3<float>& color)noexcept;  
		virtual void SetShadow(const bool value)noexcept;
		virtual void SetShadowResolution(const J_SHADOW_RESOLUTION sQuality)noexcept;
		virtual void SetAllowDisplayShadowMap(const bool value)noexcept;
	public: 
		bool IsShadowActivated()const noexcept;
		bool IsAvailableOverlap()const noexcept final; 
		bool AllowDisplayShadowMap()const noexcept; 
	protected:
		void DoActivate()noexcept override;
		void DoDeActivate()noexcept override;
	protected:
		JLight(const InitData& initData);
		~JLight();
	};
}