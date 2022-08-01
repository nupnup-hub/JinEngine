#pragma once
#include"../JResourceObject.h"
#include"../../../Graphic/JGraphicTextureType.h"
#include<d3d12.h>
#include<wrl/client.h>

namespace JinEngine
{
	namespace Graphic
	{
		struct JGraphicTextureHandle;
		class GraphicImpl;
	}
	class JTexture : public JResourceObject
	{
		REGISTER_CLASS(JTexture)
			friend class GraphicImpl;
	private:
		struct TextureMetadata : public ObjectMetadata
		{
		public:
			int textureType;
		};
	private:
		Graphic::JGraphicTextureHandle* graphicTextureHandle = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> uploadHeap = nullptr;
		Graphic::J_GRAPHIC_TEXTURE_TYPE textureType;
	public:	
		//Return Srv Heap Index
		uint GetTextureHeapIndex()const noexcept;
		//Return Srv Descriptor Buf Index
		uint GetTextureVectorIndex()const noexcept;
		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const noexcept;
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const noexcept;
		int GetTextureWidth()const noexcept;
		int GetTextureHeight()const noexcept;
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
		void ClearGraphicTextureHandle();
		bool UploadTexture();
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