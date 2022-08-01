#include"JTexture.h"
#include"../JResourceObjectFactory.h"
#include"../../Directory/JDirectory.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Graphic/JGraphic.h"
#include"../../../Graphic/JGraphicTextureHandle.h" 
#include"../../../Application/JApplicationVariable.h"
#include"../../../../Lib/LoadTextureFromFile.h"
#include"../../../../Lib/DirectX/DDSTextureLoader.h" 
#include<memory>

namespace JinEngine
{
	uint JTexture::GetTextureHeapIndex()const noexcept
	{
		return graphicTextureHandle->GetSrvHeapIndex();
	}
	uint JTexture::GetTextureVectorIndex()const noexcept
	{
		return graphicTextureHandle->GetResourceVectorIndex();
	}
	D3D12_CPU_DESCRIPTOR_HANDLE JTexture::GetCpuHandle() const noexcept
	{
		return JGraphic::Instance().ResourceInterface()->GetCpuSrvDescriptorHandle(graphicTextureHandle->GetSrvHeapIndex());
	}
	D3D12_GPU_DESCRIPTOR_HANDLE JTexture::GetGpuHandle()const noexcept
	{
		return JGraphic::Instance().ResourceInterface()->GetGpuSrvDescriptorHandle(graphicTextureHandle->GetSrvHeapIndex());
	}
	Graphic::J_GRAPHIC_TEXTURE_TYPE JTexture::GetTextureType()const noexcept
	{
		return graphicTextureHandle->GetGraphicResourceType();
	}
	int JTexture::GetTextureWidth()const noexcept
	{
		return graphicTextureHandle->GetWidth();
	}
	int JTexture::GetTextureHeight()const noexcept
	{
		return graphicTextureHandle->GetHeight();
	}
	J_RESOURCE_TYPE JTexture::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	} 
	std::string JTexture::GetFormat()const noexcept
	{
		return GetAvailableFormat()[formatIndex];
	}
	std::vector<std::string> JTexture::GetAvailableFormat()noexcept
	{
		static std::vector<std::string> format{ ".jpg",".png",".dds",".tga",".bmp" };
		return format;
	}
	void JTexture::SetTextureType(const Graphic::J_GRAPHIC_TEXTURE_TYPE textureType)noexcept
	{
		if (JTexture::textureType != textureType)
		{
			JTexture::textureType = textureType;
			ClearGraphicTextureHandle();
			UploadTexture();
		}
	}
	void JTexture::DoActivate()noexcept
	{
		JResourceObject::DoActivate(); 
	}
	void JTexture::DoDeActivate()noexcept
	{
		JResourceObject::DoDeActivate();
		ClearGraphicTextureHandle();
	}
	void JTexture::ClearGraphicTextureHandle()
	{
		if (graphicTextureHandle != nullptr)
		{
			JGraphic::Instance().ResourceInterface()->EraseGraphicTextureResource(graphicTextureHandle);
			graphicTextureHandle = nullptr;
			uploadHeap.Reset();
		}
	}
	bool JTexture::UploadTexture()
	{
		if (graphicTextureHandle != nullptr)
			return false;

		if (textureType == Graphic::J_GRAPHIC_TEXTURE_TYPE::TEXTURE_2D)
		{
			graphicTextureHandle = JGraphic::Instance().ResourceInterface()->Create2DTexture(uploadHeap, GetPath());
			if (graphicTextureHandle != nullptr)
				return true;
			else
				return false;
		}
		else if (textureType == Graphic::J_GRAPHIC_TEXTURE_TYPE::TEXTURE_CUBE)
		{
			graphicTextureHandle = JGraphic::Instance().ResourceInterface()->CreateCubeTexture(uploadHeap, GetPath());
			if (graphicTextureHandle != nullptr)
				return true;
			else
				return false;
		}
		return false;
	}
	Core::J_FILE_IO_RESULT JTexture::CallStoreResource()
	{
		return StoreObject(this);
	}
	Core::J_FILE_IO_RESULT JTexture::StoreObject(JTexture* texture)
	{
		if (texture == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (((int)texture->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		std::wofstream stream;
		stream.open(texture->GetMetafilePath(), std::ios::out | std::ios::binary);
		Core::J_FILE_IO_RESULT res = StoreMetadata(stream, texture);
		stream.close();
		return res;
	}
	Core::J_FILE_IO_RESULT JTexture::StoreMetadata(std::wofstream& stream, JTexture* texture)
	{
		if (stream.is_open())
		{
			Core::J_FILE_IO_RESULT res = JResourceObject::StoreMetadata(stream, texture);
			if (res != Core::J_FILE_IO_RESULT::SUCCESS)
				return res;

			stream << (int)texture->GetTextureType();
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JTexture* JTexture::LoadObject(JDirectory* directory, const JResourcePathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;

		if (!JResourceObject::IsResourceFormat<JTexture>(pathData.format))
			return nullptr;

		std::wifstream stream;
		stream.open(ConvertMetafilePath(pathData.wstrPath), std::ios::in | std::ios::binary);
		TextureMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, pathData.folderPath, metadata);
		stream.close();

		JTexture* newTexture;
		if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
			newTexture = new JTexture(pathData.name, metadata.guid, metadata.flag, directory, GetFormatIndex<JTexture>(pathData.format));
		else
			newTexture = new JTexture(pathData.name, Core::MakeGuid(), OBJECT_FLAG_NONE, directory, GetFormatIndex<JTexture>(pathData.format));

		newTexture->textureType = (Graphic::J_GRAPHIC_TEXTURE_TYPE)metadata.textureType;
		newTexture->UploadTexture();
		return newTexture;

	}
	Core::J_FILE_IO_RESULT JTexture::LoadMetadata(std::wifstream& stream, const std::string& folderPath, TextureMetadata& metadata)
	{
		if (stream.is_open())
		{
			Core::J_FILE_IO_RESULT res = JResourceObject::LoadMetadata(stream, metadata);
			if (res != Core::J_FILE_IO_RESULT::SUCCESS)
				return res;

			stream >> metadata.textureType;
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	void JTexture::RegisterFunc()
	{
		auto defaultC = [](JDirectory* owner) ->JResourceObject*
		{
			return new JTexture(owner->MakeUniqueFileName(GetDefaultName<JTexture>()),
				Core::MakeGuid(),
				OBJECT_FLAG_NONE,
				owner,
				JResourceObject::GetDefaultFormatIndex());
		};
		auto initC = [](const std::string& name, const size_t guid, const JOBJECT_FLAG objFlag, JDirectory* directory, const uint8 formatIndex)-> JResourceObject*
		{
			return  new JTexture(name, guid, objFlag, directory, formatIndex);
		};
		auto loadC = [](JDirectory* directory, const JResourcePathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori)->JResourceObject*
		{
			return static_cast<JTexture*>(ori)->CopyResource();
		};

		JRFI<JTexture>::Register(defaultC, initC, loadC, copyC);

		auto getFormatIndexLam = [](const std::string& format) {return JResourceObject::GetFormatIndex<JTexture>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JTexture::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JTexture::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };
		 
		RegisterTypeInfo(RTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, true },
			RTypeUtil{getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable });
	}
	JTexture::JTexture(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const int formatIndex)
		:JResourceObject(name, guid, flag, directory, formatIndex)
	{}
	JTexture::~JTexture(){}
}