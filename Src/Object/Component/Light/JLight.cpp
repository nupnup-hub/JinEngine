#include"JLight.h"
#include"JLightStruct.h" 
#include"../JComponentFactory.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../GameObject/JGameObject.h"
#include"../../GameObject/GameObjectDirty.h" 
#include"../../Component/Transform/JTransform.h"
#include"../../../Graphic/JGraphic.h"
#include"../../../Graphic/JGraphicTextureHandle.h" 
#include"../../../Graphic/FrameResource/JLightConstants.h" 
#include"../../../Graphic/FrameResource/JShadowMapCalConstants.h" 
#include<Windows.h>
#include<fstream>

namespace JinEngine
{
	using namespace DirectX;

	bool JLight::IsShadowActivated()const noexcept
	{
		return onShadow;
	}
	DirectX::XMFLOAT3 JLight::GetStrength()const noexcept
	{
		return light->strength;
	}
	float JLight::GetFalloffStart()const noexcept
	{
		return light->falloffStart;
	}
	float JLight::GetFalloffEnd()const noexcept
	{
		return light->falloffEnd;
	}
	float JLight::GetSpotPower()const noexcept
	{
		return light->spotPower;
	}
	uint JLight::GetLightCBIndex()const noexcept
	{
		return lightCBIndex;
	}
	uint JLight::GetShadowMapWidth()const noexcept
	{
		return graphicTextureHandle->GetWidth();
	}
	uint JLight::GetShadowMapHeight()const noexcept
	{
		return graphicTextureHandle->GetHeight();
	}
	uint JLight::GetShadowSrvHeapIndex()const noexcept
	{
		return graphicTextureHandle->GetSrvHeapIndex();
	}
	uint JLight::GetShadowDsvHeapIndex()const noexcept
	{
		return graphicTextureHandle->GetDsvHeapIndex();
	}
	uint JLight::GetShadowVectorIndex()const noexcept
	{
		return graphicTextureHandle->GetResourceVectorIndex();
	}
	int JLight::GetShadowCBIndex()const noexcept
	{
		return shadowCBIndex;
	}
	J_COMPONENT_TYPE JLight::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	J_COMPONENT_TYPE JLight::GetStaticComponentType()noexcept
	{
		return J_COMPONENT_TYPE::ENGINE_DEFIENED_LIGHT;
	}
	void JLight::SetStrength(const DirectX::XMFLOAT3& strength)noexcept
	{
		light->strength = strength;
		gameObjectDirty->SetLightDirty();
	}
	void JLight::SetFalloffStart(const float falloffStart)noexcept
	{
		light->falloffStart = falloffStart;
		gameObjectDirty->SetLightDirty();
	}
	void JLight::SetFalloffEnd(const float falloffEnd)noexcept
	{
		light->falloffEnd = falloffEnd;
		gameObjectDirty->SetLightDirty();
	}
	void JLight::SetSpotPower(const float spotPower)noexcept
	{
		light->spotPower = spotPower;
		gameObjectDirty->SetLightDirty();
	}
	void JLight::SetLightCBIndex(const uint index)noexcept
	{
		lightCBIndex = index;
		SetDirty();
	}
	void JLight::SetShadow(const bool value)noexcept
	{
		if (onShadow && !value)
		{
			if (JGraphic::Instance().ResourceInterface()->EraseGraphicTextureResource(graphicTextureHandle))
			{
				GetOwnerInterface()->DeRegisterShadowLight(this);
				onShadow = value;
			}
			graphicTextureHandle = nullptr;
			SetDirty();
		}
		else if (!onShadow && value)
		{
			onShadow = value;
			graphicTextureHandle = JGraphic::Instance().ResourceInterface()->CreateShadowMapTexture();
			if (graphicTextureHandle != nullptr)
				GetOwnerInterface()->RegisterShadowLight(this);
			SetDirty();
		}
	}
	void JLight::SetShadowCBIndex(const uint index)noexcept
	{
		shadowCBIndex = index;
		SetDirty();
	}
	void JLight::SetLightType(const J_LIGHT_TYPE lightType)noexcept
	{
		JLight::lightType = lightType;
	}
	bool JLight::IsDirtied()const noexcept
	{
		return gameObjectDirty->GetLightDirty() > 0;
	}
	void JLight::SetDirty()noexcept
	{
		gameObjectDirty->SetLightDirty();
	}
	void JLight::OffDirty()noexcept
	{
		gameObjectDirty->OffLightDirty();
	}
	void JLight::MinusDirty()noexcept
	{
		gameObjectDirty->LightUpdate();
	}
	void JLight::StuffLightContents(Graphic::JLightConstants& constant)noexcept
	{
		switch (lightType)
		{
		case JinEngine::J_LIGHT_TYPE::DIRECTIONAL:
			StuffDirectionalLight(constant);
			break;
		case JinEngine::J_LIGHT_TYPE::POINT:
			StuffPointLight(constant);
			break;
		case JinEngine::J_LIGHT_TYPE::SPOT:
			StuffSpotLight(constant);
			break;
		default:
			break;
		}
	}
	void JLight::StuffShadowCalContents(Graphic::JShadowMapCalConstants& constant)noexcept
	{
		const XMFLOAT3 rotf = GetOwner()->GetTransform()->GetRotation();
		const XMVECTOR rotv = XMLoadFloat3(&rotf);
		const XMVECTOR qV = XMQuaternionRotationRollPitchYawFromVector(XMVectorScale(rotv, JMathHelper::DegToRad)); 
		const XMVECTOR initDir = XMVectorSet(0, -1, 0, 1);
		const XMVECTOR lightDirV = XMVector3Normalize(XMVector3Rotate(initDir, qV));

		const XMVECTOR targetPosV = XMVectorSet(0, 0, 0, 1);
		const XMVECTOR lightPosV = XMVectorScale(lightDirV, -50);

		const XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		const XMMATRIX lightView = XMMatrixLookAtLH(lightPosV, targetPosV, lightUp);

		XMFLOAT3 sphereCenterLS;
		XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPosV, lightView));

		float l = sphereCenterLS.x - 25;
		float b = sphereCenterLS.y - 25;
		float n = sphereCenterLS.z - 25;
		float r = sphereCenterLS.x + 25;
		float t = sphereCenterLS.y + 25;
		float f = sphereCenterLS.z + 25;

		const XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
		//const XMMATRIX lightProj_P = XMMatrixPerspectiveOffCenterLH(l, r, b, t, n, f);
		// JTransform NDC space [-1,+1]^2 to texture space [0,1]^2
		const XMMATRIX T(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);

		const XMMATRIX S = lightView * lightProj * T;
		XMStoreFloat4x4(&shadowTransform, S);
		//constant.s_directionalLight.shadow. 

		const XMMATRIX viewProj = XMMatrixMultiply(lightView, lightProj);

		XMVECTOR viewVec = XMMatrixDeterminant(lightView);
		XMVECTOR projVec = XMMatrixDeterminant(lightProj);
		XMVECTOR viewProjVec = XMMatrixDeterminant(viewProj);

		const XMMATRIX invView = XMMatrixInverse(&viewVec, lightView);
		const XMMATRIX invProj = XMMatrixInverse(&projVec, lightProj);
		const XMMATRIX invViewProj = XMMatrixInverse(&viewProjVec, viewProj);

		XMStoreFloat4x4(&constant.view, XMMatrixTranspose(lightView));
		XMStoreFloat4x4(&constant.invView, XMMatrixTranspose(invView));
		XMStoreFloat4x4(&constant.proj, XMMatrixTranspose(lightProj));
		XMStoreFloat4x4(&constant.invViewProj, XMMatrixTranspose(invProj));
		XMStoreFloat4x4(&constant.viewProj, XMMatrixTranspose(viewProj));
		XMStoreFloat4x4(&constant.invProj, XMMatrixTranspose(invViewProj));
		XMStoreFloat3(&constant.eyePosW, lightPosV);

		constant.renderTargetSize = XMFLOAT2((float)GetShadowMapWidth(), (float)GetShadowMapHeight());
		constant.invRenderTargetSize = XMFLOAT2(1.0f / (float)GetShadowMapWidth(), 1.0f / (float)GetShadowMapHeight());
		constant.nearZ = n;
		constant.farZ = f;
	}
	bool JLight::IsAvailableOverlap()const noexcept
	{
		return true;
	}
	bool JLight::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection())
			return true;
		else
			return false;
	}
	void JLight::DoActivate()noexcept
	{
		JComponent::DoActivate();
		RegisterComponent<JLight>(this);
		SetDirty();
	}
	void JLight::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		JGraphic::Instance().ResourceInterface()->EraseGraphicTextureResource(graphicTextureHandle);
		DeRegisterComponent<JLight>(this);
		OffDirty();
	}
	void JLight::StuffDirectionalLight(Graphic::JLightConstants& constant)noexcept
	{
		const XMFLOAT3 rotf = GetOwner()->GetTransform()->GetRotation();
		const XMVECTOR rotv = XMLoadFloat3(&rotf);
		const XMVECTOR qV = XMQuaternionRotationRollPitchYawFromVector(XMVectorScale(rotv, JMathHelper::DegToRad));
		const XMVECTOR initDir = XMVectorSet(0, -1, 0, 1);
		const XMVECTOR dir = XMVector3Normalize(XMVector3Rotate(initDir, qV));
		if (IsShadowActivated())
		{
			XMStoreFloat3(&constant.s_directionalLight[constant.s_directionalLightMax].dLight.direction, dir);
			constant.s_directionalLight[constant.s_directionalLightMax].dLight.strength = light->strength;
			constant.s_directionalLight[constant.s_directionalLightMax].shadow.shadowMapIndex = GetShadowVectorIndex();
			XMStoreFloat4x4(&constant.s_directionalLight[constant.s_directionalLightMax].shadow.shadowTransform, 
				XMMatrixTranspose(XMLoadFloat4x4(&shadowTransform)));
			++constant.s_directionalLightMax;
		}
		else
		{
			XMStoreFloat3(&constant.directionalLight[constant.directionalLightMax].direction, dir);
			constant.directionalLight[constant.directionalLightMax].strength = light->strength;
			++constant.directionalLightMax;
		}
	}
	void JLight::StuffPointLight(Graphic::JLightConstants& constant)noexcept
	{
		if (IsShadowActivated())
		{
			constant.s_pointLight[constant.s_pointLightMax].pLight.strength = light->strength;
			constant.s_pointLight[constant.s_pointLightMax].pLight.falloffStart = light->falloffStart;
			constant.s_pointLight[constant.s_pointLightMax].pLight.position = GetOwner()->GetTransform()->GetPosition();
			constant.s_pointLight[constant.s_pointLightMax].pLight.falloffEnd = light->falloffEnd;
			constant.s_pointLight[constant.s_pointLightMax].shadow.shadowMapIndex = GetShadowVectorIndex();
			XMStoreFloat4x4(&constant.s_pointLight[constant.s_pointLightMax].shadow.shadowTransform,
				XMMatrixTranspose(XMLoadFloat4x4(&shadowTransform))); 
			++constant.s_pointLightMax;
		}
		else
		{
			constant.pointLight[constant.pointLightMax].strength = light->strength;
			constant.pointLight[constant.pointLightMax].falloffStart = light->falloffStart;
			constant.pointLight[constant.pointLightMax].position = GetOwner()->GetTransform()->GetPosition();
			constant.pointLight[constant.pointLightMax].falloffEnd = light->falloffEnd;
			++constant.pointLightMax;
		}
	}
	void JLight::StuffSpotLight(Graphic::JLightConstants& constant)noexcept
	{
		const XMFLOAT3 rotf = GetOwner()->GetTransform()->GetRotation();
		const XMVECTOR rotv = XMLoadFloat3(&rotf);
		const XMVECTOR qV = XMQuaternionRotationRollPitchYawFromVector(XMVectorScale(rotv, JMathHelper::DegToRad));
		const XMVECTOR initDir = XMVectorSet(0, -1, 0, 1);
		const XMVECTOR dir = XMVector3Normalize(XMVector3Rotate(initDir, qV));

		if (IsShadowActivated())
		{
			constant.s_spotLight[constant.s_spotLightMax].sLight.strength = light->strength;
			constant.s_spotLight[constant.s_spotLightMax].sLight.falloffStart = light->falloffStart;
			XMStoreFloat3(&constant.s_spotLight[constant.s_spotLightMax].sLight.direction, dir);
			constant.s_spotLight[constant.s_spotLightMax].sLight.falloffEnd = light->falloffEnd;
			constant.s_spotLight[constant.s_spotLightMax].sLight.position = GetOwner()->GetTransform()->GetPosition();
			constant.s_spotLight[constant.s_spotLightMax].shadow.shadowMapIndex = GetShadowVectorIndex();
			XMStoreFloat4x4(&constant.s_spotLight[constant.s_spotLightMax].shadow.shadowTransform,
				XMMatrixTranspose(XMLoadFloat4x4(&shadowTransform)));
			++constant.s_spotLightMax;
		}
		else
		{
			constant.spotLight[constant.spotLightMax].strength = light->strength;
			constant.spotLight[constant.spotLightMax].falloffStart = light->falloffStart;
			XMStoreFloat3(&constant.spotLight[constant.spotLightMax].direction, dir);
			constant.spotLight[constant.spotLightMax].falloffEnd = light->falloffEnd;
			constant.spotLight[constant.spotLightMax].position = GetOwner()->GetTransform()->GetPosition();
			++constant.spotLightMax;
		}
	}
	Core::J_FILE_IO_RESULT JLight::CallStoreComponent(std::wofstream& stream)
	{
		return StoreObject(stream, this);
	}
	Core::J_FILE_IO_RESULT JLight::StoreObject(std::wofstream& stream, JLight* light)
	{
		if (light == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (((int)light->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		Core::J_FILE_IO_RESULT res = StoreMetadata(stream, light);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		stream << light->light->strength.x << " " << light->light->strength.y << " " << light->light->strength.z << '\n' 
			<< light->light->falloffStart << " " << light->light->falloffEnd << '\n'
			<< light->light->spotPower << '\n';
		 
		return  Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JLight* JLight::LoadObject(std::wifstream& stream, JGameObject* owner)
	{
		if (owner == nullptr)
			return nullptr;

		if (!stream.is_open())
			return nullptr;
		 
		ObjectMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);

		JLight* newLightComponent;
		if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
			newLightComponent = new JLight(metadata.guid, metadata.flag, owner);
		else
			newLightComponent = new JLight(Core::MakeGuid(), OBJECT_FLAG_NONE, owner);
 
		stream >> newLightComponent->light->strength.x >> newLightComponent->light->strength.y >> newLightComponent->light->strength.z >>
			newLightComponent->light->falloffStart >> newLightComponent->light->falloffEnd >> newLightComponent->light->spotPower;

		newLightComponent->gameObjectDirty->SetLightDirty(); 
		return newLightComponent;
	}
	void JLight::RegisterFunc()
	{
		auto defaultC = [](JGameObject* owner) -> JComponent*
		{
			return new JLight(Core::MakeGuid(), OBJECT_FLAG_NONE, owner);
		};
		auto initC = [](const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner)-> JComponent*
		{
			return new JLight(guid, objFlag, owner);
		};
		auto loadC = [](std::wifstream& stream, JGameObject* owner) -> JComponent*
		{
			return LoadObject(stream, owner);
		};
		auto copyC = [](JComponent* ori, JGameObject* owner) -> JComponent*
		{
			JLight* oriLit = static_cast<JLight*>(ori);
			JLight* newLit = new JLight(Core::MakeGuid(), oriLit->GetFlag(), owner);

			newLit->light->strength = oriLit->light->strength;
			newLit->light->falloffStart = oriLit->light->falloffStart;
			newLit->light->direction = oriLit->light->direction;
			newLit->light->falloffEnd = oriLit->light->falloffEnd;
			newLit->light->position = oriLit->light->position;
			newLit->light->spotPower = oriLit->light->spotPower;

			newLit->SetLightType(oriLit->lightType);
			newLit->SetShadow(oriLit->onShadow);

			return newLit;
		};
		JCFI<JLight>::Regist(defaultC, initC, loadC, copyC);
	}
	JLight::JLight(const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner)
		:JComponent(TypeName(), guid, objFlag, owner)
	{
		gameObjectDirty = owner->GetGameObjectDirty();
		lightType = J_LIGHT_TYPE::DIRECTIONAL;
		light = std::make_unique<JLightStruct>();
	}
	JLight::~JLight() {}
}