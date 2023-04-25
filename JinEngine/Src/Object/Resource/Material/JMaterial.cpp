#include"JMaterial.h"
#include"JMaterialPrivate.h"
#include"../JResourceObjectHint.h"
#include"../Shader/JShader.h"
#include"../Texture/JTexture.h"
#include"../JResourceManager.h"
#include"../JResourceObjectUserInterface.h" 
#include"../../JFrameUpdate.h"
#include"../../Directory/JDirectory.h"
#include"../../../Core/Identity/JIdenCreator.h"
#include"../../../Core/Identity/JIdentifierImplBase.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/File/JFileIOHelper.h" 
#include"../../../Application/JApplicationProject.h"
#include"../../../Graphic/JGraphicConstants.h"
#include"../../../Graphic/FrameResource/JMaterialConstants.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../../Utility/JMathHelper.h"
#include<fstream>

namespace JinEngine
{
	namespace
	{
		static JMaterialPrivate mPrivate;
	}
	
	class JMaterial::JMaterialImpl : Core::JIdentifierImplBase,
		public JFrameUpdate<JFrameUpdate1<JFrameUpdateBase<Graphic::JMaterialConstants&>>, JFrameDirty, FrameUpdate::singleBuff>,
		public JResourceObjectUserInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JMaterialImpl)
	public:
		JMaterial* thisMat = nullptr;
	public:
		Core::JUserPtr<JShader> shader = nullptr;
	public:
		REGISTER_PROPERTY_EX(metallic, GetMetallic, SetMetallic, GUI_SLIDER(0.0f, 1.0f, false, false))
		float metallic = 0;
		REGISTER_PROPERTY_EX(roughness, GetRoughness, SetRoughness, GUI_SLIDER(0.0f, 1.0f))
		float roughness = 0;
		REGISTER_PROPERTY_EX(albedoColor, GetAlbedoColor, SetAlbedoColor, GUI_COLOR_PICKER(true))
		DirectX::XMFLOAT4 albedoColor = { 0.75f, 0.75f, 0.75f, 0.65f };
		DirectX::XMFLOAT4X4 matTransform = JMathHelper::Identity4x4();
	public:
		//Texture
		REGISTER_PROPERTY_EX(albedoMap, GetAlbedoMap, SetAlbedoMap, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, true))
		Core::JUserPtr<JTexture> albedoMap;
		REGISTER_PROPERTY_EX(normalMap, GetNormalMap, SetNormalMap, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, true))
		Core::JUserPtr<JTexture> normalMap;
		REGISTER_PROPERTY_EX(heightMap, GetHeightMap, SetHeightMap, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, true))
		Core::JUserPtr<JTexture> heightMap;
		REGISTER_PROPERTY_EX(roughnessMap, GetRoughnessMap, SetRoughnessMap, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, true))
		Core::JUserPtr<JTexture>roughnessMap;
		REGISTER_PROPERTY_EX(ambientOcclusionMap, GetAmbientOcclusionMap, SetAmbientOcclusionMap, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::IMAGE, true))
		Core::JUserPtr<JTexture> ambientOcclusionMap;
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
		bool nonCulling = false;
		//Draw ShadowMap by depth test
		bool isShadowMapWrite = false;
		//Draw bounding object by depth test
		bool isBoundingObjDepthTest = false;
		bool isSkyMateral = false;
		bool isDebugMaterial = false;
		bool alphaClip = false;
	public:
		J_SHADER_PRIMITIVE_TYPE primitiveType = J_SHADER_PRIMITIVE_TYPE::DEFAULT;
		J_SHADER_DEPTH_COMPARISON_FUNC depthComparesionFunc = J_SHADER_DEPTH_COMPARISON_FUNC::DEFAULT;
	public:
		bool canUpdateShader = true;		//데이터 로드할때 마지막에 쉐이더 업데이트하기 위한 용도
	public:
		JMaterialImpl(const InitData& initData, JMaterial* thisMat)
			:JResourceObjectUserInterface(thisMat->GetGuid()), thisMat(thisMat)
		{ 
			AddEventListener(*JResourceObject::EvInterface(), thisMat->GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		}
		~JMaterialImpl()
		{
			RemoveListener(*JResourceObject::EvInterface(), thisMat->GetGuid());
		}
	public:
		void Initialize(const InitData& initData)
		{
			SetNewFunctionFlag(SHADER_FUNCTION_NONE);
			SetFrameDirty();
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
		DirectX::XMFLOAT4 GetAlbedoColor() const noexcept
		{
			return albedoColor;
		}
		DirectX::XMFLOAT4X4 GetMatTransform()const  noexcept
		{
			return matTransform;
		}
		Core::JUserPtr<JTexture> GetAlbedoMap() const noexcept
		{
			return albedoMap;
		}
		Core::JUserPtr<JTexture> GetNormalMap() const noexcept
		{
			return normalMap;
		}
		Core::JUserPtr<JTexture> GetHeightMap() const noexcept
		{
			return heightMap;
		}
		Core::JUserPtr<JTexture> GetRoughnessMap() const noexcept
		{
			return roughnessMap;
		}
		Core::JUserPtr<JTexture> GetAmbientOcclusionMap() const noexcept
		{
			return ambientOcclusionMap;
		}
		J_GRAPHIC_SHADER_FUNCTION GetShaderGFunctionFlag()const noexcept
		{ 
			J_GRAPHIC_SHADER_FUNCTION gFunction = SHADER_FUNCTION_NONE;
			if (albedoMap != nullptr) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_ALBEDO_MAP);
			if (normalMap != nullptr) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_NORMAL_MAP);
			if (heightMap != nullptr) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_HEIGHT_MAP);
			if (roughnessMap != nullptr) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_ROUGHNESS_MAP);
			if (ambientOcclusionMap != nullptr) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_AMBIENT_OCCLUSION_MAP);
			if (shadow) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_SHADOW);
			if (light) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_LIGHT);
			if (albedoMapOnly) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_ALBEDO_MAP_ONLY);
			if (isShadowMapWrite) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_WRITE_SHADOWMAP);
			if (isBoundingObjDepthTest) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_DEPTH_TEST_BOUNDING_OBJECT);
			if (isSkyMateral) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_SKY);
			if (isDebugMaterial) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_DEBUG);
			if (alphaClip) gFunction = Core::AddSQValueEnum(gFunction, SHADER_FUNCTION_ALPHA_CLIP);
			return gFunction;
		}
		JShaderGraphicPsoCondition GetSubGraphicPso()const noexcept
		{
			JShaderGraphicPsoCondition psoCondition;
			if (nonCulling)
			{
				psoCondition.cullModeCondition = J_SHADER_PSO_APPLIY_CONDITION::APPLY_J_PSO;
				psoCondition.isCullModeNone = true;
			}
			else
			{
				psoCondition.cullModeCondition = J_SHADER_PSO_APPLIY_CONDITION::SAME_AS;
				psoCondition.isCullModeNone = false;
			}
			if (primitiveType != J_SHADER_PRIMITIVE_TYPE::DEFAULT)
			{
				psoCondition.primitiveCondition = J_SHADER_PSO_APPLIY_CONDITION::APPLY_J_PSO;
				psoCondition.primitiveType = primitiveType;
			}
			else
				psoCondition.primitiveCondition = J_SHADER_PSO_APPLIY_CONDITION::SAME_AS;

			if (depthComparesionFunc != J_SHADER_DEPTH_COMPARISON_FUNC::DEFAULT)
			{
				psoCondition.depthCompareCondition = J_SHADER_PSO_APPLIY_CONDITION::APPLY_J_PSO;
				psoCondition.depthCompareFunc = depthComparesionFunc;
			}
			else
				psoCondition.depthCompareCondition = J_SHADER_PSO_APPLIY_CONDITION::SAME_AS;
			return psoCondition;
		}
	public:
		void SetMetallic(const float value) noexcept
		{
			metallic = value;
			SetFrameDirty();
		}
		void SetRoughness(const float value) noexcept
		{
			roughness = value;
			SetFrameDirty();
		}
		void SetAlbedoColor(const DirectX::XMFLOAT4& value)noexcept
		{
			albedoColor = value;
			SetFrameDirty();
		}
		void SetMatTransform(const DirectX::XMFLOAT4X4& value)noexcept
		{
			matTransform = value;
			SetFrameDirty();
		}
		void SetAlbedoMap(Core::JUserPtr<JTexture> newTexture) noexcept
		{
			SetTexture(albedoMap, newTexture, SHADER_FUNCTION_ALBEDO_MAP);
		}
		void SetNormalMap(Core::JUserPtr<JTexture> newTexture) noexcept
		{
			SetTexture(normalMap, newTexture, SHADER_FUNCTION_NORMAL_MAP);
		}
		void SetHeightMap(Core::JUserPtr<JTexture> newTexture) noexcept
		{
			SetTexture(heightMap, newTexture, SHADER_FUNCTION_HEIGHT_MAP);
		}
		void SetRoughnessMap(Core::JUserPtr<JTexture> newTexture) noexcept
		{
			SetTexture(roughnessMap, newTexture, SHADER_FUNCTION_ROUGHNESS_MAP);
		}
		void SetAmbientOcclusionMap(Core::JUserPtr<JTexture> newTexture) noexcept
		{
			SetTexture(ambientOcclusionMap, newTexture, SHADER_FUNCTION_AMBIENT_OCCLUSION_MAP);
		}
		void SetTexture(Core::JUserPtr<JTexture>& existingTexture,
			Core::JUserPtr<JTexture> newTexture,
			const J_GRAPHIC_SHADER_FUNCTION shaderFunction)
		{
			if (thisMat->IsActivated())
				CallOffResourceReference(existingTexture.Get());
			existingTexture = newTexture;
			if (thisMat->IsActivated())
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
		void SetShadowMapWrite(const bool value)noexcept
		{
			if (isShadowMapWrite == value)
				return;

			isShadowMapWrite = value; 
			SetFrameDirty();
			SetNewFunctionFlag(GetShaderGFunctionFlag());
		}
		void SetBoundingObjectDepthTest(const bool value)noexcept
		{
			if (isBoundingObjDepthTest == value)
				return;

			isBoundingObjDepthTest = value;
			SetFrameDirty();
			SetNewFunctionFlag(GetShaderGFunctionFlag());
		}
		void SetSkyMaterial(const bool value)noexcept
		{
			if (isSkyMateral == value)
				return;

			isSkyMateral = value;
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
			if (nonCulling == value)
				return;

			nonCulling = value;
			SetFrameDirty(); 
			SetNewOption(GetSubGraphicPso());
		}
		void SetPrimitiveType(const J_SHADER_PRIMITIVE_TYPE value)noexcept
		{
			if (primitiveType == value)
				return;

			primitiveType = value;
			SetFrameDirty();
			SetNewOption(GetSubGraphicPso());
		}
		void SetDepthCompareFunc(const J_SHADER_DEPTH_COMPARISON_FUNC value)noexcept
		{
			if (depthComparesionFunc == value)
				return;

			depthComparesionFunc = value;
			SetFrameDirty(); 
			SetNewOption(GetSubGraphicPso());
		}
		void SetNewFunctionFlag(const J_GRAPHIC_SHADER_FUNCTION newFunc)
		{
			if (!canUpdateShader)
				return;

			if (shader != nullptr && shader->GetShaderGFunctionFlag() == newFunc)
				return;

			Core::JUserPtr<JShader> newShader = nullptr;
			if (shader != nullptr)
			{
				newShader = Core::GetUserPtr(JShader::FindShader(newFunc, shader->GetSubGraphicPso()));
				if (newShader == nullptr)
					newShader = JICI::CreateRetUser<JShader>(OBJECT_FLAG_NONE, newFunc, shader->GetSubGraphicPso());
			}
			else
				newShader = JICI::CreateRetUser<JShader>(OBJECT_FLAG_NONE, newFunc);
			 
			SetShader(newShader);
		}
		void SetNewOption(const JShaderGraphicPsoCondition newPso)
		{
			if (!canUpdateShader)
				return;

			if (shader != nullptr && shader->GetSubGraphicPso() == newPso)
				return;

			Core::JUserPtr<JShader> newShader = nullptr;
			if (shader != nullptr)
			{
				newShader = Core::GetUserPtr(JShader::FindShader(shader->GetShaderGFunctionFlag(), newPso));
				if (newShader == nullptr)
					newShader = JICI::CreateRetUser<JShader>(OBJECT_FLAG_NONE, shader->GetShaderGFunctionFlag(), newPso);
			}
			else
				newShader = JICI::CreateRetUser<JShader>(OBJECT_FLAG_NONE, SHADER_FUNCTION_NONE, newPso);
			 
			SetShader(newShader);
		}
		void SetShader(Core::JUserPtr<JShader> newShader)noexcept
		{
			CallOffResourceReference(shader.Get());
			JShader* preShader = shader.Get();
			shader = newShader;
			CallOnResourceReference(shader.Get());

			if (preShader != nullptr && CallGetResourceReferenceCount(preShader) == 0)
				BeginDestroy(preShader);
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
			return nonCulling;
		}
		bool OnShadowMapWrite()const noexcept
		{
			return isShadowMapWrite;
		}
		bool OnBoundingObjectDepthTest()const noexcept
		{
			return isBoundingObjDepthTest;
		}
		bool IsSkyMaterial()const noexcept
		{
			return isSkyMateral;
		}
		bool IsDebugMaterial()const noexcept
		{
			return isDebugMaterial;
		}
		bool HasAlbedoMapTexture() const noexcept
		{
			return albedoMap.IsValid();
		}
		bool HasNormalMapTexture() const noexcept
		{
			return normalMap.IsValid();
		}
		bool HasHeightMapTexture() const noexcept
		{
			return heightMap.IsValid();
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
		void TryUpdateShader()
		{
			auto overlapped = Core::GetUserPtr(JShader::FindShader(GetShaderGFunctionFlag(), GetSubGraphicPso()));
			if (overlapped.IsValid())
				SetShader(overlapped);
			else
				SetShader(JICI::CreateRetUser<JShader>(OBJECT_FLAG_NONE, GetShaderGFunctionFlag(), GetSubGraphicPso()));
		}
		void PopTexture(JTexture* texture)noexcept
		{
			const size_t tarGuid = texture->GetGuid();
			if (HasAlbedoMapTexture() && albedoMap->GetGuid() == tarGuid)
				SetAlbedoMap(Core::JUserPtr< JTexture>{});
			if (HasNormalMapTexture() && normalMap->GetGuid() == tarGuid)
				SetNormalMap(Core::JUserPtr< JTexture>{});
			if (HasHeightMapTexture() && heightMap->GetGuid() == tarGuid)
				SetHeightMap(Core::JUserPtr< JTexture>{});
			if (HasRoughnessMapTexture() && roughnessMap->GetGuid() == tarGuid)
				SetRoughnessMap(Core::JUserPtr< JTexture>{});
			if (HasAmbientOcclusionMapTexture() && ambientOcclusionMap->GetGuid() == tarGuid)
				SetAmbientOcclusionMap(Core::JUserPtr< JTexture>{});
		}
	public:
		void OnResourceRef()
		{
			CallOnResourceReference(albedoMap.Get());
			CallOnResourceReference(normalMap.Get());
			CallOnResourceReference(heightMap.Get());
			CallOnResourceReference(roughnessMap.Get());
			CallOnResourceReference(ambientOcclusionMap.Get());
		}
		void OffResourceRef()
		{
			CallOffResourceReference(albedoMap.Get());
			CallOffResourceReference(normalMap.Get());
			CallOffResourceReference(heightMap.Get());
			CallOffResourceReference(roughnessMap.Get());
			CallOffResourceReference(ambientOcclusionMap.Get());
		}
	public:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == thisMat->GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				const size_t objGuid = jRobj->GetGuid();
				if (albedoMap.IsValid() && albedoMap->GetGuid() == objGuid)
					SetAlbedoMap(Core::JUserPtr<JTexture>{});
				if (normalMap.IsValid() && normalMap->GetGuid() == objGuid)
					SetNormalMap(Core::JUserPtr<JTexture>{});
				if (heightMap.IsValid() && heightMap->GetGuid() == objGuid)
					SetHeightMap(Core::JUserPtr<JTexture>{});
				if (roughnessMap.IsValid() && roughnessMap->GetGuid() == objGuid)
					SetRoughnessMap(Core::JUserPtr<JTexture>{});
				if (ambientOcclusionMap.IsValid() && ambientOcclusionMap->GetGuid() == objGuid)
					SetAmbientOcclusionMap(Core::JUserPtr<JTexture>{});

				if (shader != nullptr && shader->GetGuid() == objGuid)
					SetShader(nullptr);
			}
		}
	public:
		void UpdateFrame(Graphic::JMaterialConstants& constant)noexcept final
		{  
			constant.AlbedoColor = albedoColor;
			constant.Metalic = metallic;
			constant.Roughness = roughness;
			XMStoreFloat4x4(&constant.MatTransform, XMMatrixTranspose(XMLoadFloat4x4(&matTransform)));
			if (albedoMap.IsValid())
				constant.AlbedoMapIndex = albedoMap->GraphicResourceUserInterface().GetResourceArrayIndex();
			if (normalMap.IsValid())
				constant.NormalMapIndex = normalMap->GraphicResourceUserInterface().GetResourceArrayIndex();
			if (heightMap.IsValid())
				constant.HeightMapIndex = heightMap->GraphicResourceUserInterface().GetResourceArrayIndex();
			if (roughnessMap.IsValid())
				constant.RoughnessMapIndex = roughnessMap->GraphicResourceUserInterface().GetResourceArrayIndex();
			if (ambientOcclusionMap.IsValid())
				constant.AmbientOcclusionMapIndex = ambientOcclusionMap->GraphicResourceUserInterface().GetResourceArrayIndex();
		}
	public:
		bool ReadAssetData()
		{
			std::wifstream stream;
			stream.open(thisMat->GetPath(), std::ios::in | std::ios::binary);
			if (!stream.is_open())
				return false;

			bool sShadow = false;
			bool sLight = false;
			bool sAlbedoOnly = false;
			bool sIsShadowMapWrite = false;
			bool sBoundingObjDepthTest = false;
			bool sIsSkyMateral = false;
			bool sIsDebugMaterial = false;
			bool sAlphaclip = false;
			bool sNonCulling = false;
			int sPrimitiveType = 0;
			int sDepthComparesionFunc = 0;
			float sMetallic;
			float sRoughness;
			DirectX::XMFLOAT4 sAlbedoColor;
			DirectX::XMFLOAT4X4 sMatTransform;

			JFileIOHelper::LoadAtomicData(stream, sShadow);
			JFileIOHelper::LoadAtomicData(stream, sLight);
			JFileIOHelper::LoadAtomicData(stream, sAlbedoOnly);
			JFileIOHelper::LoadAtomicData(stream, sIsShadowMapWrite);
			JFileIOHelper::LoadAtomicData(stream, sBoundingObjDepthTest);
			JFileIOHelper::LoadAtomicData(stream, sIsSkyMateral);
			JFileIOHelper::LoadAtomicData(stream, sIsDebugMaterial);
			JFileIOHelper::LoadAtomicData(stream, sAlphaclip);

			JFileIOHelper::LoadAtomicData(stream, sNonCulling);
			JFileIOHelper::LoadAtomicData(stream, sPrimitiveType);
			JFileIOHelper::LoadAtomicData(stream, sDepthComparesionFunc);

			JFileIOHelper::LoadAtomicData(stream, sMetallic);
			JFileIOHelper::LoadAtomicData(stream, sRoughness);

			JFileIOHelper::LoadXMFloat4(stream, sAlbedoColor);
			JFileIOHelper::LoadXMFloat4x4(stream, sMatTransform);

			Core::JUserPtr<JTexture> sAlbedoMap = JFileIOHelper::LoadHasObjectIden<JTexture>(stream);
			Core::JUserPtr<JTexture> sNormalMap = JFileIOHelper::LoadHasObjectIden<JTexture>(stream);
			Core::JUserPtr<JTexture> sHeightMap = JFileIOHelper::LoadHasObjectIden<JTexture>(stream);
			Core::JUserPtr<JTexture> sRoughnessMap = JFileIOHelper::LoadHasObjectIden<JTexture>(stream);
			Core::JUserPtr<JTexture> sAmbientOcclusionMap = JFileIOHelper::LoadHasObjectIden<JTexture>(stream);
			stream.close();

			canUpdateShader = false;
			SetShadow(sShadow);
			SetLight(sLight);
			SetAlbedoMapOnly(sAlbedoOnly);
			SetNonCulling(sNonCulling);
			SetShadowMapWrite(sIsShadowMapWrite);
			SetBoundingObjectDepthTest(sBoundingObjDepthTest);
			SetSkyMaterial(sIsSkyMateral);
			SetDebugMaterial(sIsDebugMaterial);
			SetAlphaClip(sAlphaclip);

			SetPrimitiveType((J_SHADER_PRIMITIVE_TYPE)sPrimitiveType);
			SetDepthCompareFunc((J_SHADER_DEPTH_COMPARISON_FUNC)sDepthComparesionFunc);

			SetMetallic(sMetallic);
			SetRoughness(sRoughness);
			SetAlbedoColor(sAlbedoColor);
			SetMatTransform(sMatTransform);

			SetAlbedoMap(sAlbedoMap);
			SetNormalMap(sNormalMap);
			SetHeightMap(sHeightMap);
			SetRoughnessMap(sRoughnessMap);
			SetAmbientOcclusionMap(sAmbientOcclusionMap);
			canUpdateShader = true;
			 
			TryUpdateShader();
			return true;
		}
		bool WriteAssetData()
		{
			std::wofstream stream;
			stream.open(thisMat->GetPath(), std::ios::out | std::ios::binary);
			if (!stream.is_open())
				return false;

			JFileIOHelper::StoreAtomicData(stream, L"Shadow", shadow);
			JFileIOHelper::StoreAtomicData(stream, L"Light", light);
			JFileIOHelper::StoreAtomicData(stream, L"AlbedoOnly", albedoMapOnly);
			JFileIOHelper::StoreAtomicData(stream, L"ShadowMapWrite", isShadowMapWrite);
			JFileIOHelper::StoreAtomicData(stream, L"BoundingObjDepthTest", isBoundingObjDepthTest);
			JFileIOHelper::StoreAtomicData(stream, L"SkyMaterial", isSkyMateral);
			JFileIOHelper::StoreAtomicData(stream, L"DebugMaterial", isDebugMaterial);
			JFileIOHelper::StoreAtomicData(stream, L"AlphaClip", alphaClip);

			JFileIOHelper::StoreAtomicData(stream, L"NonCulling", nonCulling);
			JFileIOHelper::StoreAtomicData(stream, L"PrimitiveType", (int)primitiveType);
			JFileIOHelper::StoreAtomicData(stream, L"DepthComparesionFunc", (int)depthComparesionFunc);

			JFileIOHelper::StoreAtomicData(stream, L"Metallic", metallic);
			JFileIOHelper::StoreAtomicData(stream, L"Roughness", roughness);

			JFileIOHelper::StoreXMFloat4(stream, L"AlbedoColor", albedoColor);
			JFileIOHelper::StoreXMFloat4x4(stream, L"Matransform", matTransform);

			JFileIOHelper::StoreHasObjectIden(stream, albedoMap.Get());
			JFileIOHelper::StoreHasObjectIden(stream, normalMap.Get());
			JFileIOHelper::StoreHasObjectIden(stream, heightMap.Get());
			JFileIOHelper::StoreHasObjectIden(stream, roughnessMap.Get());
			JFileIOHelper::StoreHasObjectIden(stream, ambientOcclusionMap.Get());

			stream.close();
			return true;
		}
	public:
		static void RegisterCallOnce()
		{
			auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex(JMaterial::GetStaticResourceType(), format); };

			static GetRTypeInfoCallable getTypeInfoCallable{ &JMaterial::StaticTypeInfo };
			static GetAvailableFormatCallable getAvailableFormatCallable{ &JMaterial::GetAvailableFormat };
			static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

			static auto setFrameLam = [](JResourceObject* jRobj)
			{
				static_cast<JMaterial*>(jRobj)->impl->SetFrameDirty();
			};
			static auto setBuffIndexLam = [](JResourceObject* jRobj, uint value)
			{
				static_cast<JMaterial*>(jRobj)->impl->SetFrameBuffOffset(value);
			};
			static SetRFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };
			static SetRFrameBuffIndexCallable setFrameBuffIndexCallable{ setBuffIndexLam };

			static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{J_RESOURCE_TYPE::SHADER}, false, true, true, false };
			static RTypeCommonFunc rTypeCFunc{ getTypeInfoCallable, getAvailableFormatCallable, getFormatIndexCallable };
			static RTypePrivateFunc rTypeiFunc{ &setFrameDirtyCallable, &setFrameBuffIndexCallable };

			RegisterRTypeInfo(rTypeHint, rTypeCFunc, rTypeiFunc);
			Core::JIdentifier::RegisterPrivateInterface(JMaterial::StaticTypeInfo(), mPrivate);
		}
	};

	JMaterial::InitData::InitData(JDirectory* directory)
		:JResourceObject::InitData(JMaterial::StaticTypeInfo(), GetDefaultFormatIndex(), GetStaticResourceType(), directory)
	{}

	JMaterial::InitData::InitData(const uint8 formatIndex, JDirectory* directory)
		:JResourceObject::InitData(JMaterial::StaticTypeInfo(), formatIndex, GetStaticResourceType(), directory)
	{}
	JMaterial::InitData::InitData(const size_t guid, const uint8 formatIndex, JDirectory* directory)
		: JResourceObject::InitData(JMaterial::StaticTypeInfo(), guid, formatIndex, GetStaticResourceType(), directory)
	{}
	JMaterial::InitData::InitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex,
		JDirectory* directory)
		: JResourceObject::InitData(JMaterial::StaticTypeInfo(), name, guid, flag, formatIndex, GetStaticResourceType(), directory)
	{ }

	Core::JIdentifierPrivate& JMaterial::GetPrivateInterface()const noexcept
	{
		return mPrivate;
	}
	JFrameUpdateUserAccess JMaterial::GetFrameUserInterface() noexcept
	{
		return JFrameUpdateUserAccess(Core::GetUserPtr(this), impl.get());
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
	DirectX::XMFLOAT4 JMaterial::GetAlbedoColor() const noexcept
	{
		return impl->GetAlbedoColor();
	}
	DirectX::XMFLOAT4X4 JMaterial::GetMatTransform()const  noexcept
	{
		return impl->GetMatTransform();
	}
	Core::JUserPtr<JTexture> JMaterial::GetAlbedoMap() const noexcept
	{
		return impl->GetAlbedoMap();
	}
	Core::JUserPtr<JTexture> JMaterial::GetNormalMap() const noexcept
	{
		return impl->GetNormalMap();
	}
	Core::JUserPtr<JTexture> JMaterial::GetHeightMap() const noexcept
	{
		return impl->GetHeightMap();
	}
	Core::JUserPtr<JTexture> JMaterial::GetRoughnessMap() const noexcept
	{
		return impl->GetRoughnessMap();
	}
	Core::JUserPtr<JTexture> JMaterial::GetAmbientOcclusionMap() const noexcept
	{
		return impl->GetAmbientOcclusionMap();
	}
	J_SHADER_PRIMITIVE_TYPE JMaterial::GetPrimitiveType()const noexcept
	{
		return impl->primitiveType;
	}
	J_SHADER_DEPTH_COMPARISON_FUNC JMaterial::GetDepthCompasionFunc()const noexcept
	{
		return impl->depthComparesionFunc;
	}
	void JMaterial::SetMetallic(const float value) noexcept
	{
		impl->SetMetallic(value);
	}
	void JMaterial::SetRoughness(const float value) noexcept
	{
		impl->SetRoughness(value);
	}
	void JMaterial::SetAlbedoColor(const DirectX::XMFLOAT4& value)noexcept
	{
		impl->SetAlbedoColor(value);
	}
	void JMaterial::SetMatTransform(const DirectX::XMFLOAT4X4& value)noexcept
	{
		impl->SetMatTransform(value);
	}
	void JMaterial::SetAlbedoMap(Core::JUserPtr<JTexture> texture) noexcept
	{
		impl->SetAlbedoMap(texture);
	}
	void JMaterial::SetNormalMap(Core::JUserPtr<JTexture> texture) noexcept
	{
		impl->SetNormalMap(texture);
	}
	void JMaterial::SetHeightMap(Core::JUserPtr<JTexture> texture) noexcept
	{
		impl->SetHeightMap(texture);
	}
	void JMaterial::SetRoughnessMap(Core::JUserPtr<JTexture> texture) noexcept
	{
		impl->SetRoughnessMap(texture);
	}
	void JMaterial::SetAmbientOcclusionMap(Core::JUserPtr<JTexture> texture) noexcept
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
	void JMaterial::SetShadowMapWrite(const bool value)noexcept
	{
		impl->SetShadowMapWrite(value);
	}
	void JMaterial::SetBoundingObjectDepthTest(const bool value)noexcept
	{
		impl->SetBoundingObjectDepthTest(value);
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
	bool JMaterial::OnShadowMapWrite()const noexcept
	{
		return impl->OnShadowMapWrite();
	}
	bool JMaterial::OnBoundingObjectDepthTest()const noexcept
	{
		return impl->OnBoundingObjectDepthTest();
	}
	bool JMaterial::IsSkyMaterial()const noexcept
	{
		return impl->IsSkyMaterial();
	}
	bool JMaterial::IsDebugMaterial()const noexcept
	{
		return impl->IsDebugMaterial();
	}
	bool JMaterial::HasAlbedoMapTexture() const noexcept
	{
		return impl->HasAlbedoMapTexture();
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
		JResourceObject::DoDeActivate();
		SetValid(false);
		impl->OffFrameDirty();
		impl->OffResourceRef();
	}
	JMaterial::JMaterial(const InitData& initData)
		: JResourceObject(initData), impl(std::make_unique<JMaterialImpl>(initData, this))
	{
		impl->Initialize(initData);
	}
	JMaterial::~JMaterial()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JMaterialPrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JMaterialPrivate::AssetDataIOInterface;
	using FrameUpdateInterface = JMaterialPrivate::FrameUpdateInterface;
	using FrameBuffInterface = JMaterialPrivate::FrameBuffInterface; 
	using UpdateShaderInterface = JMaterialPrivate::UpdateShaderInterface;

	Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JMaterial>(*static_cast<JMaterial::InitData*>(initData.get()));
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JMaterial::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}

	Core::JIdentifier* AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JMaterial::LoadData::StaticTypeInfo()))
			return nullptr;
 
		auto loadData = static_cast<JMaterial::LoadData*>(data);
		auto pathData = loadData->pathData;
		JDirectory* directory = loadData->directory;

		auto initData = std::make_unique< JMaterial::InitData>(directory);	//for load metadata
		if (LoadMetaData(pathData.engineMetaFileWPath, initData.get()) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;

		JMaterial* newMat = nullptr;
		if (directory->HasFile(initData->guid))
			newMat = static_cast<JMaterial*>(Core::GetUserPtr(JMaterial::StaticTypeInfo().TypeGuid(), initData->guid).Get());

		if (newMat == nullptr)
		{
			initData->name = pathData.name;
			auto rawPtr = mPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &mPrivate);
			newMat = static_cast<JMaterial*>(rawPtr);
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

		JMaterial* mat = static_cast<JMaterial*>(storeData->obj);
		return mat->impl->WriteAssetData() ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JMaterial::InitData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		std::wifstream stream;
		stream.open(path, std::ios::in | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto loadMetaData = static_cast<JMaterial::InitData*>(data);
		if (LoadCommonMetaData(stream, loadMetaData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		stream.close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JMaterial::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JMaterial::StoreData*>(data);
		JMaterial* mat = static_cast<JMaterial*>(storeData->obj);

		std::wofstream stream;
		stream.open(mat->GetMetaFilePath(), std::ios::out | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (StoreCommonMetaData(stream, storeData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	bool FrameUpdateInterface::UpdateStart(JMaterial* mat, const bool isUpdateForced)noexcept
	{
		if (isUpdateForced)
			mat->impl->SetFrameDirty();

		return mat->impl->IsFrameDirted();
	}
	void FrameUpdateInterface::UpdateFrame(JMaterial* mat, Graphic::JMaterialConstants& constants)noexcept
	{
		mat->impl->UpdateFrame(constants);
	}
	void FrameUpdateInterface::UpdateEnd(JMaterial* mat)noexcept
	{
		mat->impl->UpdateEnd();
	}
	uint FrameUpdateInterface::GetCBOffset(JMaterial* mat)noexcept
	{
		return mat->impl->GetFrameBuffOffset();
	}

	uint FrameBuffInterface::GetCBOffset(JMaterial* mat)noexcept
	{
		return mat->impl->GetFrameBuffOffset();
	}
 
	void UpdateShaderInterface::OnUpdateShaderTrigger(JMaterial* mat)noexcept
	{
		mat->impl->canUpdateShader = true;
	}
	void UpdateShaderInterface::OffUpdateShaderTrigger(JMaterial* mat)noexcept
	{
		mat->impl->canUpdateShader = false;
	}
	void UpdateShaderInterface::UpdateShader(JMaterial* mat)noexcept
	{ 
		mat->impl->TryUpdateShader();
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JMaterialPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	JResourceObjectPrivate::AssetDataIOInterface& JMaterialPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}
