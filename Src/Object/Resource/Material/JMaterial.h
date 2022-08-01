#pragma once  
#include"../JResourceObject.h"    
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
	class JMaterial : public JResourceObject
	{
		REGISTER_CLASS(JMaterial)
		friend class Graphic::JGraphicImpl;
		friend class ObjectDetail;
	protected:
		JShader* shader;
	private:
		uint matCBIndex = -1;
		int gameObjectDirty = Graphic::gNumFrameResources;

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
		float metallic;
		float roughness;
		DirectX::XMFLOAT4 albedoColor;
		DirectX::XMFLOAT4X4 matTransform;
	public: 
		uint GetMatCBIndex() const noexcept;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::MATERIAL;
		}
		std::string GetFormat()const noexcept final;

		float GetMetallic() const noexcept;
		float GetRoughness() const noexcept;
		DirectX::XMFLOAT4 GetAlbedoColor() const noexcept;
		DirectX::XMFLOAT4X4 GetMatTransform() const noexcept;
		JTexture* GetDiffsueMap() const noexcept;
		JTexture* GetNormalMap() const noexcept;
		JTexture* GetHeightMap() const noexcept;
		JTexture* GetRoughnessMap() const noexcept;
		JTexture* GetAmbientOcclusionMap() const noexcept;
		static std::vector<std::string> GetAvailableFormat()noexcept;

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

		bool IsMaterialDirtied()const noexcept; 
	protected:
		void DoActivate() noexcept final;
		void DoDeActivate()noexcept final;
		void StuffFrameDirty()noexcept;
		void MinusMaterialformDirty() noexcept;

		void TextureChange(JTexture* be, JTexture* af, const J_SHADER_FUNCTION func)noexcept;
		void SetNewFunctionFlag(const J_SHADER_FUNCTION newFunc);
	private:
		JShader* GetShader()const noexcept;
		void SetShader(JShader* newShader)noexcept;
		void SetMatCBIndex(uint value) noexcept;
	private:
		J_SHADER_FUNCTION CalculateShaderFunc()noexcept;
		void UpdateMaterial(Graphic::JMaterialConstants& matData)noexcept;
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JMaterial* material);
		static JMaterial* LoadObject(JDirectory* directory, const JResourcePathData& pathData);
		static void RegisterFunc();
	private:
		JMaterial(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
		~JMaterial();
	};
}
