#include"JTexture.h"
#include"JTexturePrivate.h"
#include"../JClearableInterface.h"
#include"../JResourceObjectHint.h"
#include"../JResourceObjectImporter.h"
#include"../../Directory/JDirectory.h"
#include"../../../Core/Identity/JIdenCreator.h"
#include"../../../Core/Identity/JIdentifierImplBase.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Utility/JCommonUtility.h"
#include"../../../Graphic/JGraphic.h" 
#include"../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../../Application/JApplicationProject.h" 
#include<memory>
#include<fstream>
#include<io.h>
#include<d3d12.h>

namespace JinEngine
{
	namespace
	{
		static JTexturePrivate tPrivate;
	}
 
	class JTexture::JTextureImpl : public Core::JIdentifierImplBase,
		public JClearableInterface, 
		public Graphic::JGraphicResourceInterface
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JTextureImpl)
	public:
		JTexture* thisTex = nullptr; 
	public:
		Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer = nullptr;
		Graphic::J_GRAPHIC_RESOURCE_TYPE textureType = Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D;
	public:
		JTextureImpl(const InitData& initData, JTexture* thisTex)
			:thisTex(thisTex)
		{
			if (IsValidTextureType(initData.textureType))
				textureType = initData.textureType;
			else
				textureType = Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D;
		}
		~JTextureImpl() {}
	public:
		void Initialize(const InitData& initData)
		{
			if (!thisTex->HasFile())
				ImportTexture(initData.oridataPath);
			ReadTextureData();
			thisTex->SetValid(true);
		}
	public:
		uint GetTextureWidth()const noexcept
		{
			return GetResourceWidth();
		}
		uint GetTextureHeight()const noexcept
		{
			return GetResourceHeight();
		} 
	public:
		void SetTextureType(const Graphic::J_GRAPHIC_RESOURCE_TYPE newTextureType)noexcept
		{
			if (textureType != newTextureType)
			{
				textureType = newTextureType;
				if (thisTex->IsValid())
				{
					ClearResource();
					StuffResource();
				}
			}
		}
	public:
		static bool IsValidTextureType(const Graphic::J_GRAPHIC_RESOURCE_TYPE type)
		{
			return type == Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D || type == Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE;
		}
	public:
		void StuffResource()
		{
			if (!thisTex->IsValid())
			{
				if (ReadTextureData())
					thisTex->SetValid(true);
			}
		}
		void ClearResource()
		{
			if (thisTex->IsValid())
			{
				if (HasTxtHandle())
					DestroyTexture();
				uploadBuffer.Reset();
				thisTex->SetValid(false);
			}
		}
	public:
		bool ReadTextureData()
		{
			if (!HasTxtHandle())
			{
				if (textureType == Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D)
				{
					if (Create2DTexture(uploadBuffer, thisTex->GetPath(), thisTex->GetFormat()))
						return true;
				}
				else if (textureType == Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)
				{
					if (CreateCubeMap(uploadBuffer, thisTex->GetPath(), thisTex->GetFormat()))
						return true;
				}
			}
			return false;
		}
		bool ImportTexture(const std::wstring& oriPath)
		{
			if (_waccess(oriPath.c_str(), 00) == -1)
				return false;

			std::wstring folderPath;
			std::wstring name;
			std::wstring format;
			JCUtil::DecomposeFilePath(oriPath, folderPath, name, format);

			int formatIndex = thisTex->GetFormatIndex(thisTex->GetResourceType(),format);
			if (formatIndex != thisTex->GetFormatIndex() || formatIndex == GetInvalidFormatIndex())
				return false;

			std::ifstream source(oriPath, std::ios::in | std::ios::binary);
			std::ofstream dest(thisTex->GetPath(), std::ios::out | std::ios::binary);

			dest << source.rdbuf();
			source.close();
			dest.close();
			 
			return true;
		}
	public:
		static void RegisterCallOnce()
		{
			auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex(GetStaticResourceType(), format); };

			static GetRTypeInfoCallable getTypeInfoCallable{ &JTexture::StaticTypeInfo };
			static GetAvailableFormatCallable getAvailableFormatCallable{ &JTexture::GetAvailableFormat };
			static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

			static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, true, false, false, true };
			static RTypeCommonFunc rTypeCFunc{ getTypeInfoCallable, getAvailableFormatCallable, getFormatIndexCallable };

			RegisterRTypeInfo(rTypeHint, rTypeCFunc, RTypePrivateFunc{});

			auto txtImportC = [](JDirectory* dir, const Core::JFileImportHelpData importPathData) -> std::vector<JResourceObject*>
			{
				return { JICI::Create<JTexture>(importPathData.name,
					Core::MakeGuid(),
					importPathData.flag,
					RTypeCommonCall::CallFormatIndex(GetStaticResourceType(), importPathData.format),
					dir,
					importPathData.oriFileWPath) };
			};

			JResourceObjectImporter::Instance().AddFormatInfo(L".jpg", J_RESOURCE_TYPE::TEXTURE, txtImportC);
			JResourceObjectImporter::Instance().AddFormatInfo(L".png", J_RESOURCE_TYPE::TEXTURE, txtImportC);
			JResourceObjectImporter::Instance().AddFormatInfo(L".dds", J_RESOURCE_TYPE::TEXTURE, txtImportC);
			JResourceObjectImporter::Instance().AddFormatInfo(L".tga", J_RESOURCE_TYPE::TEXTURE, txtImportC);
			JResourceObjectImporter::Instance().AddFormatInfo(L".bmp", J_RESOURCE_TYPE::TEXTURE, txtImportC);
			Core::JIdentifier::RegisterPrivateInterface(JTexture::StaticTypeInfo(), tPrivate);
		}
	};

	JTexture::InitData::InitData(const uint8 formatIndex,
		JDirectory* directory,
		const std::wstring oridataPath,
		Graphic::J_GRAPHIC_RESOURCE_TYPE textureType)
		:JResourceObject::InitData(JTexture::StaticTypeInfo(), formatIndex, GetStaticResourceType(), directory),
		oridataPath(oridataPath), textureType(textureType)
	{}
	JTexture::InitData::InitData(const size_t guid,
		const uint8 formatIndex,
		JDirectory* directory,
		const std::wstring oridataPath,
		Graphic::J_GRAPHIC_RESOURCE_TYPE textureType)
		:JResourceObject::InitData(JTexture::StaticTypeInfo(), guid, formatIndex, GetStaticResourceType(), directory),
		oridataPath(oridataPath), textureType(textureType)
	{}
	JTexture::InitData::InitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex,
		JDirectory* directory,
		const std::wstring oridataPath,
		Graphic::J_GRAPHIC_RESOURCE_TYPE textureType)
		: JResourceObject::InitData(JTexture::StaticTypeInfo(), name, guid, flag, formatIndex, GetStaticResourceType(), directory),
		oridataPath(oridataPath), textureType(textureType)
	{}
	bool JTexture::InitData::IsValidData()const noexcept
	{
		return JResourceObject::InitData::IsValidData() && _waccess(oridataPath.c_str(), 00) != -1;
	}

	JTexture::LoadMetaData::LoadMetaData(JDirectory* directory)
		:JResourceObject::InitData(JTexture::StaticTypeInfo(), GetDefaultFormatIndex(), GetStaticResourceType(), directory)
	{}
 
	Core::JIdentifierPrivate& JTexture::GetPrivateInterface()const noexcept
	{
		return tPrivate;
	}
	const Graphic::JGraphicResourceUserInterface JTexture::GraphicResourceUserInterface()const noexcept
	{
		return Graphic::JGraphicResourceUserInterface{ impl.get() };
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
		return impl->GetTextureWidth();
	}
	uint JTexture::GetTextureHeight()const noexcept
	{
		return impl->GetTextureHeight();
	}
	Graphic::J_GRAPHIC_RESOURCE_TYPE JTexture::GetTextureType()const noexcept
	{
		return impl->textureType;
	}
	void JTexture::SetTextureType(const Graphic::J_GRAPHIC_RESOURCE_TYPE textureType)noexcept
	{
		impl->SetTextureType(textureType);
	} 
	void JTexture::DoActivate()noexcept
	{	
		JResourceObject::DoActivate();
		impl->StuffResource();
	}
	void JTexture::DoDeActivate()noexcept
	{ 
		JResourceObject::DoDeActivate();
		impl->ClearResource();
	}
	JTexture::JTexture(const InitData& initData)
		: JResourceObject(initData), impl(std::make_unique<JTextureImpl>(initData, this))
	{
		impl->Initialize(initData);
	}
	JTexture::~JTexture()
	{
		impl.reset();
	}
 
	using CreateInstanceInterface = JTexturePrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JTexturePrivate::AssetDataIOInterface;

	Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JTexture>(*static_cast<JTexture::InitData*>(initData.get()));
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JTexture::InitData::StaticTypeInfo());	
		return isValidPtr && initData->IsValidData();
	}

	Core::JIdentifier* AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JTexture::LoadData::StaticTypeInfo()))
			return nullptr;
 
		auto loadData = static_cast<JTexture::LoadData*>(data);
		auto pathData = loadData->pathData;
		JDirectory* directory = loadData->directory;
		JTexture::LoadMetaData metadata(loadData->directory);

		if (LoadMetaData(pathData.engineMetaFileWPath, &metadata) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;

		JTexture* newTex = nullptr;
		if (directory->HasFile(metadata.guid))
			newTex = static_cast<JTexture*>(Core::GetUserPtr(JTexture::StaticTypeInfo().TypeGuid(), metadata.guid).Get());

		if (newTex == nullptr)
		{
			std::unique_ptr<JTexture::InitData> initData = std::make_unique<JTexture::InitData>(pathData.name,
				metadata.guid,
				metadata.flag,
				(uint8)metadata.formatIndex,
				directory,
				pathData.engineFileWPath,
				metadata.textureType);

			auto rawPtr = tPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &tPrivate);
			newTex = static_cast<JTexture*>(rawPtr);
		}
		if (newTex != nullptr)
			newTex->impl->textureType = metadata.textureType;
		return newTex;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{ 
		//asset data�� ����Ʈ�� �ѹ� ����ǰ� ���Ŀ� ������� �ʴ´� (fixed)
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JTexture::LoadMetaData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		std::wifstream stream;
		stream.open(path, std::ios::in | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto loadMetaData = static_cast<JTexture::LoadMetaData*>(data);
		if (LoadCommonMetaData(stream, loadMetaData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JFileIOHelper::LoadEnumData(stream, loadMetaData->textureType);
		stream.close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JTexture::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JTexture::StoreData*>(data);
		JTexture* tex = static_cast<JTexture*>(storeData->obj);

		std::wofstream stream;
		stream.open(tex->GetMetaFilePath(), std::ios::out | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (StoreCommonMetaData(stream, storeData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	 
		JFileIOHelper::StoreEnumData(stream, L"TextureType:", tex->GetTextureType());
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JTexturePrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	JResourceObjectPrivate::AssetDataIOInterface& JTexturePrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}