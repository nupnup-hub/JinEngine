#include"JMaterial.h"
#include"../Shader/JShader.h"
#include"../Texture/JTexture.h"
#include"../JResourceManager.h" 
#include"../JResourceObjectFactory.h"
#include"../../Directory/JDirectory.h"
#include"../../../Application/JApplicationVariable.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Graphic/FrameResource/JMaterialConstants.h"

namespace JinEngine
{
	J_RESOURCE_TYPE JMaterial::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	}
	std::string JMaterial::GetFormat()const noexcept
	{
		return GetAvailableFormat()[0];
	}
	std::vector<std::string> JMaterial::GetAvailableFormat()noexcept
	{
		static std::vector<std::string> format{ ".mat" };
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
	JTexture* JMaterial::GetDiffsueMap() const noexcept
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

		if (albedoMap != nullptr && IsActivated())
			OffResourceReference(*albedoMap);
		albedoMap = texture;
		if (albedoMap != nullptr && IsActivated())
			OnResourceReference(*albedoMap);
		SetFrameDirty();

		TextureChange(be, af, SHADER_FUNCTION_ALBEDO_MAP);
	}
	void JMaterial::SetNormalMap(JTexture* texture) noexcept
	{
		JTexture* be = normalMap;
		JTexture* af = texture;

		if (normalMap != nullptr && IsActivated())
			OffResourceReference(*normalMap);
		normalMap = texture;
		if (normalMap != nullptr && IsActivated())
			OnResourceReference(*normalMap);
		SetFrameDirty();

		TextureChange(be, af, SHADER_FUNCTION_NORMAL_MAP);
	}
	void JMaterial::SetHeightMap(JTexture* texture) noexcept
	{
		JTexture* be = heightMap;
		JTexture* af = texture;

		if (heightMap != nullptr && IsActivated())
			OffResourceReference(*heightMap);
		heightMap = texture;
		if (heightMap != nullptr && IsActivated())
			OnResourceReference(*heightMap);
		SetFrameDirty();

		TextureChange(be, af, SHADER_FUNCTION_HEIGHT_MAP);
	}
	void JMaterial::SetRoughnessMap(JTexture* texture) noexcept
	{
		JTexture* be = roughnessMap;
		JTexture* af = texture;

		if (roughnessMap != nullptr && IsActivated())
			OffResourceReference(*roughnessMap);
		roughnessMap = texture;
		if (IsActivated())
			OnResourceReference(*roughnessMap);
		SetFrameDirty();
		TextureChange(be, af, SHADER_FUNCTION_ROUGHNESS_MAP);
	}
	void JMaterial::SetAmbientOcclusionMap(JTexture* texture) noexcept
	{
		JTexture* be = ambientOcclusionMap;
		JTexture* af = texture;

		if (ambientOcclusionMap != nullptr && IsActivated())
			OffResourceReference(*ambientOcclusionMap);
		ambientOcclusionMap = texture;
		if (IsActivated())
			OnResourceReference(*ambientOcclusionMap);
		SetFrameDirty();

		TextureChange(be, af, SHADER_FUNCTION_AMBIENT_OCCLUSION_MAP);
	}
	void JMaterial::SetShadow(bool value)noexcept
	{
		light = value;
		if (light)
			SetNewFunctionFlag(JShader::AddShaderFunctionFlag(shader->GetShaderFunctionFlag(),
				SHADER_FUNCTION_SHADOW));
		else
			SetNewFunctionFlag(JShader::MinusShaderFunctionFlag(shader->GetShaderFunctionFlag(),
				SHADER_FUNCTION_SHADOW));
	}
	void JMaterial::SetLight(bool value)noexcept
	{
		shadow = value;
		if (shadow)
			SetNewFunctionFlag(JShader::AddShaderFunctionFlag(shader->GetShaderFunctionFlag(),
				SHADER_FUNCTION_LIGHT));
		else
			SetNewFunctionFlag(JShader::MinusShaderFunctionFlag(shader->GetShaderFunctionFlag(),
				SHADER_FUNCTION_LIGHT));
	}
	void JMaterial::SetAlbedoOnly(bool value)noexcept
	{
		albedoOnly = value;
		if (albedoOnly)
			SetNewFunctionFlag(JShader::AddShaderFunctionFlag(shader->GetShaderFunctionFlag(),
				SHADER_FUNCTION_ALBEDO_MAP_ONLY));
		else
			SetNewFunctionFlag(JShader::MinusShaderFunctionFlag(shader->GetShaderFunctionFlag(),
				SHADER_FUNCTION_ALBEDO_MAP_ONLY));
	}
	void JMaterial::SetNonCulling(bool value)noexcept
	{
		nonCulling = value;
		if (nonCulling)
			SetNewFunctionFlag(JShader::AddShaderFunctionFlag(shader->GetShaderFunctionFlag(),
				SHADER_FUNCTION_NONCULLING));
		else
			SetNewFunctionFlag(JShader::MinusShaderFunctionFlag(shader->GetShaderFunctionFlag(),
				SHADER_FUNCTION_NONCULLING));
	}
	void JMaterial::SetShadowMap(bool value)noexcept
	{
		shadowMap = value;
		if (shadowMap)
			SetNewFunctionFlag(JShader::AddShaderFunctionFlag(shader->GetShaderFunctionFlag(),
				SHADER_FUNCTION_SHADOW_MAP));
		else
			SetNewFunctionFlag(JShader::MinusShaderFunctionFlag(shader->GetShaderFunctionFlag(),
				SHADER_FUNCTION_SHADOW_MAP));
	}
	void JMaterial::SetSkyMaterial(bool value)noexcept
	{
		isSkyMateral = value;
		if (isSkyMateral)
		{
			SetNewFunctionFlag(JShader::AddShaderFunctionFlag(shader->GetShaderFunctionFlag(),
				SHADER_FUNCTION_SKY));
		}
		else
		{
			SetNewFunctionFlag(JShader::MinusShaderFunctionFlag(shader->GetShaderFunctionFlag(),
				SHADER_FUNCTION_SKY));
		}
	}
	void JMaterial::SetDebugMaterial(bool value)noexcept
	{
		isDebugMaterial = value;
		if (isDebugMaterial)
		{
			SetNewFunctionFlag(JShader::AddShaderFunctionFlag(shader->GetShaderFunctionFlag(),
				SHADER_FUNCTION_DEBUG));
		}
		else
		{
			SetNewFunctionFlag(JShader::MinusShaderFunctionFlag(shader->GetShaderFunctionFlag(),
				SHADER_FUNCTION_DEBUG));
		}
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
	void JMaterial::TextureChange(JTexture* be, JTexture* af, const J_SHADER_FUNCTION func)noexcept
	{
		if (be == nullptr && af != nullptr)
			SetNewFunctionFlag(JShader::AddShaderFunctionFlag(shader->GetShaderFunctionFlag(), func));
		else if (be != nullptr && af == nullptr)
			SetNewFunctionFlag(JShader::MinusShaderFunctionFlag(shader->GetShaderFunctionFlag(), func));
	}
	void JMaterial::SetNewFunctionFlag(const J_SHADER_FUNCTION newFunc)
	{
		if (shader == nullptr || shader->GetShaderFunctionFlag() != newFunc)
			SetShader(JRFI<JShader>::Create(*JResourceManager::Instance().GetDirectory(JApplicationVariable::GetProjectShaderMetafilePath()), newFunc));
	}
	void JMaterial::SetShader(JShader* newShader)noexcept
	{
		if (shader != nullptr)
			OffResourceReference(*shader);

		if (shader != nullptr && GetResourceReferenceCount(*shader) == 0 && (shader->GetFlag() & J_OBJECT_FLAG::OBJECT_FLAG_UNDESTROYABLE) == 0)
		{
			shader->BeginDestroy();
			shader = nullptr;
		}
		shader = newShader;

		if (shader != nullptr)
			OnResourceReference(*shader);
	}
	bool JMaterial::UpdateFrame(Graphic::JMaterialConstants& constant)
	{
		DirectX::XMFLOAT4X4 matTransformFloat4x4 = matTransform;
		DirectX::XMMATRIX matTransform = XMLoadFloat4x4(&matTransformFloat4x4);

		constant.AlbedoColor = albedoColor;
		constant.Metalic = metallic;
		constant.Roughness = roughness;
		XMStoreFloat4x4(&constant.MatTransform, XMMatrixTranspose(matTransform));
		if (albedoMap != nullptr)
			constant.AlbedoMapIndex = GetTxtVectorIndex(*albedoMap);
		if (normalMap != nullptr)
			constant.NormalMapIndex = GetTxtVectorIndex(*normalMap);
		if (heightMap != nullptr)
			constant.HeightMapIndex = GetTxtVectorIndex(*heightMap);
		if (roughnessMap != nullptr)
			constant.RoughnessMapIndex = GetTxtVectorIndex(*roughnessMap);
		if (ambientOcclusionMap != nullptr)
			constant.AmbientOcclusionMapIndex = GetTxtVectorIndex(*ambientOcclusionMap);

		return true;
	}
	void JMaterial::DoActivate() noexcept
	{
		JResourceObject::DoActivate();
		StuffResource();
		SetFrameDirty();
	}
	void JMaterial::DoDeActivate()noexcept
	{
		JResourceObject::DoDeActivate();
		ClearResource();
		OffFrameDirty();
	}
	void JMaterial::StuffResource()
	{
		if (!IsValid())
		{
			if (ReadMateiralData())
				SetValid(true);
		}
	}
	void JMaterial::ClearResource()
	{
		if (IsValid())
		{
			SetShader(nullptr);
			SetAlbedoMap(nullptr);
			SetNormalMap(nullptr);
			SetHeightMap(nullptr);
			SetRoughnessMap(nullptr);
			SetAmbientOcclusionMap(nullptr);
			SetValid(false);
		}
	}
	bool JMaterial::ReadMateiralData()
	{
		std::wifstream stream;
		stream.open(GetWPath(), std::ios::in | std::ios::binary);

		if (stream.is_open())
		{
			size_t shaderGuid;
			bool sShadow = false;
			bool sLight = false;
			bool sAlbedoOnly = false;
			bool sNonCulling = false;
			bool sShadowMap = false;
			bool sIsSkyMateral = false;
			bool sIsDebugMaterial = false;
			float sMetallic;
			float sRoughness;
			DirectX::XMFLOAT4 sAlbedoColor;
			DirectX::XMFLOAT4X4 sMatTransform;

			size_t albedoGuid = 0;
			size_t normalGuid = 0;
			size_t heightGuid = 0;
			size_t roughnessGuid = 0;
			size_t ambientOcclusionGuid = 0;
			bool hasAlbedo = false;
			bool hasNormal = false;
			bool hasHeight = false;
			bool hasRoughness = false;
			bool hasAmbientOcclusion = false;

			stream >> shaderGuid;
			stream >> sShadow;
			stream >> sLight;
			stream >> sAlbedoOnly;
			stream >> sNonCulling;
			stream >> sShadowMap;
			stream >> sIsSkyMateral;
			stream >> sIsDebugMaterial;

			stream >> sMetallic;
			stream >> sRoughness;
			stream >> sAlbedoColor.x; stream >> sAlbedoColor.y; stream >> sAlbedoColor.z; stream >> sAlbedoColor.w;

			stream >> sMatTransform._11; stream >> sMatTransform._12; stream >> sMatTransform._13; stream >> sMatTransform._14;
			stream >> sMatTransform._21; stream >> sMatTransform._22; stream >> sMatTransform._23; stream >> sMatTransform._24;
			stream >> sMatTransform._31; stream >> sMatTransform._32; stream >> sMatTransform._33; stream >> sMatTransform._34;
			stream >> sMatTransform._41; stream >> sMatTransform._42; stream >> sMatTransform._43; stream >> sMatTransform._44;

			stream >> albedoGuid; stream >> hasAlbedo;
			stream >> normalGuid; stream >> hasNormal;
			stream >> heightGuid; stream >> hasHeight;
			stream >> roughnessGuid; stream >> hasRoughness;
			stream >> ambientOcclusionGuid; stream >> hasAmbientOcclusion;
			stream.close();

			if (hasAlbedo)
				albedoMap = JResourceManager::Instance().GetResource<JTexture>(albedoGuid);
			if (hasNormal)
				normalMap = JResourceManager::Instance().GetResource<JTexture>(normalGuid);
			if (hasHeight)
				heightMap = JResourceManager::Instance().GetResource<JTexture>(heightGuid);
			if (hasRoughness)
				roughnessMap = JResourceManager::Instance().GetResource<JTexture>(roughnessGuid);
			if (hasAmbientOcclusion)
				ambientOcclusionMap = JResourceManager::Instance().GetResource<JTexture>(ambientOcclusionGuid);

			SetNewFunctionFlag(CalculateShaderFunc());
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
			const size_t txtGuid = jRobj->GetGuid();
			if (albedoMap != nullptr && albedoMap->GetGuid() == txtGuid)
				SetAlbedoMap(nullptr);
			else if (normalMap != nullptr && normalMap->GetGuid() == txtGuid)
				SetNormalMap(nullptr);
			else if (heightMap != nullptr && heightMap->GetGuid() == txtGuid)
				SetHeightMap(nullptr);
			else if (roughnessMap != nullptr && roughnessMap->GetGuid() == txtGuid)
				SetRoughnessMap(nullptr);
			else if (ambientOcclusionMap != nullptr && ambientOcclusionMap->GetGuid() == txtGuid)
				SetAmbientOcclusionMap(nullptr);
		}
	}
	J_SHADER_FUNCTION JMaterial::CalculateShaderFunc()noexcept
	{
		J_SHADER_FUNCTION func = SHADER_FUNCTION_NONE;
		if (albedoMap != nullptr)
			func = (J_SHADER_FUNCTION)(func | SHADER_FUNCTION_ALBEDO_MAP);
		if (normalMap != nullptr)
			func = (J_SHADER_FUNCTION)(func | SHADER_FUNCTION_NORMAL_MAP);
		if (heightMap != nullptr)
			func = (J_SHADER_FUNCTION)(func | SHADER_FUNCTION_HEIGHT_MAP);
		if (roughnessMap != nullptr)
			func = (J_SHADER_FUNCTION)(func | SHADER_FUNCTION_ROUGHNESS_MAP);
		if (ambientOcclusionMap != nullptr)
			func = (J_SHADER_FUNCTION)(func | SHADER_FUNCTION_AMBIENT_OCCLUSION_MAP);
		if (albedoOnly)
			func = (J_SHADER_FUNCTION)(func | SHADER_FUNCTION_ALBEDO_MAP_ONLY);
		if (nonCulling)
			func = (J_SHADER_FUNCTION)(func | SHADER_FUNCTION_NONCULLING);
		if (isSkyMateral)
			func = (J_SHADER_FUNCTION)(func | SHADER_FUNCTION_SKY);

		return func;
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

		stream.open(material->GetWPath(), std::ios::out | std::ios::binary);
		if (stream.is_open())
		{
			stream << material->GetShader()->GetGuid() << "\n" <<
				material->shadow << "\n" <<
				material->light << "\n" <<
				material->albedoOnly << "\n" <<
				material->nonCulling << "\n" <<
				material->shadowMap << "\n" <<
				material->isSkyMateral << "\n" <<
				material->isDebugMaterial << "\n" <<
				material->GetMetallic() << "\n" <<
				material->GetRoughness() << "\n" <<
				material->albedoColor.x << " " <<
				material->albedoColor.y << " " <<
				material->albedoColor.z << " " <<
				material->albedoColor.w << "\n" <<
				material->matTransform._11 << " " <<
				material->matTransform._12 << " " <<
				material->matTransform._13 << " " <<
				material->matTransform._14 << " " <<
				material->matTransform._21 << " " <<
				material->matTransform._22 << " " <<
				material->matTransform._23 << " " <<
				material->matTransform._24 << " " <<
				material->matTransform._31 << " " <<
				material->matTransform._32 << " " <<
				material->matTransform._33 << " " <<
				material->matTransform._34 << " " <<
				material->matTransform._41 << " " <<
				material->matTransform._42 << " " <<
				material->matTransform._43 << " " <<
				material->matTransform._44 << "\n";

			if (material->albedoMap != nullptr)
			{
				stream << material->albedoMap->GetGuid();
				stream << true;
			}
			else
			{
				stream << 0;
				stream << false;
			}

			if (material->normalMap != nullptr)
			{
				stream << material->normalMap->GetGuid();
				stream << true;
			}
			else
			{
				stream << 0;
				stream << false;
			}

			if (material->heightMap != nullptr)
			{
				stream << material->heightMap->GetGuid();
				stream << true;
			}
			else
			{
				stream << 0;
				stream << false;
			}

			if (material->roughnessMap != nullptr)
			{
				stream << material->roughnessMap->GetGuid();
				stream << true;
			}
			else
			{
				stream << 0;
				stream << false;
			}

			if (material->ambientOcclusionMap != nullptr)
			{
				stream << material->ambientOcclusionMap->GetGuid();
				stream << true;
			}
			else
			{
				stream << 0;
				stream << false;
			}

			stream.close();
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JMaterial* JMaterial::LoadObject(JDirectory* directory, const JResourcePathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;

		if (!JResourceObject::IsResourceFormat<JMaterial>(pathData.format))
			return nullptr;

		std::wifstream stream;
		stream.open(ConvertMetafilePath(pathData.wstrPath), std::ios::in | std::ios::binary);
		ObjectMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);
		stream.close();

		stream.open(pathData.wstrPath, std::ios::in | std::ios::binary);
		if (stream.is_open())
		{
			JMaterial* newMaterial = nullptr;
			if (directory->HasFile(pathData.fullName))
				newMaterial = JResourceManager::Instance().GetResourceByPath<JMaterial>(pathData.strPath);

			if (newMaterial == nullptr)
			{
				if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
					newMaterial = new JMaterial(pathData.name, metadata.guid, (J_OBJECT_FLAG)metadata.flag, directory, GetFormatIndex<JMaterial>(pathData.format));
				else
					newMaterial = new JMaterial(pathData.name, Core::MakeGuid(), J_OBJECT_FLAG::OBJECT_FLAG_NONE, directory, GetFormatIndex<JMaterial>(pathData.format));
			}

			if (newMaterial->IsValid())
				return newMaterial;
			else if (newMaterial->ReadMateiralData())
			{
				newMaterial->SetValid(true);
				return newMaterial;
			}
			else
			{
				delete newMaterial;
				return nullptr;
			}
		}
		else
			return nullptr;
	}
	void JMaterial::RegisterJFunc()
	{
		auto defaultC = [](JDirectory* owner) ->JResourceObject*
		{
			return new JMaterial(owner->MakeUniqueFileName(GetDefaultName<JMaterial>()),
				Core::MakeGuid(),
				OBJECT_FLAG_NONE,
				owner,
				JResourceObject::GetDefaultFormatIndex());
		};
		auto initC = [](const std::string& name, const size_t guid, const J_OBJECT_FLAG objFlag, JDirectory* directory, const uint8 formatIndex)-> JResourceObject*
		{
			return  new JMaterial(name, guid, objFlag, directory, formatIndex);
		};
		auto loadC = [](JDirectory* directory, const JResourcePathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori)->JResourceObject*
		{
			return static_cast<JMaterial*>(ori)->CopyResource();
		};
		JRFI<JMaterial>::Register(defaultC, initC, loadC, copyC);

		auto getFormatIndexLam = [](const std::string& format) {return JResourceObject::GetFormatIndex<JMaterial>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JMaterial::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JMaterial::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

		static auto setFrameLam = [](JResourceObject& jRobj)
		{
			static_cast<JMaterial*>(&jRobj)->SetFrameDirty();
		};
		static auto setBuffIndexLam = [](JResourceObject& jRobj, const uint& index)
		{
			static_cast<JMaterial*>(&jRobj)->SetBuffIndex(index);
		};
		static SetFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };
		static SetBuffIndexCallable setBuffIndexCallable{ setBuffIndexLam };

		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{J_RESOURCE_TYPE::SHADER}, false, true, true };
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };
		static RTypeInterfaceFunc rTypeiFunc{ &setFrameDirtyCallable, &setBuffIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, rTypeiFunc);
	}
	JMaterial::JMaterial(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		: JMaterialInterface(name, guid, flag, directory, formatIndex)
	{
		SetNewFunctionFlag(SHADER_FUNCTION_NONE);
	}
	JMaterial::~JMaterial()
	{}
}
