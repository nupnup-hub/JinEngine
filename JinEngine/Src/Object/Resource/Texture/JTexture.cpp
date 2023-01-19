#include"JTexture.h"
#include"../JResourceObjectFactory.h"
#include"../JResourceImporter.h"
#include"../../Directory/JDirectory.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Graphic/JGraphic.h" 
#include"../../../Application/JApplicationVariable.h"
#include"../../../../Lib/LoadTextureFromFile.h"
#include"../../../../Lib/DirectX/DDSTextureLoader.h" 
#include<memory>
#include<fstream>
#include<io.h>

namespace JinEngine
{
	JTexture::JTextureInitData::JTextureInitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		JDirectory* directory,
		const std::wstring oridataPath)
		:JResourceInitData(name, guid, flag, directory, JResourceObject::GetFormatIndex<JTexture>(JCUtil::DecomposeFileFormat(oridataPath))),
		oridataPath(oridataPath)
	{ 
	}
	JTexture::JTextureInitData::JTextureInitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		JDirectory* directory,
		const uint8 formatIndex)
		: JResourceInitData(name, guid, flag, directory, formatIndex), oridataPath(oridataPath)
	{ }
	JTexture::JTextureInitData::JTextureInitData(const std::wstring& name,
		JDirectory* directory,
		const std::wstring oridataPath)
		: JResourceInitData(name, directory, JResourceObject::GetFormatIndex<JTexture>(JCUtil::DecomposeFileFormat(oridataPath))),
		oridataPath(oridataPath)
	{ }

	bool JTexture::JTextureInitData::IsValidCreateData()
	{
		if (JResourceInitData::IsValidCreateData() && _waccess(oridataPath.c_str(), 00) != -1)
			return true;
		else
			return false;
	}
	J_RESOURCE_TYPE JTexture::JTextureInitData::GetResourceType() const noexcept
	{
		return J_RESOURCE_TYPE::TEXTURE;
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
	uint JTexture::GetTextureWidth()const noexcept
	{
		return GetResourceWidth();
	}
	uint JTexture::GetTextureHeight()const noexcept
	{
		return GetResourceHeight();
	}
	Graphic::J_GRAPHIC_RESOURCE_TYPE JTexture::GetTextureType()const noexcept
	{
		return textureType;
	}
	void JTexture::SetTextureType(const Graphic::J_GRAPHIC_RESOURCE_TYPE textureType)noexcept
	{
		if (JTexture::textureType != textureType)
		{
			JTexture::textureType = textureType;
			if (IsValid())
			{
				ClearResource();
				StuffResource();
			}
		}
	}
	JTextureImportInterface* JTexture::ImportInterface()noexcept
	{
		return this;
	}
	void JTexture::DoCopy(JObject* ori)
	{
		JTexture* oriT = static_cast<JTexture*>(ori);
		CopyRFile(*oriT);
		ClearResource();
		SetTextureType(oriT->GetTextureType());
		StuffResource();
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
		if (IsValid())
		{ 
			if(HasTxtHandle())
				DestroyTxtHandle();
			uploadBuffer.Reset();
			SetValid(false);
		}
	}
	bool JTexture::ReadTextureData()
	{
		if (!HasTxtHandle())
		{ 
			if (textureType == Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D)
			{
				if (Create2DTexture(uploadBuffer, GetPath(), GetFormat()))
				{
					//uploadBuffer.Reset();
					return true;
				}
			}
			else if (textureType == Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)
			{ 
				if (CreateCubeMap(uploadBuffer, GetPath(), GetFormat()))
				{
					//uploadBuffer.Reset();
					return true;
				}
			}
		}
		return false;
	}
	bool JTexture::ImportTexture(const std::wstring& oriPath)
	{
		if (_waccess(oriPath.c_str(), 00) == -1)
			return false;

		std::wstring folderPath;
		std::wstring name;
		std::wstring format;
		JCUtil::DecomposeFilePath(oriPath, folderPath, name, format);

		int formatIndex = GetFormatIndex<JTexture>(format);
		if (formatIndex != GetFormatIndex() || formatIndex == GetInvalidFormatIndex())
			return false;

		std::ifstream source(oriPath, std::ios::binary);
		std::ofstream dest(GetPath(), std::ios::binary);

		dest << source.rdbuf();
		source.close();
		dest.close();

		StoreObject(this);
		return true;
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

			JFileIOHelper::StoreEnumData(stream, L"TextureType:", texture->GetTextureType());
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JTexture* JTexture::LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;

		std::wifstream stream;
		stream.open(pathData.engineMetaFileWPath, std::ios::in | std::ios::binary);
		JTextureMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);
		stream.close();

		JTexture* newTexture = nullptr;
		if (directory->HasFile(pathData.name))
			newTexture = JResourceManager::Instance().GetResourceByPath<JTexture>(pathData.engineFileWPath);

		if (newTexture == nullptr && loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
		{
			JTextureInitData initdata{ pathData.name, metadata.guid,metadata.flag, directory, (uint8)metadata.formatIndex };
			if (initdata.IsValidLoadData())
			{
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JTexture>(initdata);
				newTexture = ownerPtr.Get();
				if (!AddInstance(std::move(ownerPtr)))
					return nullptr;
			}
		}
		if(newTexture != nullptr)
			newTexture->textureType = metadata.textureType;
		return newTexture;
	}
	Core::J_FILE_IO_RESULT JTexture::LoadMetadata(std::wifstream& stream, JTextureMetadata& metadata)
	{ 
		if (stream.is_open())
		{
			Core::J_FILE_IO_RESULT res = JResourceObject::LoadMetadata(stream, metadata);
			if (res != Core::J_FILE_IO_RESULT::SUCCESS)
				return res;
			  
			JFileIOHelper::LoadEnumData(stream, metadata.textureType);
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	void JTexture::RegisterJFunc()
	{ 
		auto defaultC = [](Core::JOwnerPtr<JResourceInitData> initdata) -> JResourceObject*
		{
			if (initdata.IsValid() && initdata->GetResourceType() == J_RESOURCE_TYPE::TEXTURE && initdata->IsValidCreateData())
			{
				JTextureInitData* tInitdata = static_cast<JTextureInitData*>(initdata.Get());
				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JTexture>(*tInitdata);
				JTexture* newTexture = ownerPtr.Get();
			 
				if (AddInstance(std::move(ownerPtr)))
				{ 
					newTexture->ImportTexture(tInitdata->oridataPath); 
					return newTexture;
				}
			}
			return nullptr;
		};
		auto loadC = [](JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori, JDirectory* directory)->JResourceObject*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JTexture>(InitData(ori->GetName(),
				Core::MakeGuid(),
				ori->GetFlag(),
				directory,
				GetFormatIndex<JTexture>(ori->GetFormat())));

			JTexture* newTexture = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				newTexture->Copy(ori);
				return newTexture;
			}
			else
				return nullptr;
		};

		JRFI<JTexture>::Register(defaultC, loadC, copyC);

		auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex<JTexture>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JTexture::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JTexture::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, true, false};
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{});

		auto txtImportC = [](JDirectory* dir, const Core::JFileImportHelpData importPathData) -> std::vector<JResourceObject*>
		{
			return { JRFI<JTexture>::Create(Core::JPtrUtil::MakeOwnerPtr<InitData>(importPathData.name,
				Core::MakeGuid(), 
				importPathData.flag,
				dir, 
				importPathData.oriFileWPath)) };
		};

		JResourceImporter::Instance().AddFormatInfo(L".jpg", J_RESOURCE_TYPE::TEXTURE, txtImportC);
		JResourceImporter::Instance().AddFormatInfo(L".png", J_RESOURCE_TYPE::TEXTURE, txtImportC);
		JResourceImporter::Instance().AddFormatInfo(L".dds", J_RESOURCE_TYPE::TEXTURE, txtImportC);
		JResourceImporter::Instance().AddFormatInfo(L".tga", J_RESOURCE_TYPE::TEXTURE, txtImportC);
		JResourceImporter::Instance().AddFormatInfo(L".bmp", J_RESOURCE_TYPE::TEXTURE, txtImportC);
	}
	JTexture::JTexture(const JTextureInitData& initdata)
		: JTextureInterface(initdata)
	{}
	JTexture::~JTexture() {}
}