#include"JTexture.h"
#include"../JResourceObjectFactory.h"
#include"../../Directory/JDirectory.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Graphic/JGraphic.h" 
#include"../../../Application/JApplicationVariable.h"
#include"../../../../Lib/LoadTextureFromFile.h"
#include"../../../../Lib/DirectX/DDSTextureLoader.h" 
#include<memory>

namespace JinEngine
{
	uint JTexture::GetTextureWidth()const noexcept
	{
		return GetTxtWidth();
	}
	uint JTexture::GetTextureHeight()const noexcept
	{
		return GetTxtHeight();
	}
	Graphic::J_GRAPHIC_TEXTURE_TYPE JTexture::GetTextureType()const noexcept
	{
		return GetTxtType();
	}
	J_RESOURCE_TYPE JTexture::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	} 
	std::wstring JTexture::GetFormat()const noexcept
	{
		return GetAvailableFormat()[GetFormatIndex()];
	}
	std::vector<std::wstring> JTexture::GetAvailableFormat()noexcept
	{
		static std::vector<std::wstring> format{ L".jpg",L".png",L".dds",L".tga",L".bmp" };
		return format;
	}
	void JTexture::SetTextureType(const Graphic::J_GRAPHIC_TEXTURE_TYPE textureType)noexcept
	{
		if (JTexture::textureType != textureType)
		{
			JTexture::textureType = textureType;
			ClearResource();
			StuffResource();
		}
	}
	bool JTexture::Copy(JObject* ori)
	{
		if (ori->HasFlag(OBJECT_FLAG_UNCOPYABLE) || ori->GetGuid() == GetGuid())
			return false;

		if (typeInfo.IsA(ori->GetTypeInfo()))
		{
			JTexture* oriT = static_cast<JTexture*>(oriT);
			CopyRFile(*oriT, *this);
			ClearResource();
			StuffResource();
			return true;
		}
		else
			return false;
	}
	void JTexture::DoActivate()noexcept
	{
		JResourceObject::DoActivate(); 
		StuffResource();
	}
	void JTexture::DoDeActivate()noexcept
	{
		JResourceObject::DoDeActivate();
		ClearResource();
	}
	void JTexture::StuffResource()
	{
		if (!IsValid())
		{
			if (ReadTextureData())
				SetValid(true);
		}
	}
	void JTexture::ClearResource()
	{
		if (IsValid() && HasTxtHandle())
		{
			DestroyTxtHandle();
			uploadHeap.Reset();
			SetValid(false);
		}
	}
	bool JTexture::ReadTextureData()
	{
		if (!HasTxtHandle())
		{
			if (textureType == Graphic::J_GRAPHIC_TEXTURE_TYPE::TEXTURE_2D)
			{
				if (Create2DTexture(uploadHeap, GetPath()))
					return true;
			}
			else if (textureType == Graphic::J_GRAPHIC_TEXTURE_TYPE::TEXTURE_CUBE)
			{
				if (CreateCubeTexture(uploadHeap, GetPath()))
					return true;
			}
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

		JTexture* newTexture = nullptr;
		if (directory->HasFile(pathData.fullName))
			newTexture = JResourceManager::Instance().GetResourceByPath<JTexture>(pathData.wstrPath);

		if (newTexture == nullptr)
		{
			if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
			{
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JTexture>(pathData.name, 
					metadata.guid,
					metadata.flag, 
					directory, 
					GetFormatIndex<JTexture>(pathData.format));
				newTexture = ownerPtr.Get();
				AddInstance(std::move(ownerPtr));
			}
			else
			{
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JTexture>(pathData.name,
					Core::MakeGuid(), 
					OBJECT_FLAG_NONE, 
					directory, 
					GetFormatIndex<JTexture>(pathData.format));
				newTexture = ownerPtr.Get();
				AddInstance(std::move(ownerPtr));
			}
		}
		 
		if (newTexture->IsValid())
			return newTexture;

		newTexture->textureType = (Graphic::J_GRAPHIC_TEXTURE_TYPE)metadata.textureType;
		if (newTexture->ReadTextureData())
			return newTexture;
		else
		{
			newTexture->SetIgnoreUndestroyableFlag(true);
			newTexture->BeginDestroy();
			return nullptr;
		}
	}
	Core::J_FILE_IO_RESULT JTexture::LoadMetadata(std::wifstream& stream, const std::wstring& folderPath, TextureMetadata& metadata)
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
	void JTexture::RegisterJFunc()
	{
		auto defaultC = [](JDirectory* directory) ->JResourceObject*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JTexture>(directory->MakeUniqueFileName(GetDefaultName<JTexture>()),
				Core::MakeGuid(),
				OBJECT_FLAG_NONE,
				directory,
				JResourceObject::GetDefaultFormatIndex());
			JResourceObject* ret = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			return ret;
		};
		auto initC = [](const std::wstring& name, const size_t guid, const J_OBJECT_FLAG objFlag, JDirectory* directory, const uint8 formatIndex)-> JResourceObject*
		{ 
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JTexture>(name, guid, objFlag, directory, formatIndex);
			JResourceObject* ret = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			return ret;
		};
		auto loadC = [](JDirectory* directory, const JResourcePathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori, JDirectory* directory)->JResourceObject*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JTexture>(directory->MakeUniqueFileName(ori->GetName()),
				Core::MakeGuid(),
				ori->GetFlag(),
				directory,
				GetFormatIndex<JTexture>(ori->GetFormat()));

			JTexture* newTexture = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			newTexture->Copy(ori);
			return newTexture;
		};

		JRFI<JTexture>::Register(defaultC, initC, loadC, copyC);

		auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex<JTexture>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JTexture::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JTexture::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };
		 
		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, true, false, false };
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{}); 
	}
	JTexture::JTexture(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const int formatIndex)
		:JTextureInterface(name, guid, flag, directory, formatIndex)
	{}
	JTexture::~JTexture(){}
}