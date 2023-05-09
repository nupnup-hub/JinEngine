#pragma once  
#include"../JComponent.h"
#include"../../JFrameUpdateUserAccess.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceUserAccess.h"
#include"JLightType.h" 
#include<DirectXMath.h>

namespace JinEngine
{
	class JLight final :public JComponent,
		public JFrameUpdateUserAccessInterface,
		public Graphic::JGraphicResourceUserAccess
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JLight)
	public: 
		class InitData final : public JComponent::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(const JUserPtr<JGameObject>& owner);
			InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner);
		};
	private:
		friend class JLightPrivate;
		class JLightImpl;
	private:
		std::unique_ptr<JLightImpl> impl;
	public:
		Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final;
		JFrameUpdateUserAccess GetFrameUserInterface() noexcept final;
		const Graphic::JGraphicResourceUserInterface GraphicResourceUserInterface()const noexcept final;
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT;
		}
	public:
		J_LIGHT_TYPE GetLightType()const noexcept;
		DirectX::XMFLOAT3 GetStrength()const noexcept;
		DirectX::XMVECTOR GetLightDir()const noexcept;
		float GetFalloffStart()const noexcept;
		float GetFalloffEnd()const noexcept;
		float GetSpotPower()const noexcept;
	public:
		void SetLightType(const J_LIGHT_TYPE lightType)noexcept;
		void SetStrength(const DirectX::XMFLOAT3& strength)noexcept;
		void SetFalloffStart(const float falloffStart)noexcept;
		void SetFalloffEnd(const float falloffEnd)noexcept;
		void SetSpotPower(const float spotPower)noexcept;
		void SetShadow(const bool value)noexcept;
	public:
		bool IsShadowActivated()const noexcept;
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final; 
	private:
		JLight(const InitData& initData);
		~JLight();
	};
}