#pragma once
#include"../JComponent.h" 
#include"../../IFrameResourceControl.h" 
#include"JLightType.h"
#include<memory>
#include<DirectXMath.h>

namespace JinEngine
{ 
	struct JLightStruct; 
	class JGameObject;
	class GameObjectDirty;
	namespace Graphic
	{
		struct JLightConstants;
		struct JShadowMapCalConstants;
		struct JGraphicTextureHandle;
	}

	//수정필요 
	class JLight : public JComponent, public IFrameResourceControl
	{
		REGISTER_CLASS(JLight)
	private:
		std::unique_ptr<JLightStruct> light;
		J_LIGHT_TYPE lightType;
		GameObjectDirty* gameObjectDirty;
		Graphic::JGraphicTextureHandle* graphicTextureHandle;
		uint lightCBIndex = 0; 

		bool onShadow = false;
		int shadowCBIndex = -1; 
		DirectX::XMFLOAT4X4 shadowTransform;
	public:
		bool IsShadowActivated()const noexcept;
		DirectX::XMFLOAT3 GetStrength()const noexcept;
		float GetFalloffStart()const noexcept;
		float GetFalloffEnd()const noexcept;
		float GetSpotPower()const noexcept;
		uint GetLightCBIndex()const noexcept;
		uint GetShadowMapWidth()const noexcept;
		uint GetShadowMapHeight()const noexcept;
		uint GetShadowSrvHeapIndex()const noexcept;
		uint GetShadowDsvHeapIndex()const noexcept;
		uint GetShadowVectorIndex()const noexcept;
		int GetShadowCBIndex()const noexcept; 
		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static J_COMPONENT_TYPE GetStaticComponentType()noexcept;

		void SetStrength(const DirectX::XMFLOAT3& strength)noexcept;
		void SetFalloffStart(const float falloffStart)noexcept;
		void SetFalloffEnd(const float falloffEnd)noexcept;
		void SetSpotPower(const float spotPower)noexcept;
		void SetLightCBIndex(const uint index)noexcept;
		void SetShadow(const bool value)noexcept;
		void SetShadowCBIndex(const uint index)noexcept;
		void SetLightType(const J_LIGHT_TYPE lightType)noexcept;

		bool IsDirtied()const noexcept; 
		void SetDirty()noexcept;
		void OffDirty()noexcept;
		void MinusDirty()noexcept;
		 
		void StuffLightContents(Graphic::JLightConstants& constant)noexcept;
		void StuffShadowCalContents(Graphic::JShadowMapCalConstants& constant)noexcept;
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final; 
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;  
	private:
		void StuffDirectionalLight(Graphic::JLightConstants& constant)noexcept;
		void StuffPointLight(Graphic::JLightConstants& constant)noexcept;
		void StuffSpotLight(Graphic::JLightConstants& constant)noexcept;
		Core::J_FILE_IO_RESULT CallStoreComponent(std::wofstream& stream)final;
		static Core::J_FILE_IO_RESULT StoreObject(std::wofstream& stream, JLight* light);
		static JLight* LoadObject(std::wifstream& stream, JGameObject* owner);
		static void RegisterFunc();
	private:
		JLight(const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner);
		~JLight();
	};
}