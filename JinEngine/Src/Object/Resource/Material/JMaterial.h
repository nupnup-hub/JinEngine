#pragma once  
#include"../JResourceObject.h"   
#include"../Shader/JShaderFunctionEnum.h"
#include"../Shader/JShaderGraphicPsoCondition.h" 
#include"../../../Graphic/Upload/Frameresource/JFrameUpdateUserAccess.h"
#include<DirectXMath.h>

namespace JinEngine
{ 
	class JShader;   
	class JTexture;
	class JMaterialPrivate;
	class JMaterial : public JResourceObject, 
		public Graphic::JFrameUpdateUserAccessInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JMaterial)
		REGISTER_CLASS_IDENTIFIER_DEFAULT_LAZY_DESTRUCTION
	public: 
		class InitData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			InitData(const JUserPtr<JDirectory>& directory);
			InitData(const uint8 formatIndex, const JUserPtr<JDirectory>& directory);
			InitData(const size_t guid, const uint8 formatIndex, const JUserPtr<JDirectory>& directory);
			InitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				const uint8 formatIndex,
				const JUserPtr<JDirectory>& directory);
		};
	private:
		friend class JMaterialPrivate;
		class JMaterialImpl;
	private:
		std::unique_ptr<JMaterialImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		Graphic::JFrameUpdateUserAccess FrameUserInterface() noexcept final;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::MATERIAL;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
		JUserPtr<JShader> GetShader()const noexcept;
		JShader* GetRawShader()const noexcept;
		float GetMetallic() const noexcept;
		float GetRoughness() const noexcept;
		DirectX::XMFLOAT4 GetAlbedoColor() const noexcept;
		DirectX::XMFLOAT4X4 GetMatTransform() const noexcept;
		JUserPtr<JTexture> GetAlbedoMap() const noexcept;
		JUserPtr<JTexture> GetNormalMap() const noexcept;
		JUserPtr<JTexture> GetHeightMap() const noexcept;
		JUserPtr<JTexture> GetRoughnessMap() const noexcept;
		JUserPtr<JTexture> GetAmbientOcclusionMap() const noexcept;
		J_SHADER_PRIMITIVE_TYPE GetPrimitiveType()const noexcept;
		J_SHADER_DEPTH_COMPARISON_FUNC GetDepthCompasionFunc()const noexcept;
	public:
		void SetMetallic(const float value) noexcept;
		void SetRoughness(const float value) noexcept;
		void SetAlbedoColor(const DirectX::XMFLOAT4& value) noexcept;
		void SetMatTransform(const DirectX::XMFLOAT4X4& value) noexcept;
		void SetAlbedoMap(JUserPtr<JTexture> texture) noexcept;
		void SetNormalMap(JUserPtr<JTexture>texture) noexcept;
		void SetHeightMap(JUserPtr<JTexture> texture) noexcept;
		void SetRoughnessMap(JUserPtr<JTexture> texture) noexcept;
		void SetAmbientOcclusionMap(JUserPtr<JTexture> texture) noexcept;
		void SetShadow(const bool value)noexcept;
		void SetLight(const bool value)noexcept;
		void SetAlbedoMapOnly(const bool value)noexcept;
		void SetShadowMapWrite(const bool value)noexcept;
		void SetBoundingObjectDepthTest(const bool value)noexcept;
		void SetSkyMaterial(const bool value)noexcept;
		void SetDebugMaterial(const bool value)noexcept;
		void SetAlphaClip(const bool value)noexcept;
		void SetNonCulling(const bool value)noexcept;
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
		bool HasAlbedoMapTexture() const noexcept;
		bool HasNormalMapTexture() const noexcept;
		bool HasHeightMapTexture() const noexcept;
		bool HasRoughnessMapTexture() const noexcept;
		bool HasAmbientOcclusionMapTexture() const noexcept;
	public:
		void PopTexture(JTexture* texture)noexcept;
	protected:
		void DoActivate() noexcept final;
		void DoDeActivate()noexcept final;
	private:
		JMaterial(const InitData& initData);
		~JMaterial();
	};
}