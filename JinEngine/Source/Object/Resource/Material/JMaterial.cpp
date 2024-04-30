#include"JMaterial.h"
#include"JMaterialPrivate.h"
#include"../JResourceObjectHint.h"
#include"../Shader/JShader.h"
#include"../Texture/JTexture.h"
#include"../JResourceManager.h"
#include"../JResourceObjectUserInterface.h" 
#include"../JResourceObjectEventDesc.h"
#include"../../Directory/JDirectory.h"
#include"../../JObjectFileIOHelper.h"
#include"../../../Core/Identity/JIdenCreator.h"
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Guid/JGuidCreator.h" 
#include"../../../Core/Math/JMathHelper.h" 
#include"../../../Application/JApplicationProject.h"
#include"../../../Graphic/JGraphicConstants.h"
#include"../../../Graphic/Frameresource/JMaterialConstants.h"
#include"../../../Graphic/Frameresource/JFrameUpdate.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include<fstream> 
namespace JinEngine
{
	namespace
	{
		using MaterialFrameUpdate = Graphic::JFrameUpdate<Graphic::JFrameUpdateInterfaceHolder1<
			Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL, Graphic::JMaterialConstants&>>,
			Graphic::JFrameDirty>;
	}
	namespace
	{
		static JMaterialPrivate mPrivate;
	}
			
	enum class MATERIAL_TEXTURE
	{
		ALBEDO,
		NORMAL,
		HEIGHT,
		METALLIC,
		ROUGHNESS,
		AMBIENT,
		COUNT
	};
	class JMaterial::JMaterialImpl : public Core::JTypeImplBase,
		public MaterialFrameUpdate,
		public JResourceObjectUserInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JMaterialImpl)
	public:
		using MaterialFrame = JFrameInterface1;
	public:
		JWeakPtr<JMaterial> thisPointer = nullptr;
	public:
		JUserPtr<JShader> shader = nullptr;
	public:
		REGISTER_PROPERTY_EX(metallic, GetMetallic, SetMetallic, GUI_SLIDER(0.0f, 1.0f))
		float metallic = Core::JMaterialParameter::InitMetalic();
		REGISTER_PROPERTY_EX(roughness, GetRoughness, SetRoughness, GUI_SLIDER(0.0f, 1.0f))
		float roughness = Core::JMaterialParameter::InitRoughness();
		REGISTER_PROPERTY_EX(specularFactor, GetSpecular, SetSpecular, GUI_SLIDER(0.0f, 1.0f))
		float specularFactor = Core::JMaterialParameter::InitSpecularFactor();
		REGISTER_PROPERTY_EX(albedoColor, GetAlbedoColor, SetAlbedoColor, GUI_COLOR_PICKER(true))
		JVector4<float> albedoColor = Core::JMaterialParameter::InitAlbedoColor();
		JMatrix4x4 matTransform = JMatrix4x4::Identity();
	public:
		//Texture
		REGISTER_PROPERTY_EX(albedoMap, GetAlbedoMap, SetAlbedoMap, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, false, true))
		JUserPtr<JTexture> albedoMap;
		REGISTER_PROPERTY_EX(normalMap, GetNormalMap, SetNormalMap, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, false, true))
		JUserPtr<JTexture> normalMap;
		REGISTER_PROPERTY_EX(heightMap, GetHeightMap, SetHeightMap, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, false, true))
		JUserPtr<JTexture> heightMap;
		REGISTER_PROPERTY_EX(metallicMap, GetMetallicMap, SetMetallicMap, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, false, true))
		JUserPtr<JTexture>metallicMap;
		REGISTER_PROPERTY_EX(roughnessMap, GetRoughnessMap, SetRoughnessMap, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, false, true))
		JUserPtr<JTexture>roughnessMap;
		REGISTER_PROPERTY_EX(ambientOcclusionMap, GetAmbientOcclusionMap, SetAmbientOcclusionMap, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, false, true))
		JUserPtr<JTexture> ambientOcclusionMap;
		REGISTER_PROPERTY_EX(specularMap, GetSpecularMap, SetSpecularMap, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, false, true))
		JUserPtr<JTexture> specularMap;
	public:
		//Shader function option
		//수정필요
		//isDebug
		REGISTER_PROPERTY_EX(shadow, OnShadow, SetShadow, GUI_CHECKBOX())
		bool shadow = false;
		REGISTER_PROPERTY_EX(light, OnLight, SetLight, GUI_CHECKBOX())
		bool light = false;
		REGISTER_PROPERTY_EX(albedoMapOnly, OnAlbedoOnly, SetAlbedoMapOnly, GUI_CHECKBOX())
		bool albedoMapOnly = false; 
		REGISTER_PROPERTY_EX(skyMaterial, IsSkyMaterial, SetSkyMaterial, GUI_CHECKBOX())
		bool skyMaterial = false;
		bool isDebugMaterial = false;
		bool alphaClip = false;
	public:
		bool canUpdateShader = true;		//데이터 로드할때 마지막에 쉐이더 업데이트하기 위한 용도
	public:
		JGraphicShaderCondition shaderCond;  
	public:
		JMaterialImpl(const InitData& initData, JMaterial* thisMatRaw)
		{ 
		}
		~JMaterialImpl()
		{ 
		}
	public:
		float GetMetallic() const noexcept
		{
			return metallic;
		}
		float GetRoughness() const noexcept
		{
			return roughness;
		}
		float GetSpecular() const noexcept
		{
			return specularFactor;
		}
		JVector4<float> GetAlbedoColor() const noexcept
		{
			return albedoColor;
		}
		JMatrix4x4 GetMatTransform()const  noexcept
		{
			return matTransform;
		}
		JUserPtr<JTexture> GetAlbedoMap() const noexcept
		{
			return albedoMap;
		}
		JUserPtr<JTexture> GetNormalMap() const noexcept
		{
			return normalMap;
		}
		JUserPtr<JTexture> GetHeightMap() const noexcept
		{
			return heightMap;
		}
		JUserPtr<JTexture> GetMetallicMap() const noexcept
		{
			return metallicMap;
		}
		JUserPtr<JTexture> GetRoughnessMap() const noexcept
		{
			return roughnessMap;
		}
		JUserPtr<JTexture> GetAmbientOcclusionMap() const noexcept
		{
			return ambientOcclusionMap;
		}
		JUserPtr<JTexture> GetSpecularMap() const noexcept
		{
			return specularMap;
		}
		J_GRAPHIC_SHADER_FUNCTION GetShaderGFunctionFlag()const noexcept
		{ 
			J_GRAPHIC_SHADER_FUNCTION gFunction = SHADER_FUNCTION_NONE;
			if (albedoMap != nullptr) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_ALBEDO_MAP);
			if (specularMap != nullptr) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_SPECULAR_MAP);
			if (normalMap != nullptr) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_NORMAL_MAP);
			if (heightMap != nullptr) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_HEIGHT_MAP);
			if (metallicMap != nullptr) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_METALLIC_MAP);
			if (roughnessMap != nullptr) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_ROUGHNESS_MAP);
			if (ambientOcclusionMap != nullptr) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_AMBIENT_OCCLUSION_MAP);
			if (shadow) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_SHADOW);
			if (light) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_LIGHT);
			if (albedoMapOnly) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_ALBEDO_MAP_ONLY);
			if (skyMaterial) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_SKY);
			if (isDebugMaterial) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_DEBUG);
			if (alphaClip) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_ALPHA_CLIP);
			return gFunction;
		}
		JGraphicShaderCondition GetShaderCondition()const noexcept
		{
			return shaderCond;
		}
		int TryGetResourceArrayIndex(const JUserPtr<JTexture>& texture, const int failReturn = invalidIndex)const noexcept
		{ 
			return texture != nullptr ? texture->GraphicResourceUserInterface().GetFirstResourceArrayIndex() : failReturn;
		}
	public:
		void SetMetallic(const float value) noexcept
		{
			metallic = std::clamp(value, 0.0f, 1.0f);
			SetFrameDirty();
		}
		void SetRoughness(const float value) noexcept
		{
			roughness = std::clamp(value, 0.0f, 1.0f);
			SetFrameDirty();
		}
		void SetSpecular(const float value) noexcept
		{
			specularFactor = std::clamp(value, 0.0f, 1.0f);
			SetFrameDirty();
		}
		void SetAlbedoColor(const JVector4<float>& value)noexcept
		{
			albedoColor = value;
			SetFrameDirty();
		}
		void SetMatTransform(const JMatrix4x4& value)noexcept
		{
			matTransform = value;
			SetFrameDirty();
		}
		void SetAlbedoMap(JUserPtr<JTexture> newTexture) noexcept
		{ 
			SetTexture(albedoMap, newTexture, SHADER_FUNCTION_ALBEDO_MAP);
		}
		void SetSpecularMap(JUserPtr<JTexture> newTexture) noexcept
		{
			SetTexture(specularMap, newTexture, SHADER_FUNCTION_SPECULAR_MAP);
		}
		void SetNormalMap(JUserPtr<JTexture> newTexture) noexcept
		{
			SetTexture(normalMap, newTexture, SHADER_FUNCTION_NORMAL_MAP);
		}
		void SetHeightMap(JUserPtr<JTexture> newTexture) noexcept
		{
			SetTexture(heightMap, newTexture, SHADER_FUNCTION_HEIGHT_MAP);
		}
		void SetMetallicMap(JUserPtr<JTexture> newTexture) noexcept
		{
			SetTexture(metallicMap, newTexture, SHADER_FUNCTION_METALLIC_MAP);
		}
		void SetRoughnessMap(JUserPtr<JTexture> newTexture) noexcept
		{
			SetTexture(roughnessMap, newTexture, SHADER_FUNCTION_ROUGHNESS_MAP);
		}
		void SetAmbientOcclusionMap(JUserPtr<JTexture> newTexture) noexcept
		{
			SetTexture(ambientOcclusionMap, newTexture, SHADER_FUNCTION_AMBIENT_OCCLUSION_MAP);
		} 
		void SetTexture(JUserPtr<JTexture>& existingTexture,
			JUserPtr<JTexture> newTexture,
			const J_GRAPHIC_SHADER_FUNCTION shaderFunction)
		{
			if (thisPointer->IsActivated())
				CallOffResourceReference(existingTexture.Get());
			existingTexture = newTexture;
			if (thisPointer->IsActivated())
				CallOnResourceReference(existingTexture.Get());

			SetFrameDirty();	
			SetNewFunctionFlag(GetShaderGFunctionFlag()); 
		}
		void SetShadow(const bool value)noexcept
		{
			if (shadow == value)
				return;

			shadow = value;
			SetFrameDirty();
			SetNewFunctionFlag(GetShaderGFunctionFlag());
		}
		void SetLight(const bool value)noexcept
		{
			if (light == value)
				return;

			light = value;
			SetFrameDirty();
			SetNewFunctionFlag(GetShaderGFunctionFlag());
		}
		void SetAlbedoMapOnly(const bool value)noexcept
		{
			if (albedoMapOnly == value)
				return;

			albedoMapOnly = value;
			SetFrameDirty();
			SetNewFunctionFlag(GetShaderGFunctionFlag());
		}	 
		void SetSkyMaterial(const bool value)noexcept
		{
			if (skyMaterial == value)
				return;

			skyMaterial = value;
			SetNonCulling(true);
			SetDepthCompareFunc(J_SHADER_DEPTH_COMPARISON_FUNC::LESS_EQUAL);

			SetFrameDirty();
			SetNewFunctionFlag(GetShaderGFunctionFlag());
		}
		void SetDebugMaterial(const bool value)noexcept
		{
			if (isDebugMaterial == value)
				return;

			isDebugMaterial = value;
			SetFrameDirty();
			SetNewFunctionFlag(GetShaderGFunctionFlag());
		}
		void SetAlphaClip(const bool value)noexcept
		{
			if (alphaClip == value)
				return;

			alphaClip = value;
			SetFrameDirty();
			SetNewFunctionFlag(GetShaderGFunctionFlag());
		}
		void SetNonCulling(const bool value)noexcept
		{
			if (shaderCond.isCullModeNone == value)
				return;

			shaderCond.isCullModeNone = value;
			if (shaderCond.isCullModeNone)
				shaderCond.cullModeCondition = J_SHADER_APPLIY_CONDITION::APPLY;
			else
				shaderCond.cullModeCondition = J_SHADER_APPLIY_CONDITION::NOT;
			shaderCond.isCullModeNone = value;
			SetFrameDirty(); 
			SetNewOption(GetShaderCondition());
		}
		void SetPrimitiveType(const J_SHADER_PRIMITIVE_TYPE value)noexcept
		{
			if (shaderCond.primitiveType == value)
				return;

			shaderCond.primitiveType = value;
			if (shaderCond.primitiveType != J_SHADER_PRIMITIVE_TYPE::TRIANGLE)
				shaderCond.primitiveCondition = J_SHADER_APPLIY_CONDITION::APPLY;
			else
				shaderCond.primitiveCondition = J_SHADER_APPLIY_CONDITION::NOT;
			SetFrameDirty();
			SetNewOption(GetShaderCondition());
		}
		void SetDepthCompareFunc(const J_SHADER_DEPTH_COMPARISON_FUNC value)noexcept
		{
			if (shaderCond.depthCompareFunc == value)
				return;

			shaderCond.depthCompareFunc = value;
			if (shaderCond.depthCompareFunc != J_SHADER_DEPTH_COMPARISON_FUNC::DEFAULT)
				shaderCond.depthCompareCondition = J_SHADER_APPLIY_CONDITION::APPLY;
			else
				shaderCond.depthCompareCondition = J_SHADER_APPLIY_CONDITION::NOT;
			SetFrameDirty(); 
			SetNewOption(GetShaderCondition());
		}
		void SetNewFunctionFlag(const J_GRAPHIC_SHADER_FUNCTION newFunc)
		{
			if (!canUpdateShader)
				return;

			if (shader != nullptr && shader->GetShaderGFunctionFlag() == newFunc)
				return;

			JGraphicShaderCondition subPos;
			if (shader != nullptr)
				subPos = shader->GetShaderCondition(); 

			JUserPtr<JShader> newShader = JShader::FindShader(newFunc, subPos);
			if (newShader == nullptr)
				newShader = JICI::Create<JShader>(OBJECT_FLAG_NONE, newFunc, subPos);
			 
			SetShader(newShader);
		}
		void SetNewOption(const JGraphicShaderCondition newPso)
		{
			if (!canUpdateShader)
				return;

			if (shader != nullptr && shader->GetShaderCondition() == newPso)
				return;

			JUserPtr<JShader> newShader = nullptr;
			if (shader != nullptr)
			{
				newShader = JShader::FindShader(shader->GetShaderGFunctionFlag(), newPso);
				if (newShader == nullptr)
					newShader = JICI::Create<JShader>(OBJECT_FLAG_NONE, shader->GetShaderGFunctionFlag(), newPso);
			}
			else
				newShader = JICI::Create<JShader>(OBJECT_FLAG_NONE, SHADER_FUNCTION_NONE, newPso);
			 
			SetShader(newShader);
		}
		void SetShader(JUserPtr<JShader> newShader)noexcept
		{
			CallOffResourceReference(shader.Get());
			JShader* preShader = shader.Get();
			shader = newShader;
			CallOnResourceReference(shader.Get());
			 
			//if (preShader != nullptr && CallGetResourceReferenceCount(preShader) == 0)
			//	BeginDestroy(preShader);
		}
	public:
		bool OnShadow()const noexcept
		{
			return shadow;
		}
		bool OnLight()const noexcept
		{
			return light;
		}
		bool OnAlbedoOnly()const noexcept
		{
			return albedoMapOnly;
		}
		bool OnNonCulling()const noexcept
		{
			return shaderCond.isCullModeNone;
		}
		bool IsSkyMaterial()const noexcept
		{
			return skyMaterial;
		}
		bool IsDebugMaterial()const noexcept
		{
			return isDebugMaterial;
		}
		bool HasAlbedoMapTexture() const noexcept
		{
			return albedoMap.IsValid();
		}
		bool HasSpecularMapTexture() const noexcept
		{
			return specularMap.IsValid();
		}
		bool HasNormalMapTexture() const noexcept
		{
			return normalMap.IsValid();
		}
		bool HasHeightMapTexture() const noexcept
		{
			return heightMap.IsValid();
		}
		bool HasMetallicMapTexture() const noexcept
		{
			return metallicMap.IsValid();
		}
		bool HasRoughnessMapTexture() const noexcept
		{
			return roughnessMap.IsValid();
		}
		bool HasAmbientOcclusionMapTexture() const noexcept
		{
			return ambientOcclusionMap.IsValid();
		}
	public:
		void TryUpdateShader()	//for lazy update
		{
			auto overlapped = JShader::FindShader(GetShaderGFunctionFlag(), GetShaderCondition());
			if (overlapped.IsValid())
				SetShader(overlapped);
			else
				SetShader(JICI::Create<JShader>(OBJECT_FLAG_NONE, GetShaderGFunctionFlag(), GetShaderCondition()));
		}
		void PopTexture(JTexture* texture)noexcept
		{
			const size_t tarGuid = texture->GetGuid();
			if (HasAlbedoMapTexture() && albedoMap->GetGuid() == tarGuid)
				SetAlbedoMap(JUserPtr<JTexture>{});
			if (HasSpecularMapTexture() && specularMap->GetGuid() == tarGuid)
				SetSpecularMap(JUserPtr<JTexture>{});
			if (HasNormalMapTexture() && normalMap->GetGuid() == tarGuid)
				SetNormalMap(JUserPtr<JTexture>{});
			if (HasHeightMapTexture() && heightMap->GetGuid() == tarGuid)
				SetHeightMap(JUserPtr<JTexture>{});
			if (HasMetallicMapTexture() && metallicMap->GetGuid() == tarGuid)
				SetMetallicMap(JUserPtr<JTexture>{});
			if (HasRoughnessMapTexture() && roughnessMap->GetGuid() == tarGuid)
				SetRoughnessMap(JUserPtr<JTexture>{});
			if (HasAmbientOcclusionMapTexture() && ambientOcclusionMap->GetGuid() == tarGuid)
				SetAmbientOcclusionMap(JUserPtr<JTexture>{});
		}
	public:
		void OnResourceRef()
		{
			CallOnResourceReference(albedoMap.Get());
			CallOnResourceReference(normalMap.Get());
			CallOnResourceReference(heightMap.Get());
			CallOnResourceReference(metallicMap.Get());
			CallOnResourceReference(roughnessMap.Get());
			CallOnResourceReference(ambientOcclusionMap.Get());
		}
		void OffResourceRef()
		{
			CallOffResourceReference(albedoMap.Get());
			CallOffResourceReference(normalMap.Get());
			CallOffResourceReference(heightMap.Get());
			CallOffResourceReference(metallicMap.Get());
			CallOffResourceReference(roughnessMap.Get());
			CallOffResourceReference(ambientOcclusionMap.Get());
		}
	public:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj, JResourceEventDesc* desc)
		{
			if (iden == thisPointer->GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				const size_t objGuid = jRobj->GetGuid(); 
				if (jRobj->GetResourceType() == J_RESOURCE_TYPE::TEXTURE)
				{
					JTexture* texture = static_cast<JTexture*>(jRobj);
					const int arrayIndex = texture->GraphicResourceUserInterface().GetFirstResourceArrayIndex();

					PopTexture(texture);
					if (!IsFrameDirted() && (TryGetResourceArrayIndex(albedoMap) >= arrayIndex ||
						TryGetResourceArrayIndex(normalMap) >= arrayIndex ||
						TryGetResourceArrayIndex(heightMap) >= arrayIndex ||
						TryGetResourceArrayIndex(metallicMap) >= arrayIndex ||
						TryGetResourceArrayIndex(roughnessMap) >= arrayIndex ||
						TryGetResourceArrayIndex(ambientOcclusionMap) >= arrayIndex ||
						TryGetResourceArrayIndex(specularMap) >= arrayIndex))
						SetFrameDirty();
				}
				else if (shader != nullptr && shader->GetGuid() == objGuid)
					SetShader(nullptr);
			}
			else if (eventType == J_RESOURCE_EVENT_TYPE::UPDATE_NON_FRAME_RESOURCE &&
				jRobj->GetResourceType() == J_RESOURCE_TYPE::TEXTURE &&
				desc != nullptr)
			{ 
				JResourceUpdateEvDesc* evDesc = static_cast<JResourceUpdateEvDesc*>(desc);			 
				switch (evDesc->action)
				{
				case JinEngine::JResourceUpdateEvDesc::USER_ACTION::UPDATE_USER_ONLY:
				{
					SetFrameDirty();
					break;
				}
				case JinEngine::JResourceUpdateEvDesc::USER_ACTION::UPDATE_USER_AND_REAR_OF_FRAME_BUFFER:
				{
					const int frameIndex = GetFrameIndex();
					auto rawVec = thisPointer->GetTypeInfo().GetInstanceRawPtrVec();
					for (const auto& data : rawVec)
					{
						if (static_cast<JMaterial*>(data)->impl->GetFrameIndex() > frameIndex)
							static_cast<JMaterial*>(data)->impl->SetFrameDirty();
					}
					break;
				}
				default:
					break;
				}
			}
		}
	public:
		void UpdateFrame(Graphic::JMaterialConstants& constant)noexcept final
		{    
			static constexpr uint missingIndex = Graphic::Constants::missingIndex;

			constant.albedoColor = albedoColor;
			constant.metallic = metallic;
			constant.roughness = roughness;
			constant.specularFactor = specularFactor;
			constant.matTransform.StoreXM(XMMatrixTranspose(matTransform.LoadXM()));
			constant.albedoMapIndex = TryGetResourceArrayIndex(albedoMap, missingIndex);
			constant.normalMapIndex = TryGetResourceArrayIndex(normalMap, missingIndex);
			constant.heightMapIndex = TryGetResourceArrayIndex(heightMap, missingIndex);
			constant.metallicMapIndex = TryGetResourceArrayIndex(metallicMap, missingIndex);
			constant.roughnessMapIndex = TryGetResourceArrayIndex(roughnessMap, missingIndex);
			constant.ambientOcclusionMapIndex = TryGetResourceArrayIndex(ambientOcclusionMap, missingIndex);
			constant.specularMapIndex = TryGetResourceArrayIndex(specularMap, missingIndex);		 
			MaterialFrame::MinusMovedDirty();
		}
	public:
		bool ReadAssetData()
		{
			JFileIOTool tool;
			if (!tool.Begin(thisPointer->GetPath(), JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
				return false;

			bool sShadow = false;
			bool sLight = false;
			bool sAlbedoOnly = false; 
			bool sIsSkyMateral = false;
			bool sIsDebugMaterial = false;
			bool sAlphaclip = false;
			bool sNonCulling = false;
			J_SHADER_PRIMITIVE_TYPE sPrimitiveType = J_SHADER_PRIMITIVE_TYPE::TRIANGLE;
			J_SHADER_DEPTH_COMPARISON_FUNC sDepthComparesionFunc = J_SHADER_DEPTH_COMPARISON_FUNC::DEFAULT;
			float sMetallic = 0;
			float sRoughness = 0;
			float sSpecularFactor = 0;
			JVector4<float> sAlbedoColor;
			JMatrix4x4 sMatTransform;

			JObjectFileIOHelper::LoadAtomicData(tool, sShadow, "Shadow");
			JObjectFileIOHelper::LoadAtomicData(tool, sLight, "Light");
			JObjectFileIOHelper::LoadAtomicData(tool, sAlbedoOnly, "AlbedoOnly");
			JObjectFileIOHelper::LoadAtomicData(tool, sIsSkyMateral, "SkyMaterial");
			JObjectFileIOHelper::LoadAtomicData(tool, sIsDebugMaterial, "DebugMaterial");
			JObjectFileIOHelper::LoadAtomicData(tool, sAlphaclip, "AlphaClip");

			JObjectFileIOHelper::LoadAtomicData(tool, sNonCulling, "NonCulling");
			JObjectFileIOHelper::LoadEnumData(tool, sPrimitiveType, "PrimitiveType");
			JObjectFileIOHelper::LoadEnumData(tool, sDepthComparesionFunc, "DepthComparesionFunc");

			JObjectFileIOHelper::LoadAtomicData(tool, sMetallic, "Metallic");
			JObjectFileIOHelper::LoadAtomicData(tool, sRoughness, "Roughness");
			JObjectFileIOHelper::LoadAtomicData(tool, sSpecularFactor, "Specular");

			JObjectFileIOHelper::LoadVector4(tool, sAlbedoColor, "AlbedoColor");
			JObjectFileIOHelper::LoadMatrix4x4(tool, sMatTransform, "Matransform");
 
			JUserPtr<JTexture> sAlbedoMap = JObjectFileIOHelper::_LoadHasIden<JTexture>(tool, "AlbedoMap");
			JUserPtr<JTexture> sNormalMap = JObjectFileIOHelper::_LoadHasIden<JTexture>(tool, "NormalMap");
			JUserPtr<JTexture> sHeightMap = JObjectFileIOHelper::_LoadHasIden<JTexture>(tool, "HeightMap");
			JUserPtr<JTexture> sMetallicMap =  JObjectFileIOHelper::_LoadHasIden<JTexture>(tool, "MetallicMap");
			JUserPtr<JTexture> sRoughnessMap = JObjectFileIOHelper::_LoadHasIden<JTexture>(tool, "RoughnessMap");
			JUserPtr<JTexture> sAmbientOcclusionMap = JObjectFileIOHelper::_LoadHasIden<JTexture>(tool, "AmbientOcclusionMap");
			JUserPtr<JTexture> sSpecularMap = JObjectFileIOHelper::_LoadHasIden<JTexture>(tool, "SpecularMap");
			tool.Close();

			canUpdateShader = false;
			SetShadow(sShadow);
			SetLight(sLight);
			SetAlbedoMapOnly(sAlbedoOnly);
			SetNonCulling(sNonCulling); 
			SetSkyMaterial(sIsSkyMateral);
			SetDebugMaterial(sIsDebugMaterial);
			SetAlphaClip(sAlphaclip);

			SetPrimitiveType((J_SHADER_PRIMITIVE_TYPE)sPrimitiveType);
			SetDepthCompareFunc((J_SHADER_DEPTH_COMPARISON_FUNC)sDepthComparesionFunc);

			SetMetallic(sMetallic);
			SetRoughness(sRoughness);
			SetSpecular(sSpecularFactor);
			SetAlbedoColor(sAlbedoColor);
			SetMatTransform(sMatTransform);

			SetAlbedoMap(sAlbedoMap);
			SetNormalMap(sNormalMap);
			SetHeightMap(sHeightMap);
			SetMetallicMap(sMetallicMap);
			SetRoughnessMap(sRoughnessMap);
			SetAmbientOcclusionMap(sAmbientOcclusionMap);
			SetSpecularMap(sSpecularMap);
			canUpdateShader = true;
			 
			TryUpdateShader();
			return true;
		}
		bool WriteAssetData()
		{
			JFileIOTool tool;
			if (!tool.Begin(thisPointer->GetPath(), JFileIOTool::TYPE::JSON))
				return false;

			JObjectFileIOHelper::StoreAtomicData(tool, shadow, "Shadow");
			JObjectFileIOHelper::StoreAtomicData(tool, light, "Light");
			JObjectFileIOHelper::StoreAtomicData(tool, albedoMapOnly, "AlbedoOnly");
			JObjectFileIOHelper::StoreAtomicData(tool, skyMaterial, "SkyMaterial");
			JObjectFileIOHelper::StoreAtomicData(tool, isDebugMaterial, "DebugMaterial");
			JObjectFileIOHelper::StoreAtomicData(tool, alphaClip, "AlphaClip");

			JObjectFileIOHelper::StoreAtomicData(tool, shaderCond.isCullModeNone, "NonCulling");
			JObjectFileIOHelper::StoreEnumData(tool, shaderCond.primitiveType, "PrimitiveType");
			JObjectFileIOHelper::StoreEnumData(tool, shaderCond.depthCompareFunc, "DepthComparesionFunc");
			 
			JObjectFileIOHelper::StoreAtomicData(tool, metallic, "Metallic");
			JObjectFileIOHelper::StoreAtomicData(tool, roughness, "Roughness");
			JObjectFileIOHelper::StoreAtomicData(tool, specularFactor, "Specular");

			JObjectFileIOHelper::StoreVector4(tool, albedoColor, "AlbedoColor");
			JObjectFileIOHelper::StoreMatrix4x4(tool, matTransform, "Matransform");

			JObjectFileIOHelper::_StoreHasIden(tool, albedoMap.Get(), "AlbedoMap");
			JObjectFileIOHelper::_StoreHasIden(tool, normalMap.Get(), "NormalMap");
			JObjectFileIOHelper::_StoreHasIden(tool, heightMap.Get(), "HeightMap");
			JObjectFileIOHelper::_StoreHasIden(tool, metallicMap.Get(), "MetallicMap");
			JObjectFileIOHelper::_StoreHasIden(tool, roughnessMap.Get(), "RoughnessMap");
			JObjectFileIOHelper::_StoreHasIden(tool, ambientOcclusionMap.Get(), "AmbientOcclusionMap");
			JObjectFileIOHelper::_StoreHasIden(tool, specularMap.Get(), "SpecularMap");
			 
			tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
			return true;
		}
	public:
		void NotifyReAlloc()
		{
			MaterialFrame::ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL, (MaterialFrame*)this);
			ResetEventListenerPointer(*JResourceObject::EvInterface(), thisPointer->GetGuid());
		}
	public:
		void Initialize(InitData* initData)
		{
			SetNewFunctionFlag(SHADER_FUNCTION_NONE);
			SetFrameDirty();
		}
		void RegisterThisPointer(JMaterial* mat)
		{
			thisPointer = Core::GetWeakPtr(mat);
		}
		void RegisterPostCreation()
		{ 
			auto vec = {J_RESOURCE_EVENT_TYPE::UPDATE_NON_FRAME_RESOURCE , J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE };
			AddEventListener(*JResourceObject::EvInterface(), thisPointer->GetGuid(), vec);
		}
		void DeRegisterPreDestruction()
		{
			RemoveListener(*JResourceObject::EvInterface(), thisPointer->GetGuid());
		}
		void RegisterRItemFrameData()
		{
			//all material belong same area
			static constexpr size_t materialArea = 0;
			MaterialFrame::RegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL, (MaterialFrame*)this, materialArea);
		}
		void DeRegisterRItemFrameData()
		{
			MaterialFrame::DeRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::MATERIAL, (MaterialFrame*)this);
		}
		static void RegisterTypeData()
		{
			auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex(JMaterial::GetStaticResourceType(), format); };

			static GetRTypeInfoCallable getTypeInfoCallable{ &JMaterial::StaticTypeInfo };
			static GetAvailableFormatCallable getAvailableFormatCallable{ &JMaterial::GetAvailableFormat };
			static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

			static auto setFrameLam = [](JResourceObject* jRobj)
			{
				static_cast<JMaterial*>(jRobj)->impl->SetFrameDirty();
			};
			static SetRFrameDirtyCallable setFrameDirtyCallable{ setFrameLam }; 

			static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{J_RESOURCE_TYPE::SHADER}, false, true, true, false };
			static RTypeCommonFunc rTypeCFunc{ getTypeInfoCallable, getAvailableFormatCallable, getFormatIndexCallable };
			static RTypePrivateFunc rTypeiFunc{ &setFrameDirtyCallable};

			RegisterRTypeInfo(rTypeHint, rTypeCFunc, rTypeiFunc);
			Core::JIdentifier::RegisterPrivateInterface(JMaterial::StaticTypeInfo(), mPrivate);

			IMPL_REALLOC_BIND(JMaterial::JMaterialImpl, thisPointer)
		}
	};

	JMaterial::InitData::InitData(const JUserPtr<JDirectory>& directory)
		:JResourceObject::InitData(JMaterial::StaticTypeInfo(), GetDefaultFormatIndex(), GetStaticResourceType(), directory)
	{}

	JMaterial::InitData::InitData(const uint8 formatIndex, const JUserPtr<JDirectory>& directory)
		:JResourceObject::InitData(JMaterial::StaticTypeInfo(), formatIndex, GetStaticResourceType(), directory)
	{}
	JMaterial::InitData::InitData(const size_t guid, const uint8 formatIndex, const JUserPtr<JDirectory>& directory)
		: JResourceObject::InitData(JMaterial::StaticTypeInfo(), guid, formatIndex, GetStaticResourceType(), directory)
	{}
	JMaterial::InitData::InitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory)
		: JResourceObject::InitData(JMaterial::StaticTypeInfo(), name, guid, flag, formatIndex, GetStaticResourceType(), directory)
	{ }

	Core::JIdentifierPrivate& JMaterial::PrivateInterface()const noexcept
	{
		return mPrivate;
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
	JUserPtr<JShader> JMaterial::GetShader()const noexcept
	{
		return impl->shader;
	}
	JShader* JMaterial::GetRawShader()const noexcept
	{
		return impl->shader.Get();
	}
	float JMaterial::GetMetallic() const noexcept
	{
		return impl->GetMetallic();
	}
	float JMaterial::GetRoughness() const noexcept
	{
		return impl->GetRoughness();
	}
	float JMaterial::GetSpecularFactor() const noexcept
	{
		return impl->GetSpecular();
	}
	JVector4<float> JMaterial::GetAlbedoColor() const noexcept
	{
		return impl->GetAlbedoColor();
	}
	JMatrix4x4 JMaterial::GetMatTransform()const  noexcept
	{
		return impl->GetMatTransform();
	}
	JUserPtr<JTexture> JMaterial::GetAlbedoMap() const noexcept
	{
		return impl->GetAlbedoMap();
	}
	JUserPtr<JTexture> JMaterial::GetNormalMap() const noexcept
	{
		return impl->GetNormalMap();
	}
	JUserPtr<JTexture> JMaterial::GetHeightMap() const noexcept
	{
		return impl->GetHeightMap();
	}
	JUserPtr<JTexture> JMaterial::GetMetallicMap() const noexcept
	{
		return impl->GetMetallicMap();
	}
	JUserPtr<JTexture> JMaterial::GetRoughnessMap() const noexcept
	{
		return impl->GetRoughnessMap();
	}
	JUserPtr<JTexture> JMaterial::GetAmbientOcclusionMap() const noexcept
	{
		return impl->GetAmbientOcclusionMap();
	}
	JUserPtr<JTexture> JMaterial::GetSpecularMap() const noexcept
	{
		return impl->GetSpecularMap();
	}
	JGraphicShaderCondition JMaterial::GetShaderCondition()const noexcept
	{
		return impl->shaderCond;
	}
	J_SHADER_PRIMITIVE_TYPE JMaterial::GetPrimitiveType()const noexcept
	{
		return impl->shaderCond.primitiveType;
	}
	J_SHADER_DEPTH_COMPARISON_FUNC JMaterial::GetDepthCompasionFunc()const noexcept
	{
		return impl->shaderCond.depthCompareFunc;
	}
	void JMaterial::SetMetallic(const float value) noexcept
	{
		impl->SetMetallic(value);
	}
	void JMaterial::SetRoughness(const float value) noexcept
	{
		impl->SetRoughness(value);
	}
	void JMaterial::SetSpecularFactor(const float value) noexcept
	{
		impl->SetSpecular(value);
	}
	void JMaterial::SetAlbedoColor(const JVector4<float>& value)noexcept
	{
		impl->SetAlbedoColor(value);
	}
	void JMaterial::SetMatTransform(const JMatrix4x4& value)noexcept
	{
		impl->SetMatTransform(value);
	}
	void JMaterial::SetAlbedoMap(JUserPtr<JTexture> texture) noexcept
	{
		impl->SetAlbedoMap(texture);
	}
	void JMaterial::SetSpecularMap(JUserPtr<JTexture> texture) noexcept
	{
		impl->SetSpecularMap(texture);
	}
	void JMaterial::SetNormalMap(JUserPtr<JTexture> texture) noexcept
	{
		impl->SetNormalMap(texture);
	}
	void JMaterial::SetHeightMap(JUserPtr<JTexture> texture) noexcept
	{
		impl->SetHeightMap(texture);
	}
	void JMaterial::SetMetallicMap(JUserPtr<JTexture> texture) noexcept
	{
		impl->SetMetallicMap(texture);
	}
	void JMaterial::SetRoughnessMap(JUserPtr<JTexture> texture) noexcept
	{
		impl->SetRoughnessMap(texture);
	}
	void JMaterial::SetAmbientOcclusionMap(JUserPtr<JTexture> texture) noexcept
	{
		impl->SetAmbientOcclusionMap(texture);
	} 
	void JMaterial::SetShadow(const bool value)noexcept
	{
		impl->SetShadow(value);
	}
	void JMaterial::SetLight(const bool value)noexcept
	{
		impl->SetLight(value);
	}
	void JMaterial::SetAlbedoMapOnly(const bool value)noexcept
	{
		impl->SetAlbedoMapOnly(value);
	} 
	void JMaterial::SetSkyMaterial(const bool value)noexcept
	{
		impl->SetSkyMaterial(value);
	}
	void JMaterial::SetDebugMaterial(const bool value)noexcept
	{
		impl->SetDebugMaterial(value);
	}
	void JMaterial::SetAlphaClip(const bool value)noexcept
	{
		impl->SetAlphaClip(value);
	}
	void JMaterial::SetNonCulling(const bool value)noexcept
	{
		impl->SetNonCulling(value);
	}
	void JMaterial::SetPrimitiveType(const J_SHADER_PRIMITIVE_TYPE value)noexcept
	{
		impl->SetPrimitiveType(value);
	}
	void JMaterial::SetDepthCompareFunc(const J_SHADER_DEPTH_COMPARISON_FUNC value)noexcept
	{
		impl->SetDepthCompareFunc(value);
	}
	bool JMaterial::OnShadow()const noexcept
	{
		return impl->OnShadow();
	}
	bool JMaterial::OnLight()const noexcept
	{
		return impl->OnLight();
	}
	bool JMaterial::OnAlbedoOnly()const noexcept
	{
		return impl->OnAlbedoOnly();
	}
	bool JMaterial::OnNonCulling()const noexcept
	{
		return impl->OnNonCulling();
	} 
	bool JMaterial::IsFrameDirted()const noexcept
	{
		return impl->IsFrameDirted();
	}
	bool JMaterial::IsSkyMaterial()const noexcept
	{
		return impl->IsSkyMaterial();
	}
	bool JMaterial::IsDebugMaterial()const noexcept
	{
		return impl->IsDebugMaterial();
	}
	bool JMaterial::IsSame(const Core::JMaterialParameter& param)const noexcept
	{
		return impl->metallic == param.metallic &&
			impl->roughness == param.roughness &&
			impl->specularFactor == param.specularFactor &&
			impl->albedoColor == param.albedoColor;
	}
	bool JMaterial::HasAlbedoMapTexture() const noexcept
	{
		return impl->HasAlbedoMapTexture();
	}
	bool JMaterial::HasSpecularMapTexture() const noexcept
	{
		return impl->HasSpecularMapTexture();
	}
	bool JMaterial::HasNormalMapTexture() const noexcept
	{
		return impl->HasNormalMapTexture();
	}
	bool JMaterial::HasHeightMapTexture() const noexcept
	{
		return impl->HasHeightMapTexture();
	}
	bool JMaterial::HasRoughnessMapTexture() const noexcept
	{
		return impl->HasRoughnessMapTexture();
	}
	bool JMaterial::JMaterial::HasAmbientOcclusionMapTexture() const noexcept
	{
		return impl->HasAmbientOcclusionMapTexture();
	}
	void JMaterial::PopTexture(JTexture* texture)noexcept
	{
		impl->PopTexture(texture);
	}
	void JMaterial::DoActivate() noexcept
	{
		JResourceObject::DoActivate();
		SetValid(true);
		impl->SetFrameDirty(); 
		impl->OnResourceRef();
	}
	void JMaterial::DoDeActivate()noexcept
	{
		SetValid(false);
		impl->OffFrameDirty();
		impl->OffResourceRef();
		JResourceObject::DoDeActivate();
	}
	JMaterial::JMaterial(const InitData& initData)
		: JResourceObject(initData), impl(std::make_unique<JMaterialImpl>(initData, this))
	{		
	}
	JMaterial::~JMaterial()
	{ 
		impl.reset();
	}

	using CreateInstanceInterface = JMaterialPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JMaterialPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JMaterialPrivate::AssetDataIOInterface;
	using FrameUpdateInterface = JMaterialPrivate::FrameUpdateInterface;
	using FrameIndexInterface = JMaterialPrivate::FrameIndexInterface; 
	using UpdateShaderInterface = JMaterialPrivate::UpdateShaderInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JMaterial>(*static_cast<JMaterial::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JResourceObjectPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JMaterial* mat = static_cast<JMaterial*>(createdPtr);
		mat->impl->RegisterThisPointer(mat);
		mat->impl->RegisterPostCreation();
		mat->impl->RegisterRItemFrameData();
		mat->impl->Initialize(static_cast<JMaterial::InitData*>(initData));
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JMaterial::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{ 
		static_cast<JMaterial*>(ptr)->impl->DeRegisterPreDestruction();
		static_cast<JMaterial*>(ptr)->impl->DeRegisterRItemFrameData();
		JResourceObjectPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JMaterial::LoadData::StaticTypeInfo()))
			return nullptr;
 
		auto loadData = static_cast<JMaterial::LoadData*>(data);
		auto pathData = loadData->pathData;
		JUserPtr<JDirectory> directory = loadData->directory;

		auto initData = std::make_unique< JMaterial::InitData>(directory);	//for load metadata
		if (LoadMetaData(pathData.engineMetaFileWPath, initData.get()) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;

		JUserPtr<JMaterial> newMat = nullptr;
		if (directory->HasFile(initData->guid))
			newMat = Core::GetUserPtr<JMaterial>(JMaterial::StaticTypeInfo().TypeGuid(), initData->guid);

		if (newMat == nullptr)
		{
			initData->name = pathData.name;
			auto idenUser = mPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &mPrivate);
			newMat.ConnnectChild(idenUser);
		}
		newMat->impl->ReadAssetData();
		return newMat;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JMaterial::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JMaterial::StoreData*>(data);
		if (!storeData->HasCorrectType(JMaterial::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JUserPtr<JMaterial> mat;
		mat.ConnnectChild(storeData->obj);
		return mat->impl->WriteAssetData() ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JMaterial::InitData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JFileIOTool tool;
		if (!tool.Begin(path, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto loadMetaData = static_cast<JMaterial::InitData*>(data);
		if (LoadCommonMetaData(tool, loadMetaData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		tool.Close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JMaterial::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JMaterial::StoreData*>(data);
		JUserPtr<JMaterial> mat;
		mat.ConnnectChild(storeData->obj);

		JFileIOTool tool;
		if (!tool.Begin(mat->GetMetaFilePath(), JFileIOTool::TYPE::JSON))
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (StoreCommonMetaData(tool, storeData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	bool FrameUpdateInterface::UpdateStart(JMaterial* mat, const bool isUpdateForced)noexcept
	{
		if (isUpdateForced)
			mat->impl->SetFrameDirty();

		mat->impl->SetLastFrameUpdatedTrigger(false);
		mat->impl->SetLastFrameHotUpdatedTrigger(false);
		return mat->impl->IsFrameDirted();
	}
	void FrameUpdateInterface::UpdateFrame(JMaterial* mat, Graphic::JMaterialConstants& constants)noexcept
	{
		mat->impl->UpdateFrame(constants);
	}
	void FrameUpdateInterface::UpdateEnd(JMaterial* mat)noexcept
	{
		if (mat->impl->IsFrameHotDirted())
			mat->impl->SetLastFrameHotUpdatedTrigger(true);
		mat->impl->SetLastFrameUpdatedTrigger(true);
		mat->impl->UpdateFrameEnd();
	}
	uint FrameUpdateInterface::GetMaterialFrameIndex(JMaterial* mat)noexcept
	{
		return mat->impl->MaterialFrame::GetFrameIndex();
	}
	bool FrameUpdateInterface::IsLastFrameUpdated(JMaterial* mat)
	{
		return mat->impl->IsLastFrameUpdated();
	}
	bool FrameUpdateInterface::HasRecopyRequest(JMaterial* mat)noexcept
	{
		return mat->impl->MaterialFrame::HasMovedDirty();
	}

	uint FrameIndexInterface::GetMaterialFrameIndex(JMaterial* mat)noexcept
	{
		return mat->impl->MaterialFrame::GetFrameIndex();
	}
 
	void UpdateShaderInterface::OnUpdateShaderTrigger(const JUserPtr<JMaterial>& mat)noexcept
	{
		mat->impl->canUpdateShader = true;
	}
	void UpdateShaderInterface::OffUpdateShaderTrigger(const JUserPtr<JMaterial>& mat)noexcept
	{
		mat->impl->canUpdateShader = false;
	}
	void UpdateShaderInterface::UpdateShader(const JUserPtr<JMaterial>& mat)noexcept
	{ 
		mat->impl->TryUpdateShader();
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JMaterialPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JMaterialPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
	JResourceObjectPrivate::AssetDataIOInterface& JMaterialPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}
