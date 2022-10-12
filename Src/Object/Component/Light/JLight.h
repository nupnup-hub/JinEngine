#pragma once 
#include"JLightInterface.h"
#include"JLightType.h"
#include<memory>
#include<DirectXMath.h>

namespace JinEngine
{ 
	struct JLightStruct; 
	class JGameObject; 

	class JLight final :public JLightInterface
	{
		REGISTER_CLASS(JLight)
	private:
		std::unique_ptr<JLightStruct> lightData;
		J_LIGHT_TYPE lightType;  

		bool onShadow = false; 
		DirectX::XMFLOAT4X4 shadowTransform;
	public:
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static constexpr J_COMPONENT_TYPE GetStaticComponentType()noexcept
		{
			return J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT;
		}
	public:
		DirectX::XMFLOAT3 GetStrength()const noexcept;
		float GetFalloffStart()const noexcept;
		float GetFalloffEnd()const noexcept;
		float GetSpotPower()const noexcept; 

		void SetStrength(const DirectX::XMFLOAT3& strength)noexcept;
		void SetFalloffStart(const float falloffStart)noexcept;
		void SetFalloffEnd(const float falloffEnd)noexcept;
		void SetSpotPower(const float spotPower)noexcept; 
		void SetShadow(const bool value)noexcept; 
		void SetLightType(const J_LIGHT_TYPE lightType)noexcept;
		 
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
		void StuffDirectionalLight(Graphic::JLightConstants& constant)noexcept;
		void StuffPointLight(Graphic::JLightConstants& constant)noexcept;
		void StuffSpotLight(Graphic::JLightConstants& constant)noexcept;
	private:
		bool UpdateFrame(Graphic::JLightConstants& lightConstant, Graphic::JShadowMapConstants& shadowConstant) final;
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