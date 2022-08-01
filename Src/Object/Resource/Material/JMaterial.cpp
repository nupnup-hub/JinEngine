#include"JMaterial.h"
#include"../Shader/JShader.h"
#include"../Texture/JTexture.h"
#include"../JResourceManager.h" 
#include"../JResourceObjectFactory.h"
#include"../../../Application/JApplicationVariable.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Graphic/FrameResource/JMaterialConstants.h"

namespace JinEngine
{
	uint JMaterial::GetMatCBIndex() const noexcept
	{
		return matCBIndex;
	}
	J_RESOURCE_TYPE JMaterial::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	}
	std::string JMaterial::GetFormat()const noexcept
	{
		return GetAvailableFormat()[0];
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
	std::vector<std::string> JMaterial::GetAvailableFormat()noexcept
	{
		static std::vector<std::string> format{ ".mat" };
		return format;
	}
	void JMaterial::SetMetallic(float value) noexcept
	{
		metallic = value;
		StuffFrameDirty();
	}
	void JMaterial::SetRoughness(float value) noexcept
	{
		roughness = value;
		StuffFrameDirty();
	}
	void JMaterial::SetAlbedoColor(const DirectX::XMFLOAT4& value)noexcept
	{
		albedoColor = value;
		StuffFrameDirty();
	}
	void JMaterial::SetMatTransform(const DirectX::XMFLOAT4X4& value)noexcept
	{
		matTransform = value;
		StuffFrameDirty();
	}
	void JMaterial::SetAlbedoMap(JTexture* texture) noexcept
	{
		JTexture* be = albedoMap;
		JTexture* af = texture;

		if (albedoMap != nullptr && IsActivated())
			albedoMap->OffReference();
		albedoMap = texture;
		if (albedoMap != nullptr && IsActivated())
			albedoMap->OnReference();
		StuffFrameDirty();

		TextureChange(be, af, SHADER_FUNCTION_ALBEDO_MAP);
	}
	void JMaterial::SetNormalMap(JTexture* texture) noexcept
	{
		JTexture* be = normalMap;
		JTexture* af = texture;

		if (normalMap != nullptr && IsActivated())
			normalMap->OffReference();
		normalMap = texture;
		if (normalMap != nullptr && IsActivated())
			normalMap->OnReference();
		StuffFrameDirty();

		TextureChange(be, af, SHADER_FUNCTION_NORMAL_MAP);
	}
	void JMaterial::SetHeightMap(JTexture* texture) noexcept
	{
		JTexture* be = heightMap;
		JTexture* af = texture;

		if (heightMap != nullptr && IsActivated())
			heightMap->OffReference();
		heightMap = texture;
		if (heightMap != nullptr && IsActivated())
			heightMap->OnReference();
		StuffFrameDirty();

		TextureChange(be, af, SHADER_FUNCTION_HEIGHT_MAP);
	}
	void JMaterial::SetRoughnessMap(JTexture* texture) noexcept
	{
		JTexture* be = roughnessMap;
		JTexture* af = texture;

		if (roughnessMap != nullptr && IsActivated())
			roughnessMap->OffReference();
		roughnessMap = texture;
		if (IsActivated())
			roughnessMap->OnReference();
		StuffFrameDirty();

		TextureChange(be, af, SHADER_FUNCTION_ROUGHNESS_MAP);
	}
	void JMaterial::SetAmbientOcclusionMap(JTexture* texture) noexcept
	{
		JTexture* be = ambientOcclusionMap;
		JTexture* af = texture;

		if (ambientOcclusionMap != nullptr && IsActivated())
			ambientOcclusionMap->OffReference();
		ambientOcclusionMap = texture;
		if (IsActivated())
			ambientOcclusionMap->OnReference();
		StuffFrameDirty();

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
	bool JMaterial::IsMaterialDirtied()const noexcept
	{
		return  gameObjectDirty > 0;
	}
	void JMaterial::DoActivate() noexcept
	{
		JResourceObject::DoActivate();
		shader->OnReference();
		if (HasAlbedoMapTexture())
			albedoMap->OnReference();
		if (HasNormalMapTexture())
			normalMap->OnReference();
		if (HasHeightMapTexture())
			heightMap->OnReference();
		if (HasRoughnessMapTexture())
			roughnessMap->OnReference();
		if (HasAmbientOcclusionMapTexture())
			ambientOcclusionMap->OnReference();
	}
	void JMaterial::DoDeActivate()noexcept
	{
		JResourceObject::DoDeActivate();
		shader->OffReference();
		if (HasAlbedoMapTexture())
			albedoMap->OffReference();
		if (HasNormalMapTexture())
			normalMap->OffReference();
		if (HasHeightMapTexture())
			heightMap->OffReference();
		if (HasRoughnessMapTexture())
			roughnessMap->OffReference();
		if (HasAmbientOcclusionMapTexture())
			ambientOcclusionMap->OffReference();
	}
	void JMaterial::StuffFrameDirty()noexcept
	{
		gameObjectDirty = Graphic::gNumFrameResources; 
	}
	void JMaterial::MinusMaterialformDirty()noexcept
	{
		--gameObjectDirty;
		if (gameObjectDirty < 0)
			gameObjectDirty = 0;
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
		if (shader->GetShaderFunctionFlag() != newFunc)
			SetShader(JRFI<JShader>::Create(*JResourceManager::Instance().GetDirectory(JApplicationVariable::GetProjectShaderMetafilePath()), newFunc));
	}
	JShader* JMaterial::GetShader()const noexcept
	{
		return shader;
	}
	void JMaterial::SetShader(JShader* newShader)noexcept
	{
		if (shader != nullptr)
			shader->OffReference();

		if (shader != nullptr && shader->GetReferenceCount() == 0 && (shader->GetFlag() & JOBJECT_FLAG::OBJECT_FLAG_INERASABLE) == 0)
			JResourceManager::Instance().EraseResource(shader);

		shader = newShader;

		if (shader != nullptr)
			shader->OnReference();
	}
	void JMaterial::SetMatCBIndex(uint value) noexcept
	{
		matCBIndex = value;
		StuffFrameDirty();
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
	void JMaterial::UpdateMaterial(Graphic::JMaterialConstants& matData)noexcept
	{
		DirectX::XMFLOAT4X4 matTransformFloat4x4 = matTransform;
		DirectX::XMMATRIX matTransform = XMLoadFloat4x4(&matTransformFloat4x4);

		matData.AlbedoColor = albedoColor;
		matData.Metalic = metallic;
		matData.Roughness = roughness;
		XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
		if (albedoMap != nullptr)
			matData.AlbedoMapIndex = albedoMap->GetTextureVectorIndex();
		if (normalMap != nullptr)
			matData.NormalMapIndex = normalMap->GetTextureVectorIndex();
		if (heightMap != nullptr)
			matData.HeightMapIndex = heightMap->GetTextureVectorIndex();
		if (roughnessMap != nullptr)
			matData.RoughnessMapIndex = roughnessMap->GetTextureVectorIndex();
		if (ambientOcclusionMap != nullptr)
			matData.AmbientOcclusionMapIndex = ambientOcclusionMap->GetTextureVectorIndex();
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

			//JResourceManager::Instance().GetResouirc
			JMaterial* newMaterial;
			if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
				newMaterial = new JMaterial(pathData.name, metadata.guid, (JOBJECT_FLAG)metadata.flag, directory, GetFormatIndex<JMaterial>(pathData.format));
			else
				newMaterial = new JMaterial(pathData.name, Core::MakeGuid(), JOBJECT_FLAG::OBJECT_FLAG_NONE, directory, GetFormatIndex<JMaterial>(pathData.format));

			newMaterial->SetMatCBIndex(JResourceManager::Instance().GetResourceCount<JMaterial>());
			if (hasAlbedo)
				newMaterial->albedoMap = JResourceManager::Instance().GetResource<JTexture>(albedoGuid);
			if (hasNormal)
				newMaterial->normalMap = JResourceManager::Instance().GetResource<JTexture>(normalGuid);
			if (hasHeight)
				newMaterial->heightMap = JResourceManager::Instance().GetResource<JTexture>(heightGuid);
			if (hasRoughness)
				newMaterial->roughnessMap = JResourceManager::Instance().GetResource<JTexture>(roughnessGuid);
			if (hasAmbientOcclusion)
				newMaterial->ambientOcclusionMap = JResourceManager::Instance().GetResource<JTexture>(ambientOcclusionGuid);

			newMaterial->SetNewFunctionFlag(newMaterial->CalculateShaderFunc());
			stream.close();
			return newMaterial;
			/*JShader* sShader = JResourceManager::Instance().GetResource<JShader>(shaderGuid);
			if (sShader != nullptr)
				shader = sShader;*/
		}
		else
			return nullptr;
	}
	void JMaterial::RegisterFunc()
	{
		auto defaultC = [](JDirectory* owner) ->JResourceObject*
		{
			return new JMaterial(owner->MakeUniqueFileName(GetDefaultName<JMaterial>()),
				Core::MakeGuid(),
				OBJECT_FLAG_NONE,
				owner,
				JResourceObject::GetDefaultFormatIndex());
		};
		auto initC = [](const std::string& name, const size_t guid, const JOBJECT_FLAG objFlag, JDirectory* directory, const uint8 formatIndex)-> JResourceObject*
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
		 
		RegisterTypeInfo(RTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{J_RESOURCE_TYPE::SHADER}, false },
			RTypeUtil{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable });	 
	}
	JMaterial::JMaterial(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex)
		: JResourceObject(name, guid, flag, directory, formatIndex)
	{}
	JMaterial::~JMaterial()
	{}
}
