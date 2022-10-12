#include"JLight.h"
#include"JLightStruct.h" 
#include"../JComponentFactory.h"
#include"../../Component/Transform/JTransform.h"
#include"../../GameObject/JGameObject.h" 
#include"../../Resource/Scene/JScene.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Guid/GuidCreator.h" 
#include"../../../Core/File/JFileConstant.h"
#include"../../../Graphic/FrameResource/JLightConstants.h" 
#include"../../../Graphic/FrameResource/JShadowMapConstants.h" 
#include<Windows.h>
#include<fstream>

namespace JinEngine
{
	using namespace DirectX;
	static auto isAvailableoverlapLam = []() {return true; };
	J_COMPONENT_TYPE JLight::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	DirectX::XMFLOAT3 JLight::GetStrength()const noexcept
	{
		return lightData->strength;
	}
	float JLight::GetFalloffStart()const noexcept
	{
		return lightData->falloffStart;
	}
	float JLight::GetFalloffEnd()const noexcept
	{
		return lightData->falloffEnd;
	}
	float JLight::GetSpotPower()const noexcept
	{
		return lightData->spotPower;
	}
	void JLight::SetStrength(const DirectX::XMFLOAT3& strength)noexcept
	{
		lightData->strength = strength;
		SetFrameDirty();
	}
	void JLight::SetFalloffStart(const float falloffStart)noexcept
	{
		lightData->falloffStart = falloffStart;
		SetFrameDirty();
	}
	void JLight::SetFalloffEnd(const float falloffEnd)noexcept
	{
		lightData->falloffEnd = falloffEnd;
		SetFrameDirty();
	}
	void JLight::SetSpotPower(const float spotPower)noexcept
	{
		lightData->spotPower = spotPower;
		SetFrameDirty();
	}
	void JLight::SetShadow(const bool value)noexcept
	{
		static auto IsShadow = [](JComponent& jcomp)
		{
			return static_cast<JLight*>(&jcomp)->onShadow;
		};

		if (value)
		{
			if (IsActivated() && !onShadow)
			{
				CreateShadowMap();
				GetOwner()->GetOwnerScene()->AppInterface()->SetBackSideComponentDirty(*this, IsShadow);
				SetFrameDirty();
				onShadow = value;
			}
		}
		else
		{
			if (onShadow)
			{
				DestroyShadowMap();
				GetOwner()->GetOwnerScene()->AppInterface()->SetBackSideComponentDirty(*this, IsShadow);
				SetFrameDirty();
				onShadow = value;
			}
		}
	}
	void JLight::SetLightType(const J_LIGHT_TYPE lightType)noexcept
	{
		JLight::lightType = lightType;
	}
	bool JLight::IsShadowActivated()const noexcept
	{
		return onShadow;
	}
	bool JLight::IsAvailableOverlap()const noexcept
	{
		return isAvailableoverlapLam();
	}
	bool JLight::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection())
			return true;
		else
			return false;
	}
	void JLight::DoCopy(JObject* ori)
	{
		JLight* oriLit = static_cast<JLight*>(ori);
		lightData->strength = oriLit->lightData->strength;
		lightData->falloffStart = oriLit->lightData->falloffStart;
		lightData->direction = oriLit->lightData->direction;
		lightData->falloffEnd = oriLit->lightData->falloffEnd;
		lightData->position = oriLit->lightData->position;
		lightData->spotPower = oriLit->lightData->spotPower;

		SetLightType(oriLit->lightType);
		SetShadow(oriLit->onShadow);
		SetFrameDirty();
	}
	void JLight::DoActivate()noexcept
	{
		JComponent::DoActivate();
		RegisterComponent();
		if (onShadow)
			CreateShadowMap();
		SetFrameDirty();
	}
	void JLight::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		DeRegisterComponent();
		if (onShadow)
			DestroyShadowMap();
		OffFrameDirty();
	}
	void JLight::CreateShadowMap()noexcept
	{
		CreateShadowMapTexture();
		AddDrawRequest(GetOwner()->GetOwnerScene(), this);
	}
	void JLight::DestroyShadowMap()noexcept
	{
		PopDrawRequest(GetOwner()->GetOwnerScene(), this);
		DestroyTxtHandle();
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
			constant.s_directionalLight[constant.s_directionalLightMax].dLight.strength = lightData->strength;
			constant.s_directionalLight[constant.s_directionalLightMax].shadow.shadowMapIndex = GetTxtVectorIndex();
			XMStoreFloat4x4(&constant.s_directionalLight[constant.s_directionalLightMax].shadow.shadowTransform,
				XMMatrixTranspose(XMLoadFloat4x4(&shadowTransform)));
			++constant.s_directionalLightMax;
		}
		else
		{
			XMStoreFloat3(&constant.directionalLight[constant.directionalLightMax].direction, dir);
			constant.directionalLight[constant.directionalLightMax].strength = lightData->strength;
			++constant.directionalLightMax;
		}
	}
	void JLight::StuffPointLight(Graphic::JLightConstants& constant)noexcept
	{
		if (IsShadowActivated())
		{
			constant.s_pointLight[constant.s_pointLightMax].pLight.strength = lightData->strength;
			constant.s_pointLight[constant.s_pointLightMax].pLight.falloffStart = lightData->falloffStart;
			constant.s_pointLight[constant.s_pointLightMax].pLight.position = GetOwner()->GetTransform()->GetPosition();
			constant.s_pointLight[constant.s_pointLightMax].pLight.falloffEnd = lightData->falloffEnd;
			constant.s_pointLight[constant.s_pointLightMax].shadow.shadowMapIndex = GetTxtVectorIndex();
			XMStoreFloat4x4(&constant.s_pointLight[constant.s_pointLightMax].shadow.shadowTransform,
				XMMatrixTranspose(XMLoadFloat4x4(&shadowTransform)));
			++constant.s_pointLightMax;
		}
		else
		{
			constant.pointLight[constant.pointLightMax].strength = lightData->strength;
			constant.pointLight[constant.pointLightMax].falloffStart = lightData->falloffStart;
			constant.pointLight[constant.pointLightMax].position = GetOwner()->GetTransform()->GetPosition();
			constant.pointLight[constant.pointLightMax].falloffEnd = lightData->falloffEnd;
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
			constant.s_spotLight[constant.s_spotLightMax].sLight.strength = lightData->strength;
			constant.s_spotLight[constant.s_spotLightMax].sLight.falloffStart = lightData->falloffStart;
			XMStoreFloat3(&constant.s_spotLight[constant.s_spotLightMax].sLight.direction, dir);
			constant.s_spotLight[constant.s_spotLightMax].sLight.falloffEnd = lightData->falloffEnd;
			constant.s_spotLight[constant.s_spotLightMax].sLight.position = GetOwner()->GetTransform()->GetPosition();
			constant.s_spotLight[constant.s_spotLightMax].shadow.shadowMapIndex = GetTxtVectorIndex();
			XMStoreFloat4x4(&constant.s_spotLight[constant.s_spotLightMax].shadow.shadowTransform,
				XMMatrixTranspose(XMLoadFloat4x4(&shadowTransform)));
			++constant.s_spotLightMax;
		}
		else
		{
			constant.spotLight[constant.spotLightMax].strength = lightData->strength;
			constant.spotLight[constant.spotLightMax].falloffStart = lightData->falloffStart;
			XMStoreFloat3(&constant.spotLight[constant.spotLightMax].direction, dir);
			constant.spotLight[constant.spotLightMax].falloffEnd = lightData->falloffEnd;
			constant.spotLight[constant.spotLightMax].position = GetOwner()->GetTransform()->GetPosition();
			++constant.spotLightMax;
		}
	}
	bool JLight::UpdateFrame(Graphic::JLightConstants& lightConstant, Graphic::JShadowMapConstants& shadowConstant)
	{
		if (IsFrameDirted())
		{
			switch (lightType)
			{
			case JinEngine::J_LIGHT_TYPE::DIRECTIONAL:
				StuffDirectionalLight(lightConstant);
				break;
			case JinEngine::J_LIGHT_TYPE::POINT:
				StuffPointLight(lightConstant);
				break;
			case JinEngine::J_LIGHT_TYPE::SPOT:
				StuffSpotLight(lightConstant);
				break;
			default:
				break;
			}

			if (onShadow)
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

				XMStoreFloat4x4(&shadowConstant.view, XMMatrixTranspose(lightView));
				XMStoreFloat4x4(&shadowConstant.invView, XMMatrixTranspose(invView));
				XMStoreFloat4x4(&shadowConstant.proj, XMMatrixTranspose(lightProj));
				XMStoreFloat4x4(&shadowConstant.invViewProj, XMMatrixTranspose(invProj));
				XMStoreFloat4x4(&shadowConstant.viewProj, XMMatrixTranspose(viewProj));
				XMStoreFloat4x4(&shadowConstant.invProj, XMMatrixTranspose(invViewProj));
				XMStoreFloat3(&shadowConstant.eyePosW, lightPosV);

				shadowConstant.renderTargetSize = XMFLOAT2((float)GetTxtWidth(), (float)GetTxtHeight());
				shadowConstant.invRenderTargetSize = XMFLOAT2(1.0f / (float)GetTxtWidth(), 1.0f / (float)GetTxtHeight());
				shadowConstant.nearZ = n;
				shadowConstant.farZ = f;
			}
			return true;
		}
		else
			return false;
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

		JFileIOHelper::StoreObjectIden(stream, light);
		JFileIOHelper::StoreXMFloat3(stream, L"Strength:", light->lightData->strength);
		JFileIOHelper::StoreAtomicData(stream, L"FallOffsetStart:", light->lightData->falloffStart);
		JFileIOHelper::StoreAtomicData(stream, L"FallOffsetEnd:", light->lightData->falloffEnd);
		JFileIOHelper::StoreAtomicData(stream, L"SpotPower:", light->lightData->spotPower);

		return  Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JLight* JLight::LoadObject(std::wifstream& stream, JGameObject* owner)
	{
		if (owner == nullptr)
			return nullptr;

		if (!stream.is_open())
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;

		XMFLOAT3 strength;
		float fallOffsetStart;
		float fallOffsetEnd;
		float spotPower;

		JFileIOHelper::LoadObjectIden(stream, guid, flag);
		JFileIOHelper::LoadXMFloat3(stream, strength);
		JFileIOHelper::LoadAtomicData(stream, fallOffsetStart);
		JFileIOHelper::LoadAtomicData(stream, fallOffsetEnd);
		JFileIOHelper::LoadAtomicData(stream, spotPower);

		Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JLight>(guid, flag, owner);
		JLight* newLightComponent = ownerPtr.Get();

		if (!AddInstance(std::move(ownerPtr)))
			return nullptr;

		newLightComponent->SetStrength(strength);
		newLightComponent->SetFalloffStart(fallOffsetStart);
		newLightComponent->SetFalloffEnd(fallOffsetEnd);
		newLightComponent->SetSpotPower(spotPower);

		return newLightComponent;
	}
	void JLight::RegisterJFunc()
	{
		auto defaultC = [](JGameObject* owner) -> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JLight>(Core::MakeGuid(), OBJECT_FLAG_NONE, owner);
			JLight* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
				return newComp;
			else
				return nullptr;
		};
		auto initC = [](const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)-> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JLight>(guid, objFlag, owner);
			JLight* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
				return newComp;
			else
				return nullptr;
		};
		auto loadC = [](std::wifstream& stream, JGameObject* owner) -> JComponent*
		{
			return LoadObject(stream, owner);
		};
		auto copyC = [](JComponent* ori, JGameObject* owner) -> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JLight>(Core::MakeGuid(), ori->GetFlag(), owner);
			JLight* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				if (newComp->Copy(ori))
					return newComp;
				else
				{
					newComp->BegineForcedDestroy();
					return nullptr;
				}
			}
			else
				return nullptr;
		};
		JCFI<JLight>::Register(defaultC, initC, loadC, copyC);

		static GetTypeNameCallable getTypeNameCallable{ &JLight::TypeName };
		static GetTypeInfoCallable getTypeInfoCallable{ &JLight::StaticTypeInfo };
		bool(*ptr)() = isAvailableoverlapLam;
		static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };

		static auto setFrameLam = [](JComponent& component) {static_cast<JLight*>(&component)->SetFrameDirty(); };
		static SetFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };

		static JCI::CTypeHint cTypeHint{ GetStaticComponentType(), true };
		static JCI::CTypeCommonFunc cTypeCommonFunc{ getTypeNameCallable, getTypeInfoCallable,isAvailableOverlapCallable };
		static JCI::CTypeInterfaceFunc cTypeInterfaceFunc{ &setFrameDirtyCallable };

		JCI::RegisterTypeInfo(cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
	}
	JLight::JLight(const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)
		:JLightInterface(TypeName(), guid, objFlag, owner)
	{
		lightType = J_LIGHT_TYPE::DIRECTIONAL;
		lightData = std::make_unique<JLightStruct>();
	}
	JLight::~JLight() {}
}