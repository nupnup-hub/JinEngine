#pragma once  
#include"JMaterialInterface.h"   
#include"../Shader/JShaderFunctionEnum.h"
#include"../../../Graphic/FrameResource/JFrameResourceCash.h"   
#include"../../../Utility/JMathHelper.h"  

namespace JinEngine
{ 
	class JDirectory;
	class JShader;
	class JTexture;
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
		JTexture* albedoMap = nullptr;
		JTexture* normalMap = nullptr;
		JTexture* heightMap = nullptr;
		JTexture* roughnessMap = nullptr;
		JTexture* ambientOcclusionMap = nullptr;

		//수정필요
		//isDebug
		bool shadow = false;
		bool light = false;
		bool albedoOnly = false;
		bool nonCulling = false;
		bool shadowMap = false;
		bool isSkyMateral = false;
		bool isDebugMaterial = false;
		float metallic = 0;
		float roughness = 0;
		DirectX::XMFLOAT4 albedoColor = { 1,1,1,1 };
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
		JTexture* GetDiffsueMap() const noexcept;
		JTexture* GetNormalMap() const noexcept;
		JTexture* GetHeightMap() const noexcept;
		JTexture* GetRoughnessMap() const noexcept;
		JTexture* GetAmbientOcclusionMap() const noexcept;

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
		void SetAlbedoOnly(bool value)noexcept;
		void SetNonCulling(bool value)noexcept;
		void SetShadowMap(bool value)noexcept;
		void SetSkyMaterial(bool value)noexcept;
		void SetDebugMaterial(bool value)noexcept;
		//레지스터에 등록된 머테리얼만 검사하므로 gameObjectDirty pointer 유무는 체크하지 않는다.
		bool HasAlbedoMapTexture() const noexcept;
		bool HasNormalMapTexture() const noexcept;
		bool HasHeightMapTexture() const noexcept;
		bool HasRoughnessMapTexture() const noexcept;
		bool HasAmbientOcclusionMapTexture() const noexcept;

		void PopTexture(JTexture* texture)noexcept;
	protected:
		void TextureChange(JTexture* be, JTexture* af, const J_SHADER_FUNCTION func)noexcept;
		void SetNewFunctionFlag(const J_SHADER_FUNCTION newFunc);
	private:
		void SetShader(JShader* newShader)noexcept; 
	private:
		bool UpdateFrame(Graphic::JMaterialConstants& constant)final;
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
