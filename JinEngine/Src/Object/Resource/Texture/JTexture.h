#pragma once
#include"JTextureInterface.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceType.h"
#include<d3d12.h> 

namespace JinEngine
{
	class JTexture final: public JTextureInterface
	{
		REGISTER_CLASS(JTexture)
	public:
		struct JTextureInitData : public JResourceInitData
		{
		public:
			const std::wstring oridataPath;
		public:
			JTextureInitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				JDirectory* directory,
				const std::wstring oridataPath);
			JTextureInitData(const std::wstring& name,
				const size_t guid,
				const J_OBJECT_FLAG flag,
				JDirectory* directory,
				const uint8 formatIndex);
			JTextureInitData(const std::wstring& name,
				JDirectory* directory,
				const std::wstring oridataPath);
		public:
			bool IsValidCreateData()final; 
		};
		using InitData = JTextureInitData;
	private:
		struct JTextureMetadata : public JResourceMetaData
		{
		public:
			Graphic::J_GRAPHIC_RESOURCE_TYPE textureType;
		};
	private: 
		Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer = nullptr;
		Graphic::J_GRAPHIC_RESOURCE_TYPE textureType = Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D;
	public:
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::TEXTURE;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
	public:	  
		uint GetTextureWidth()const noexcept;
		uint GetTextureHeight()const noexcept;
		Graphic::J_GRAPHIC_RESOURCE_TYPE GetTextureType()const noexcept;
		void SetTextureType(const Graphic::J_GRAPHIC_RESOURCE_TYPE textureType)noexcept;
	public:
		JTextureImportInterface* ImportInterface()noexcept final;
	private:
		void DoCopy(JObject* ori)final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void StuffResource() final;
		void ClearResource() final;
		bool ReadTextureData();
		bool ImportTexture(const std::wstring& oriPath) final;
	private:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JTexture* texture);
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JTexture* texture);
		static JTexture* LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData);
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, JTextureMetadata& metadata);
		static void RegisterCallOnce();
	private:
		JTexture(const JTextureInitData& initdata);
		~JTexture();
	};

}