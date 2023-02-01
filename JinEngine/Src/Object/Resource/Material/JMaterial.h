#pragma once  
#include"JMaterialInterface.h"  
#include"../Texture/JTexture.h"
#include"../Shader/JShaderFunctionEnum.h"
#include"../Shader/JShaderGraphicPsoCondition.h"
#include"../../../Graphic/JGraphicConstants.h"   
#include"../../../Utility/JMathHelper.h"   

namespace JinEngine
{
	class JDirectory;
	class JShader; 
	namespace Graphic
	{
		struct JMaterialConstants;
		class JGraphicImpl;
	}
	 
	class JMaterial : public JMaterialInterface
	{
		REGISTER_CLASS(JMaterial)
	public:
		struct JMaterialInitData : JResourceInitData
		{
		public:
			JMaterialInitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				JDirectory* directory,
				const uint8 formatIndex = JResourceObject::GetFormatIndex<JMaterial>(GetAvailableFormat()[0]));
			JMaterialInitData(const std::wstring& name,
				JDirectory* directory,
				const uint8 formatIndex = JResourceObject::GetFormatIndex<JMaterial>(GetAvailableFormat()[0]));
			JMaterialInitData(JDirectory* directory,
				const uint8 formatIndex = JResourceObject::GetFormatIndex<JMaterial>(GetAvailableFormat()[0]));
		public:
			J_RESOURCE_TYPE GetResourceType() const noexcept;
		};
		using InitData = JMaterialInitData;
	private:
		JShader* shader = nullptr;
	private:
		REGISTER_PROPERTY_EX(albedoMap, GetAlbedoMap, SetAlbedoMap, GUI_SELECTOR(true, true))
		JTexture* albedoMap = nullptr;
		REGISTER_PROPERTY_EX(normalMap, GetNormalMap, SetNormalMap, GUI_SELECTOR(true, true))
		JTexture* normalMap = nullptr;
		REGISTER_PROPERTY_EX(heightMap, GetHeightMap, SetHeightMap, GUI_SELECTOR(true, true))
		JTexture* heightMap = nullptr;
		REGISTER_PROPERTY_EX(roughnessMap, GetRoughnessMap, SetRoughnessMap, GUI_SELECTOR(true, true))
		JTexture* roughnessMap = nullptr;
		REGISTER_PROPERTY_EX(ambientOcclusionMap, GetAmbientOcclusionMap, SetAmbientOcclusionMap, GUI_SELECTOR(true, true))
		JTexture* ambientOcclusionMap = nullptr;

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
		J_SHADER_PRIMITIVE_TYPE primitiveType = J_SHADER_PRIMITIVE_TYPE::DEFAULT;
		J_SHADER_DEPTH_COMPARISON_FUNC depthComparesionFunc = J_SHADER_DEPTH_COMPARISON_FUNC::DEFAULT;
		REGISTER_PROPERTY_EX(metallic, GetMetallic, SetMetallic, GUI_SLIDER(0.0f, 1.0f, false, false))
		float metallic = 0;
		REGISTER_PROPERTY_EX(roughness, GetRoughness, SetRoughness, GUI_SLIDER(0.0f, 1.0f))
		float roughness = 0;
		REGISTER_PROPERTY_EX(albedoColor, GetAlbedoColor, SetAlbedoColor, GUI_COLOR_PICKER(true))
		DirectX::XMFLOAT4 albedoColor = { 0.75f, 0.75f, 0.75f, 0.65f };
		DirectX::XMFLOAT4X4 matTransform = JMathHelper::Identity4x4();
	public:
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::MATERIAL;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;

		JShader* GetShader()const noexcept;
		float GetMetallic() const noexcept;
		float GetRoughness() const noexcept;
		DirectX::XMFLOAT4 GetAlbedoColor() const noexcept;
		DirectX::XMFLOAT4X4 GetMatTransform() const noexcept;
		JTexture* GetAlbedoMap() const noexcept;
		JTexture* GetNormalMap() const noexcept;
		JTexture* GetHeightMap() const noexcept;
		JTexture* GetRoughnessMap() const noexcept;
		JTexture* GetAmbientOcclusionMap() const noexcept;
		J_SHADER_PRIMITIVE_TYPE GetPrimitiveType()const noexcept;
		J_SHADER_DEPTH_COMPARISON_FUNC GetDepthCompasionFunc()const noexcept;

		void SetMetallic(float value) noexcept;
		void SetRoughness(float value) noexcept;
		void SetAlbedoColor(const DirectX::XMFLOAT4& value) noexcept;
		void SetMatTransform(const DirectX::XMFLOAT4X4& value) noexcept;
		void SetAlbedoMap(JTexture* texture) noexcept;
		void SetNormalMap(JTexture* texture) noexcept;
		void SetHeightMap(JTexture* texture) noexcept;
		void SetRoughnessMap(JTexture* texture) noexcept;
		void SetAmbientOcclusionMap(JTexture* texture) noexcept;
		void SetShadow(bool value)noexcept;
		void SetLight(bool value)noexcept;
		void SetAlbedoMapOnly(bool value)noexcept;
		void SetNonCulling(bool value)noexcept;
		void SetShadowMapWrite(bool value)noexcept;
		void SetBoundingObjectDepthTest(bool value)noexcept;
		void SetSkyMaterial(bool value)noexcept;
		void SetDebugMaterial(bool value)noexcept; 
		void SetAlphaClip(bool value)noexcept;
		void SetPrimitiveType(const J_SHADER_PRIMITIVE_TYPE value)noexcept;
		void SetDepthCompareFunc(const J_SHADER_DEPTH_COMPARISON_FUNC value)noexcept;
	public:
		bool OnShadow()const noexcept;
		bool OnLight()const noexcept;
		bool OnAlbedoOnly()const noexcept;
		bool OnNonCulling()const noexcept;
		bool OnShadowMapWrite()const noexcept;
		bool OnBoundingObjectDepthTest()const noexcept;
		bool IsSkyMaterial()const noexcept;
		bool IsDebugMaterial()const noexcept; 
		//레지스터에 등록된 머테리얼만 검사하므로 gameObjectDirty pointer 유무는 체크하지 않는다.
		bool HasAlbedoMapTexture() const noexcept;
		bool HasNormalMapTexture() const noexcept;
		bool HasHeightMapTexture() const noexcept;
		bool HasRoughnessMapTexture() const noexcept;
		bool HasAmbientOcclusionMapTexture() const noexcept;

		void PopTexture(JTexture* texture)noexcept;
	protected:
		void TextureChange(JTexture* be, JTexture* af, const J_GRAPHIC_SHADER_FUNCTION func)noexcept;
		void SetNewFunctionFlag(const J_GRAPHIC_SHADER_FUNCTION newFunc);
		void SetNewOption(const JShaderGraphicPsoCondition newPso);
	private:
		void SetShader(JShader* newShader)noexcept;
	private:
		void UpdateFrame(Graphic::JMaterialConstants& constant)final;
	private:
		void DoCopy(JObject* ori) final;
	protected:
		void DoActivate() noexcept final;
		void DoDeActivate()noexcept final;
	private:
		bool WriteMaterialData();
		bool ReadMateiralData();
	private:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
	private:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JMaterial* material);
		static JMaterial* LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData);
		static void RegisterJFunc();
	private:
		JMaterial(const JMaterialInitData& initdata);
		~JMaterial();
	};
}
