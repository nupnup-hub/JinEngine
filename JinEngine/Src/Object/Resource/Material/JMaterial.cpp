#include"JMaterial.h"
#include"../Shader/JShader.h"
#include"../Texture/JTexture.h"
#include"../JResourceManager.h" 
#include"../JResourceObjectFactory.h"
#include"../../Directory/JDirectory.h"
#include"../../../Application/JApplicationVariable.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Graphic/FrameResource/JMaterialConstants.h"
#include<fstream>

namespace JinEngine
{
	JMaterial::JMaterialInitData::JMaterialInitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		JDirectory* directory,
		const uint8 formatIndex)
		:JResourceInitData(name, guid, flag, directory, formatIndex)
	{}
	JMaterial::JMaterialInitData::JMaterialInitData(const std::wstring& name,
		JDirectory* directory,
		const uint8 formatIndex)
		: JResourceInitData(name, directory, formatIndex)
	{}
	JMaterial::JMaterialInitData::JMaterialInitData(JDirectory* directory,
		const uint8 formatIndex)
		: JResourceInitData(GetDefaultName<JMaterial>(), directory, formatIndex)
	{}

	J_RESOURCE_TYPE JMaterial::JMaterialInitData::GetResourceType() const noexcept
	{
		return J_RESOURCE_TYPE::MATERIAL;
	}

	J_RESOURCE_TYPE JMaterial::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	}
	std::wstring JMaterial::GetFormat()const noexcept
	{
		return GetAvailableFormat()[0];
	}
	std::vector<std::wstring> JMaterial::GetAvailableFormat()noexcept
	{
		static std::vector<std::wstring> format{ L".mat" };
		return format;
	}
	JShader* JMaterial::GetShader()const noexcept
	{
		return shader;
	}
	float JMaterial::GetMetallic() const noexcept
	{
		return metallic;
	}
	float JMaterial::GetRoughness() const noexcept
	{
		return roughness;
	}
	DirectX::XMFLOAT4 JMaterial::GetAlbedoColor() const noexcept
	{
		return albedoColor;
	}
	DirectX::XMFLOAT4X4 JMaterial::GetMatTransform()const  noexcept
	{
		return matTransform;
	}
	JTexture* JMaterial::GetAlbedoMap() const noexcept
	{
		return albedoMap;
	}
	JTexture* JMaterial::GetNormalMap() const noexcept
	{
		return normalMap;
	}
	JTexture* JMaterial::GetHeightMap() const noexcept
	{
		return heightMap;
	}
	JTexture* JMaterial::GetRoughnessMap() const noexcept
	{
		return roughnessMap;
	}
	JTexture* JMaterial::GetAmbientOcclusionMap() const noexcept
	{
		return ambientOcclusionMap;
	}
	void JMaterial::SetMetallic(float value) noexcept
	{
		metallic = value;
		SetFrameDirty();
	}
	void JMaterial::SetRoughness(float value) noexcept
	{
		roughness = value;
		SetFrameDirty();
	}
	void JMaterial::SetAlbedoColor(const DirectX::XMFLOAT4& value)noexcept
	{
		albedoColor = value;
		SetFrameDirty();
	}
	void JMaterial::SetMatTransform(const DirectX::XMFLOAT4X4& value)noexcept
	{
		matTransform = value;
		SetFrameDirty();
	}
	void JMaterial::SetAlbedoMap(JTexture* texture) noexcept
	{
		JTexture* be = albedoMap;
		JTexture* af = texture;

		if (IsActivated())
			CallOffResourceReference(albedoMap);
		albedoMap = texture;
		if (IsActivated())
			CallOnResourceReference(albedoMap);
		SetFrameDirty();
		TextureChange(be, af, SHADER_FUNCTION_ALBEDO_MAP);
	}
	void JMaterial::SetNormalMap(JTexture* texture) noexcept
	{
		JTexture* be = normalMap;
		JTexture* af = texture;

		if (IsActivated())
			CallOffResourceReference(normalMap);
		normalMap = texture;
		if (IsActivated())
			CallOnResourceReference(normalMap);
		SetFrameDirty();
		TextureChange(be, af, SHADER_FUNCTION_NORMAL_MAP);
	}
	void JMaterial::SetHeightMap(JTexture* texture) noexcept
	{
		JTexture* be = heightMap;
		JTexture* af = texture;

		if (IsActivated())
			CallOffResourceReference(heightMap);
		heightMap = texture;
		if (IsActivated())
			CallOnResourceReference(heightMap);
		SetFrameDirty();
		TextureChange(be, af, SHADER_FUNCTION_HEIGHT_MAP);
	}
	void JMaterial::SetRoughnessMap(JTexture* texture) noexcept
	{
		JTexture* be = roughnessMap;
		JTexture* af = texture;

		if (IsActivated())
			CallOffResourceReference(roughnessMap);
		roughnessMap = texture;
		if (IsActivated())
			CallOnResourceReference(roughnessMap);
		SetFrameDirty();
		TextureChange(be, af, SHADER_FUNCTION_ROUGHNESS_MAP);
	}
	void JMaterial::SetAmbientOcclusionMap(JTexture* texture) noexcept
	{
		JTexture* be = ambientOcclusionMap;
		JTexture* af = texture;

		if (IsActivated())
			CallOffResourceReference(ambientOcclusionMap);
		ambientOcclusionMap = texture;
		if (IsActivated())
			CallOnResourceReference(ambientOcclusionMap);
		SetFrameDirty();
		TextureChange(be, af, SHADER_FUNCTION_AMBIENT_OCCLUSION_MAP);
	}
	void JMaterial::SetShadow(bool value)noexcept
	{
		if (shadow == value)
			return;

		shadow = value;
		SetFrameDirty();
		if (shadow)
			SetNewFunctionFlag(Core::AddSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_SHADOW));
		else
			SetNewFunctionFlag(Core::MinusSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_SHADOW));
	}
	void JMaterial::SetLight(bool value)noexcept
	{
		if (light == value)
			return;

		light = value;
		SetFrameDirty();
		if (light)
			SetNewFunctionFlag(Core::AddSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_LIGHT));
		else
			SetNewFunctionFlag(Core::MinusSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_LIGHT));
	}
	void JMaterial::SetAlbedoOnly(bool value)noexcept
	{
		if (albedoOnly == value)
			return;

		albedoOnly = value;
		SetFrameDirty();
		if (albedoOnly)
			SetNewFunctionFlag(Core::AddSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_ALBEDO_MAP_ONLY));
		else
			SetNewFunctionFlag(Core::MinusSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_ALBEDO_MAP_ONLY));
	}
	void JMaterial::SetNonCulling(bool value)noexcept
	{
		if (nonCulling == value)
			return;

		nonCulling = value;
		SetFrameDirty();
		if (nonCulling)
			SetNewFunctionFlag(Core::AddSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_NONCULLING));
		else
			SetNewFunctionFlag(Core::MinusSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_NONCULLING));
	}
	void JMaterial::SetShadowMapWrite(bool value)noexcept
	{
		if (isShadowMapWrite == value)
			return;

		isShadowMapWrite = value;
		SetFrameDirty();
		if (isShadowMapWrite)
			SetNewFunctionFlag(Core::AddSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_WRITE_SHADOWMAP));
		else
			SetNewFunctionFlag(Core::MinusSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_WRITE_SHADOWMAP));
	}
	void JMaterial::SetBoundingObjectDepthTest(bool value)noexcept
	{
		if (isBoundingObjDepthTest == value)
			return;

		isBoundingObjDepthTest = value;
		SetFrameDirty();
		if (isBoundingObjDepthTest)
			SetNewFunctionFlag(Core::AddSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_DEPTH_TEST_BOUNDING_OBJECT));
		else
			SetNewFunctionFlag(Core::MinusSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_DEPTH_TEST_BOUNDING_OBJECT));
	}
	void JMaterial::SetSkyMaterial(bool value)noexcept
	{
		if (isSkyMateral == value)
			return;

		isSkyMateral = value;
		SetFrameDirty();
		if (isSkyMateral)
			SetNewFunctionFlag(Core::AddSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_SKY));
		else
			SetNewFunctionFlag(Core::MinusSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_SKY));
	}
	void JMaterial::SetDebugMaterial(bool value)noexcept
	{
		if (isDebugMaterial == value)
			return;

		isDebugMaterial = value;
		SetFrameDirty();
		if (isDebugMaterial)
			SetNewFunctionFlag(Core::AddSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_DEBUG));
		else
			SetNewFunctionFlag(Core::MinusSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_DEBUG));
	}
	void JMaterial::SetAlphaClip(bool value)noexcept
	{
		if (alphaClip == value)
			return;

		alphaClip = value;
		SetFrameDirty();
		if (alphaClip)
			SetNewFunctionFlag(Core::AddSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_ALPHA_CLIP));
		else
			SetNewFunctionFlag(Core::MinusSQValueEnum(shader->GetShaderFunctionFlag(), SHADER_FUNCTION_ALPHA_CLIP));
	}
	bool JMaterial::OnShadow()const noexcept
	{
		return shadow;
	}
	bool JMaterial::OnLight()const noexcept
	{
		return light;
	}
	bool JMaterial::OnAlbedoOnly()const noexcept
	{
		return albedoOnly;
	}
	bool JMaterial::OnNonCulling()const noexcept
	{
		return nonCulling;
	}
	bool JMaterial::OnShadowMapWrite()const noexcept
	{
		return isShadowMapWrite;
	}
	bool JMaterial::OnBoundingObjectDepthTest()const noexcept
	{
		return isBoundingObjDepthTest;
	}
	bool JMaterial::IsSkyMaterial()const noexcept
	{
		return isSkyMateral;
	}
	bool JMaterial::IsDebugMaterial()const noexcept
	{
		return isDebugMaterial;
	}
	bool JMaterial::HasAlbedoMapTexture() const noexcept
	{
		return albedoMap != nullptr;
	}
	bool JMaterial::HasNormalMapTexture() const noexcept
	{
		return normalMap != nullptr;
	}
	bool JMaterial::HasHeightMapTexture() const noexcept
	{
		return heightMap != nullptr;
	}
	bool JMaterial::HasRoughnessMapTexture() const noexcept
	{
		return roughnessMap != nullptr;
	}
	bool JMaterial::JMaterial::HasAmbientOcclusionMapTexture() const noexcept
	{
		return ambientOcclusionMap != nullptr;
	}
	void JMaterial::PopTexture(JTexture* texture)noexcept
	{
		const size_t tarGuid = texture->GetGuid();
		if (HasAlbedoMapTexture() && albedoMap->GetGuid() == tarGuid)
			SetAlbedoMap(nullptr);
		if (HasNormalMapTexture() && normalMap->GetGuid() == tarGuid)
			SetNormalMap(nullptr);
		if (HasHeightMapTexture() && heightMap->GetGuid() == tarGuid)
			SetHeightMap(nullptr);
		if (HasRoughnessMapTexture() && roughnessMap->GetGuid() == tarGuid)
			SetRoughnessMap(nullptr);
		if (HasAmbientOcclusionMapTexture() && ambientOcclusionMap->GetGuid() == tarGuid)
			SetAmbientOcclusionMap(nullptr);
	}
	void JMaterial::TextureChange(JTexture* be, JTexture* af, const J_GRAPHIC_SHADER_FUNCTION func)noexcept
	{
		if (be == nullptr && af != nullptr)
			SetNewFunctionFlag(Core::AddSQValueEnum(shader->GetShaderFunctionFlag(), func));
		else if (be != nullptr && af == nullptr)
			SetNewFunctionFlag(Core::MinusSQValueEnum(shader->GetShaderFunctionFlag(), func));
	}
	void JMaterial::SetNewFunctionFlag(const J_GRAPHIC_SHADER_FUNCTION newFunc)
	{
		if (shader == nullptr || shader->GetShaderFunctionFlag() != newFunc)
			SetShader(JRFI<JShader>::Create(Core::JPtrUtil::MakeOwnerPtr<JShader::InitData>(newFunc)));
	}
	//쉐이더는 머테리얼 activate 여부에 관계없이 소유될때 참조 카운트가 증감한다. 
	void JMaterial::SetShader(JShader* newShader)noexcept
	{
		CallOffResourceReference(shader);
		if (shader != nullptr && CallGetResourceReferenceCount(*shader) == 0)
			BeginDestroy(shader);
		shader = newShader;
		CallOnResourceReference(shader);
	}
	bool JMaterial::UpdateFrame(Graphic::JMaterialConstants& constant)
	{
		if (IsFrameDirted())
		{
			constant.AlbedoColor = albedoColor;
			constant.Metalic = metallic;
			constant.Roughness = roughness;
			XMStoreFloat4x4(&constant.MatTransform, XMMatrixTranspose(XMLoadFloat4x4(&matTransform)));
			if (albedoMap != nullptr)
				constant.AlbedoMapIndex = CallGetTxtVectorIndex(*albedoMap);
			if (normalMap != nullptr)
				constant.NormalMapIndex = CallGetTxtVectorIndex(*normalMap);
			if (heightMap != nullptr)
				constant.HeightMapIndex = CallGetTxtVectorIndex(*heightMap);
			if (roughnessMap != nullptr)
				constant.RoughnessMapIndex = CallGetTxtVectorIndex(*roughnessMap);
			if (ambientOcclusionMap != nullptr)
				constant.AmbientOcclusionMapIndex = CallGetTxtVectorIndex(*ambientOcclusionMap);
			return true;
		}
		else
			return false;
	}
	void JMaterial::DoCopy(JObject* ori)
	{
		JMaterial* oriM = static_cast<JMaterial*>(ori);
		CopyRFile(*oriM);
		ReadMateiralData();
	}
	void JMaterial::DoActivate() noexcept
	{
		JResourceObject::DoActivate();
		SetValid(true);
		SetFrameDirty();
		CallOnResourceReference(albedoMap);
		CallOnResourceReference(normalMap);
		CallOnResourceReference(heightMap);
		CallOnResourceReference(roughnessMap);
		CallOnResourceReference(ambientOcclusionMap);
	}
	void JMaterial::DoDeActivate()noexcept
	{
		StoreObject(this);
		JResourceObject::DoDeActivate();
		SetValid(false);
		OffFrameDirty();
		CallOffResourceReference(albedoMap);
		CallOffResourceReference(normalMap);
		CallOffResourceReference(heightMap);
		CallOffResourceReference(roughnessMap);
		CallOffResourceReference(ambientOcclusionMap);
	}
	bool JMaterial::WriteMaterialData()
	{
		std::wofstream stream;
		stream.open(GetPath(), std::ios::out | std::ios::binary);

		if (stream.is_open())
		{
			JFileIOHelper::StoreAtomicData(stream, L"Shadow", shadow);
			JFileIOHelper::StoreAtomicData(stream, L"Light", light);
			JFileIOHelper::StoreAtomicData(stream, L"AlbedoOnly", albedoOnly);
			JFileIOHelper::StoreAtomicData(stream, L"NonCulling", nonCulling);
			JFileIOHelper::StoreAtomicData(stream, L"ShadowMapWrite", isShadowMapWrite);
			JFileIOHelper::StoreAtomicData(stream, L"BoundingObjDepthTest", isBoundingObjDepthTest);
			JFileIOHelper::StoreAtomicData(stream, L"SkyMaterial", isSkyMateral);
			JFileIOHelper::StoreAtomicData(stream, L"DebugMaterial", isDebugMaterial);
			JFileIOHelper::StoreAtomicData(stream, L"AlphaClip", alphaClip);
			JFileIOHelper::StoreAtomicData(stream, L"Metallic", metallic);
			JFileIOHelper::StoreAtomicData(stream, L"Roughness", roughness);

			JFileIOHelper::StoreXMFloat4(stream, L"AlbedoColor", albedoColor);
			JFileIOHelper::StoreXMFloat4x4(stream, L"Matransform", matTransform);

			JFileIOHelper::StoreHasObjectIden(stream, albedoMap);
			JFileIOHelper::StoreHasObjectIden(stream, normalMap);
			JFileIOHelper::StoreHasObjectIden(stream, heightMap);
			JFileIOHelper::StoreHasObjectIden(stream, roughnessMap);
			JFileIOHelper::StoreHasObjectIden(stream, ambientOcclusionMap);

			stream.close();
			return true;
		}
		else
			return false;
	}
	bool JMaterial::ReadMateiralData()
	{
		std::wifstream stream;
		stream.open(GetPath(), std::ios::in | std::ios::binary);

		if (stream.is_open())
		{
			bool sShadow = false;
			bool sLight = false;
			bool sAlbedoOnly = false;
			bool sNonCulling = false;
			bool sIsShadowMapWrite = false;
			bool sBoundingObjDepthTest = false;
			bool sIsSkyMateral = false;
			bool sIsDebugMaterial = false;
			bool sAlphaclip = false;
			float sMetallic;
			float sRoughness;
			DirectX::XMFLOAT4 sAlbedoColor;
			DirectX::XMFLOAT4X4 sMatTransform;

			JFileIOHelper::LoadAtomicData(stream, sShadow);
			JFileIOHelper::LoadAtomicData(stream, sLight);
			JFileIOHelper::LoadAtomicData(stream, sAlbedoOnly);
			JFileIOHelper::LoadAtomicData(stream, sNonCulling);
			JFileIOHelper::LoadAtomicData(stream, sIsShadowMapWrite);
			JFileIOHelper::LoadAtomicData(stream, sBoundingObjDepthTest);
			JFileIOHelper::LoadAtomicData(stream, sIsSkyMateral);
			JFileIOHelper::LoadAtomicData(stream, sIsDebugMaterial);
			JFileIOHelper::LoadAtomicData(stream, sAlphaclip);
			JFileIOHelper::LoadAtomicData(stream, sMetallic);
			JFileIOHelper::LoadAtomicData(stream, sRoughness);

			JFileIOHelper::LoadXMFloat4(stream, sAlbedoColor);
			JFileIOHelper::LoadXMFloat4x4(stream, sMatTransform);

			JIdentifier* sAlbedoMap = JFileIOHelper::LoadHasObjectIden(stream);
			JIdentifier* sNormalMap = JFileIOHelper::LoadHasObjectIden(stream);
			JIdentifier* sHeightMap = JFileIOHelper::LoadHasObjectIden(stream);
			JIdentifier* sRoughnessMap = JFileIOHelper::LoadHasObjectIden(stream);
			JIdentifier* sAmbientOcclusionMap = JFileIOHelper::LoadHasObjectIden(stream);
			stream.close();

			SetShadow(sShadow);
			SetLight(sLight);
			SetAlbedoOnly(sAlbedoOnly);
			SetNonCulling(sNonCulling);
			SetShadowMapWrite(sIsShadowMapWrite);
			SetBoundingObjectDepthTest(sBoundingObjDepthTest);
			SetSkyMaterial(sIsSkyMateral);
			SetDebugMaterial(sIsDebugMaterial);
			SetAlphaClip(sAlphaclip);

			SetMetallic(sMetallic);
			SetRoughness(sRoughness);
			SetAlbedoColor(sAlbedoColor);
			SetMatTransform(sMatTransform);

			if (sAlbedoMap != nullptr && sAlbedoMap->GetTypeInfo().IsA(JTexture::StaticTypeInfo()))
				SetAlbedoMap(static_cast<JTexture*>(sAlbedoMap));
			if (sNormalMap != nullptr && sNormalMap->GetTypeInfo().IsA(JTexture::StaticTypeInfo()))
				SetNormalMap(static_cast<JTexture*>(sNormalMap));
			if (sHeightMap != nullptr && sHeightMap->GetTypeInfo().IsA(JTexture::StaticTypeInfo()))
				SetHeightMap(static_cast<JTexture*>(sHeightMap));
			if (sRoughnessMap != nullptr && sRoughnessMap->GetTypeInfo().IsA(JTexture::StaticTypeInfo()))
				SetRoughnessMap(static_cast<JTexture*>(sRoughnessMap));
			if (sAmbientOcclusionMap != nullptr && sAmbientOcclusionMap->GetTypeInfo().IsA(JTexture::StaticTypeInfo()))
				SetAmbientOcclusionMap(static_cast<JTexture*>(sAmbientOcclusionMap));
			return true;
		}
		else
			return false;
	}
	void JMaterial::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
	{
		if (iden == GetGuid())
			return;

		if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
		{
			const size_t objGuid = jRobj->GetGuid();
			if (albedoMap != nullptr && albedoMap->GetGuid() == objGuid)
				SetAlbedoMap(nullptr);
			if (normalMap != nullptr && normalMap->GetGuid() == objGuid)
				SetNormalMap(nullptr);
			if (heightMap != nullptr && heightMap->GetGuid() == objGuid)
				SetHeightMap(nullptr);
			if (roughnessMap != nullptr && roughnessMap->GetGuid() == objGuid)
				SetRoughnessMap(nullptr);
			if (ambientOcclusionMap != nullptr && ambientOcclusionMap->GetGuid() == objGuid)
				SetAmbientOcclusionMap(nullptr);

			if (shader != nullptr && shader->GetGuid() == objGuid)
				SetShader(nullptr);
		}
	}
	Core::J_FILE_IO_RESULT JMaterial::CallStoreResource()
	{
		return StoreObject(this);
	}
	Core::J_FILE_IO_RESULT JMaterial::StoreObject(JMaterial* material)
	{
		if (material == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (((int)material->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		std::wofstream stream;
		stream.open(material->GetMetafilePath(), std::ios::out | std::ios::binary);
		Core::J_FILE_IO_RESULT storeMetaRes = JResourceObject::StoreMetadata(stream, material);
		stream.close();

		if (storeMetaRes != Core::J_FILE_IO_RESULT::SUCCESS)
			return storeMetaRes;

		if (material->WriteMaterialData())
			return Core::J_FILE_IO_RESULT::SUCCESS;
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JMaterial* JMaterial::LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;

		std::wifstream stream;
		stream.open(pathData.engineMetaFileWPath, std::ios::in | std::ios::binary);
		JResourceMetaData metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);
		stream.close();

		JMaterial* newMaterial = nullptr;
		if (directory->HasFile(pathData.name))
			newMaterial = JResourceManager::Instance().GetResourceByPath<JMaterial>(pathData.engineFileWPath);

		if (newMaterial == nullptr && loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
		{
			JMaterialInitData initdata{ pathData.name, metadata.guid,metadata.flag, directory, (uint8)metadata.formatIndex };
			if (initdata.IsValidLoadData())
			{
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JMaterial>(initdata);
				newMaterial = ownerPtr.Get();
				if (!AddInstance(std::move(ownerPtr)))
					return nullptr;
			}
		}

		if (newMaterial == nullptr)
			return nullptr;

		if (newMaterial->IsValid())
			return newMaterial;
		else if (newMaterial->ReadMateiralData())
		{
			newMaterial->SetValid(true);
			return newMaterial;
		}
		else
		{
			JObject::BegineForcedDestroy(newMaterial);
			return nullptr;
		}
	}
	void JMaterial::RegisterJFunc()
	{
		auto defaultC = [](Core::JOwnerPtr<JResourceInitData>initdata) ->JResourceObject*
		{
			if (initdata.IsValid() && initdata->GetResourceType() == J_RESOURCE_TYPE::MATERIAL && initdata->IsValidCreateData())
			{
				JMaterialInitData* mInitdata = static_cast<JMaterialInitData*>(initdata.Get());
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JMaterial>(*mInitdata);
				JMaterial* newMaterial = ownerPtr.Get();
				if (AddInstance(std::move(ownerPtr)))
				{
					StoreObject(newMaterial);
					return newMaterial;
				}
			}
			return nullptr;
		};
		auto loadC = [](JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori, JDirectory* directory)->JResourceObject*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JMaterial>(InitData(ori->GetName(),
				Core::MakeGuid(),
				ori->GetFlag(),
				directory,
				GetFormatIndex<JMaterial>(ori->GetFormat())));

			JMaterial* newMaterial = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				newMaterial->Copy(ori);
				return newMaterial;
			}
			else
				return nullptr;
		};
		JRFI<JMaterial>::Register(defaultC, loadC, copyC);

		auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex<JMaterial>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JMaterial::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JMaterial::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

		static auto setFrameLam = [](JResourceObject& jRobj)
		{
			static_cast<JMaterial*>(&jRobj)->SetFrameDirty();
		};
		static auto setBuffIndexLam = [](JResourceObject& jRobj, const uint& value)
		{
			static_cast<JMaterial*>(&jRobj)->SetFrameBuffOffset(value);
		};
		static SetFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };
		static SetFrameBuffIndexCallable setFrameBuffIndexCallable{ setBuffIndexLam };

		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{J_RESOURCE_TYPE::SHADER}, false, true };
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };
		static RTypeInterfaceFunc rTypeiFunc{ &setFrameDirtyCallable, &setFrameBuffIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, rTypeiFunc);
	}
	JMaterial::JMaterial(const JMaterialInitData& initdata)
		: JMaterialInterface(initdata)
	{
		SetNewFunctionFlag(SHADER_FUNCTION_NONE);
		AddEventListener(*JResourceManager::Instance().EvInterface(), GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		SetFrameDirty();
	}
	JMaterial::~JMaterial()
	{
		RemoveListener(*JResourceManager::Instance().EvInterface(), GetGuid());
	}
}
