#pragma once 
#include"JLightInterface.h"
#include"JLightType.h"
#include<memory>
#include<DirectXMath.h>

namespace JinEngine
{
	struct JLightConstants;
	class JGameObject;

	class JLight final :public JLightInterface
	{
		REGISTER_CLASS(JLight)
	private:
		REGISTER_GUI_ENUM_CONDITION(LightType, J_LIGHT_TYPE, lightType)
		REGISTER_PROPERTY_EX(lightType, GetLightType, SetLightType, GUI_ENUM_COMBO(J_LIGHT_TYPE))
		J_LIGHT_TYPE lightType;
	private:
		REGISTER_PROPERTY_EX(strength, GetStrength, SetStrength, GUI_COLOR_PICKER(false))
		DirectX::XMFLOAT3 strength = { 0.8f, 0.8f, 0.8f };
		REGISTER_PROPERTY_EX(falloffStart, GetFalloffStart, SetFalloffStart, GUI_SLIDER(1, 100, true, false, GUI_ENUM_CONDITION_USER(LightType, J_LIGHT_TYPE::POINT, J_LIGHT_TYPE::SPOT)))
		float falloffStart = 1.0f;
		REGISTER_PROPERTY_EX(falloffEnd, GetFalloffEnd, SetFalloffEnd, GUI_SLIDER(1, 100, true, false, GUI_ENUM_CONDITION_USER(LightType, J_LIGHT_TYPE::POINT, J_LIGHT_TYPE::SPOT)))
		float falloffEnd = 10.0f;
		REGISTER_PROPERTY_EX(spotPower, GetSpotPower, SetSpotPower, GUI_SLIDER(0, 1, true, false, GUI_ENUM_CONDITION_USER(LightType, J_LIGHT_TYPE::POINT)))
		float spotPower = 0;
	private:
		REGISTER_PROPERTY_EX(onShadow, IsShadowActivated, SetShadow, GUI_CHECKBOX())
		bool onShadow = false;
		DirectX::XMFLOAT4X4 shadowTransform;
	public:
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

		void SetLightType(const J_LIGHT_TYPE lightType)noexcept;
		void SetStrength(const DirectX::XMFLOAT3& strength)noexcept;
		void SetFalloffStart(const float falloffStart)noexcept;
		void SetFalloffEnd(const float falloffEnd)noexcept;
		void SetSpotPower(const float spotPower)noexcept;
		void SetShadow(const bool value)noexcept;

		bool IsShadowActivated()const noexcept;
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final;
	private:
		void DoCopy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void CreateShadowMap()noexcept;
		void DestroyShadowMap()noexcept;
	private:
		DirectX::XMMATRIX CalLightView()const noexcept;
		DirectX::XMMATRIX CalLightProj()const noexcept;
		void UpdateShadowTransform()noexcept;
		void UpdateFrame(Graphic::JLightConstants& lightConstant) final;
		void UpdateFrame(Graphic::JShadowMapLightConstants& smLightConstant) final;
		void UpdateFrame(Graphic::JShadowMapConstants& shadowConstant) final;
	private:
		Core::J_FILE_IO_RESULT CallStoreComponent(std::wofstream& stream)final;
		static Core::J_FILE_IO_RESULT StoreObject(std::wofstream& stream, JLight* light);
		static JLight* LoadObject(std::wifstream& stream, JGameObject* owner);
		static void RegisterJFunc();
	private:
		JLight(const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
		~JLight();
	};
}