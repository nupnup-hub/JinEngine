#pragma once
#include"JTextureInterface.h"
#include"../../../Graphic/JGraphicTextureType.h"
#include<d3d12.h> 

namespace JinEngine
{
	class JTexture : public JTextureInterface
	{
		REGISTER_CLASS(JTexture)
	private:
		struct TextureMetadata : public ObjectMetadata
		{
		public:
			int textureType;
		};
	private: 
		Microsoft::WRL::ComPtr<ID3D12Resource> uploadHeap = nullptr;
		Graphic::J_GRAPHIC_TEXTURE_TYPE textureType;
	public:	  
		uint GetTextureWidth()const noexcept;
		uint GetTextureHeight()const noexcept;
		Graphic::J_GRAPHIC_TEXTURE_TYPE GetTextureType()const noexcept;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::TEXTURE;
		}
		std::string GetFormat()const noexcept final;
		static std::vector<std::string> GetAvailableFormat()noexcept;
		void SetTextureType(const Graphic::J_GRAPHIC_TEXTURE_TYPE textureType)noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void StuffResource() final;
		void ClearResource() final;
		bool ReadTextureData();
	private:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JTexture* texture);
		static Core::J_FILE_IO_RESULT StoreMetadata(std::wofstream& stream, JTexture* texture);
		static JTexture* LoadObject(JDirectory* directory, const JResourcePathData& pathData);
		static Core::J_FILE_IO_RESULT LoadMetadata(std::wifstream& stream, const std::string& folderPath, TextureMetadata& metadata);
		static void RegisterFunc();
	private:
		JTexture(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const int formatIndex);
		~JTexture();
	};

}