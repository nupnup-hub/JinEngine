#include"JTexture.h"
#include"JTexturePrivate.h"
#include"../JClearableInterface.h"
#include"../JResourceObjectHint.h"
#include"../JResourceObjectImporter.h"
#include"../JResourceObjectEventDesc.h"
#include"../../Directory/JDirectory.h"
#include"../../JObjectFileIOHelper.h"
#include"../../../Core/Identity/JIdenCreator.h"
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/Guid/JGuidCreator.h" 
#include"../../../Core/Utility/JCommonUtility.h"
#include"../../../Graphic/JGraphic.h" 
#include"../../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../../Application/JApplicationProject.h" 
#include<memory>
#include<fstream>
#include<io.h> 

namespace JinEngine
{
	namespace
	{
		static JTexturePrivate tPrivate;
	}
	namespace Private
	{
		static Graphic::JMipmapGenerateDesc InitMipmapGenerateDesc()
		{
			Graphic::JMipmapGenerateDesc desc;
			desc.type = Graphic::J_GRAPHIC_MIP_MAP_TYPE::GRAPHIC_API_DEFAULT;
			desc.kernelSize = Graphic::J_KENEL_SIZE::_3x3;
			desc.sharpnessFactor = 2.5f;
			return desc;
		}
		 
		static constexpr float minMipmapSharpness = 0.001f;
		static constexpr float maxMipmapSharpness = 32;
	}
	

	class JTexture::JTextureImpl : public Core::JTypeImplBase,
		public JClearableInterface, 
		public Graphic::JGraphicSingleResourceHolder
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JTextureImpl) 
	public:
		JWeakPtr<JTexture> thisPointer = nullptr;
	public: 
		Graphic::J_GRAPHIC_RESOURCE_TYPE textureType = Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D;
		REGISTER_PROPERTY_EX(resolution, GetTextureResolution, SetTextureResolution, GUI_ENUM_COMBO(J_TEXTURE_RESOLUTION, "-a {v} x {v}; -c {v} != 0;"))
		J_TEXTURE_RESOLUTION resolution = J_TEXTURE_RESOLUTION::ORIGINAL;
	public:
		Graphic::JMipmapGenerateDesc mipMapGenerateDesc = Private::InitMipmapGenerateDesc();
		REGISTER_GUI_ENUM_CONDITION(TextureMipmapType, Graphic::J_GRAPHIC_MIP_MAP_TYPE, GetMipmapType, true)
		REGISTER_METHOD_GUI_WIDGET(MipmapType, GetMipmapType, SetMipmapType, GUI_ENUM_COMBO(Graphic::J_GRAPHIC_MIP_MAP_TYPE))
		REGISTER_METHOD_GUI_WIDGET(MipmapKernelSize, GetMipmapKernelSize, SetMipmapKernelSize, GUI_ENUM_COMBO(Graphic::J_KENEL_SIZE))
		REGISTER_METHOD_GUI_WIDGET(MipmapSharpness, GetMipmapSharpnessFactor, SetMipmapSharpnessFactor, GUI_SLIDER(Private::minMipmapSharpness, Private::maxMipmapSharpness, true))
	public:
		JTextureImpl(const InitData& initData, JTexture* thisTexRaw)
		{
			if (IsValidTextureType(initData.textureType))
				textureType = initData.textureType;
			else
				textureType = Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D;
			resolution = initData.resoultion;
			mipMapGenerateDesc = initData.mipMapDesc;
		}
		~JTextureImpl()
		{ 
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
		J_TEXTURE_RESOLUTION GetTextureResolution()const noexcept
		{
			return resolution;
		}
		Graphic::J_GRAPHIC_MIP_MAP_TYPE GetMipmapType()const noexcept
		{
			return mipMapGenerateDesc.type;
		}
		Graphic::J_KENEL_SIZE GetMipmapKernelSize()const noexcept
		{
			return mipMapGenerateDesc.kernelSize;
		}
		float GetMipmapSharpnessFactor()const noexcept
		{
			return mipMapGenerateDesc.sharpnessFactor;
		}
	public:
		void SetTextureType(const Graphic::J_GRAPHIC_RESOURCE_TYPE newTextureType)noexcept
		{
			if (textureType != newTextureType)
			{
				textureType = newTextureType;
				if (thisPointer->IsValid())
				{
					ClearResource();
					StuffResource();
				}
			}
		}
		void SetTextureResolution(const J_TEXTURE_RESOLUTION newResolution)
		{
			if (resolution == newResolution)
				return;

			resolution = newResolution;
			if (HasGraphicResourceHandle())
			{
				DestroyGraphicResource();
				ReadTextureData();
				JResourceObjectPrivate::EventInterface::NotifyEvent(thisPointer.Get(),
					J_RESOURCE_EVENT_TYPE::UPDATE_RESOURCE,
					std::make_unique<JResourceUpdateEvDesc>(JResourceUpdateEvDesc::USER_ACTION::UPDATE_USER_AND_REAR_OF_FRAME_BUFFER));
			}	
		}
		void SetMipmapDesc(const Graphic::JMipmapGenerateDesc& newDesc)
		{
			if (newDesc == mipMapGenerateDesc)
				return;

			const bool changedExceptType =(mipMapGenerateDesc.kernelSize != newDesc.kernelSize ||
				mipMapGenerateDesc.sharpnessFactor != newDesc.sharpnessFactor) && mipMapGenerateDesc.type == newDesc.type;
			const bool denyModifyMipmap = changedExceptType && mipMapGenerateDesc.type != Graphic::J_GRAPHIC_MIP_MAP_TYPE::GAUSSIAN && 
				mipMapGenerateDesc.type != Graphic::J_GRAPHIC_MIP_MAP_TYPE::KAISER;

			if (HasGraphicResourceHandle() && !denyModifyMipmap)
			{
				if (newDesc.type == Graphic::J_GRAPHIC_MIP_MAP_TYPE::NONE || newDesc.type == Graphic::J_GRAPHIC_MIP_MAP_TYPE::GRAPHIC_API_DEFAULT)
				{
					mipMapGenerateDesc = newDesc;
					DestroyGraphicResource();
					ReadTextureData();
					JResourceObjectPrivate::EventInterface::NotifyEvent(thisPointer.Get(),
						J_RESOURCE_EVENT_TYPE::UPDATE_RESOURCE,
						std::make_unique<JResourceUpdateEvDesc>(JResourceUpdateEvDesc::USER_ACTION::UPDATE_USER_AND_REAR_OF_FRAME_BUFFER));
				}
				else
				{
					auto createDesc = CreateTextureCreateDesc();
					createDesc.mipMapDesc = newDesc;

					if (SetMipmap(GetFirstInfo(), createDesc))
						mipMapGenerateDesc = newDesc;
				}
			}
			else
				mipMapGenerateDesc = newDesc;
		}
		void SetMipmapType(const Graphic::J_GRAPHIC_MIP_MAP_TYPE mipmapType)noexcept
		{
			Graphic::JMipmapGenerateDesc newDesc = mipMapGenerateDesc;
			newDesc.type = mipmapType;
			SetMipmapDesc(newDesc);
		}
		void SetMipmapKernelSize(const Graphic::J_KENEL_SIZE kenelType)noexcept
		{
			Graphic::JMipmapGenerateDesc newDesc = mipMapGenerateDesc;
			newDesc.kernelSize = kenelType;
			SetMipmapDesc(newDesc);
		}
		void SetMipmapSharpnessFactor(const float sharpness)noexcept
		{
			Graphic::JMipmapGenerateDesc newDesc = mipMapGenerateDesc;
			newDesc.sharpnessFactor = std::clamp(sharpness, Private::minMipmapSharpness, Private::maxMipmapSharpness);
			SetMipmapDesc(newDesc);
		}
	public:
		static bool IsValidTextureType(const Graphic::J_GRAPHIC_RESOURCE_TYPE type)
		{
			return type == Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D || type == Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE;
		}
	private:
		void AdjustResoultion()noexcept
		{
			if (resolution == J_TEXTURE_RESOLUTION::ORIGINAL)
				return;

			auto firstInfo = GetFirstInfo();
			if (firstInfo == nullptr)
				return;
			 
			const uint bigOne = max(firstInfo->GetWidth(), firstInfo->GetHeight());
			auto elementValueVec = Core::GetEnumElementValueVec<J_TEXTURE_RESOLUTION>();
			for (const auto& data : elementValueVec)
			{ 
				if (data == (uint)J_TEXTURE_RESOLUTION::ORIGINAL)
					continue;

				if (data >= bigOne)
				{
					resolution = (J_TEXTURE_RESOLUTION)data;
					break;
				}
			}
			
		}
		void AdjustMipmap()
		{
			mipMapGenerateDesc.type = GetFirstInfo()->GetMipmapType();
		}
	public:
		void StuffResource()
		{
			if (!thisPointer->IsValid())
			{
				if (ReadTextureData())
					thisPointer->SetValid(true);
			}
		}
		void ClearResource()
		{
			if (thisPointer->IsValid())
			{
				if (HasGraphicResourceHandle())
					DestroyGraphicResource(); 
				thisPointer->SetValid(false);
			}
		}
		Graphic::JTextureCreateDesc CreateTextureCreateDesc()
		{
			return Graphic::JTextureCreateDesc(thisPointer->GetPath(), thisPointer->GetFormat(), (uint)resolution, CreateMipmapGenerateDesc());
		}
		Graphic::JMipmapGenerateDesc CreateMipmapGenerateDesc()
		{
			return mipMapGenerateDesc;
		}
	public:
		bool ReadTextureData()
		{
			if (!HasGraphicResourceHandle())
			{ 
				if (textureType == Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_2D)
				{
					if (Create2DTexture(CreateTextureCreateDesc()))
					{ 
						AdjustResoultion();
						AdjustMipmap();
						return true;
					}
				}
				else if (textureType == Graphic::J_GRAPHIC_RESOURCE_TYPE::TEXTURE_CUBE)
				{
					if (CreateCubeMap(CreateTextureCreateDesc()))
					{
						AdjustResoultion();
						AdjustMipmap();
						return true;
					}
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

			int formatIndex = thisPointer->GetFormatIndex(thisPointer->GetResourceType(),format);
			if (formatIndex != thisPointer->GetFormatIndex() || formatIndex == GetInvalidFormatIndex())
				return false;

			std::ifstream source(oriPath, std::ios::in | std::ios::binary);
			std::ofstream dest(thisPointer->GetPath(), std::ios::out | std::ios::binary);

			dest << source.rdbuf();
			source.close();
			dest.close();
			 
			return true;
		}
	public:
		void NotifyReAlloc()
		{		 
			RegisterInterfacePointer();
		}
	public:
		void Initialize(InitData* initData)
		{
			//객체를 처음생성할때 initData에서 import
			if (!thisPointer->HasFile())
				ImportTexture(initData->oridataPath);
			ReadTextureData();
			thisPointer->SetValid(true);
		}
		void RegisterThisPointer(JTexture* tex)
		{
			thisPointer = Core::GetWeakPtr(tex);  
		}
		void RegisterInterfacePointer()
		{
			Graphic::JGraphicResourceInterface::SetInterfacePointer(this); 
		}
		void DeRegisterInterfacePointer()
		{
			Graphic::JGraphicResourceInterface::SetInterfacePointer(nullptr);
		}
		static void RegisterTypeData()
		{
			auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex(GetStaticResourceType(), format); };

			static GetRTypeInfoCallable getTypeInfoCallable{ &JTexture::StaticTypeInfo };
			static GetAvailableFormatCallable getAvailableFormatCallable{ &JTexture::GetAvailableFormat };
			static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

			static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, true, false, false, true };
			static RTypeCommonFunc rTypeCFunc{ getTypeInfoCallable, getAvailableFormatCallable, getFormatIndexCallable };

			RegisterRTypeInfo(rTypeHint, rTypeCFunc, RTypePrivateFunc{});

			auto txtImportC = [](JUserPtr<JDirectory> dir, const Core::JFileImportHelpData importPathData) -> std::vector<JUserPtr<JResourceObject>>
			{
				return { JICI::Create<JTexture>(importPathData.name,
					Core::MakeGuid(),
					(J_OBJECT_FLAG)importPathData.flag,
					RTypeCommonCall::CallFormatIndex(GetStaticResourceType(), importPathData.format),
					dir,
					importPathData.oriFileWPath) };
			};

			auto foramatVec = JTexture::GetAvailableFormat();
			for(const auto& data: foramatVec)
				JResourceObjectImporter::Instance().AddFormatInfo(data, J_RESOURCE_TYPE::TEXTURE, txtImportC);
			//JResourceObjectImporter::Instance().AddFormatInfo(L".png", J_RESOURCE_TYPE::TEXTURE, txtImportC);
			//JResourceObjectImporter::Instance().AddFormatInfo(L".dds", J_RESOURCE_TYPE::TEXTURE, txtImportC);
			//JResourceObjectImporter::Instance().AddFormatInfo(L".tga", J_RESOURCE_TYPE::TEXTURE, txtImportC);
			//JResourceObjectImporter::Instance().AddFormatInfo(L".bmp", J_RESOURCE_TYPE::TEXTURE, txtImportC);

			Core::JIdentifier::RegisterPrivateInterface(JTexture::StaticTypeInfo(), tPrivate);
			IMPL_REALLOC_BIND(JTexture::JTextureImpl, thisPointer)
		}
	};

	JTexture::InitData::InitData(const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		const std::wstring oridataPath,
		Graphic::J_GRAPHIC_RESOURCE_TYPE textureType)
		:JResourceObject::InitData(JTexture::StaticTypeInfo(), formatIndex, GetStaticResourceType(), directory),
		oridataPath(oridataPath), textureType(textureType)
	{}
	JTexture::InitData::InitData(const size_t guid,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		const std::wstring oridataPath,
		Graphic::J_GRAPHIC_RESOURCE_TYPE textureType)
		:JResourceObject::InitData(JTexture::StaticTypeInfo(), guid, formatIndex, GetStaticResourceType(), directory),
		oridataPath(oridataPath), textureType(textureType)
	{}
	JTexture::InitData::InitData(const std::wstring& name,
		const size_t guid,
		const J_OBJECT_FLAG flag,
		const uint8 formatIndex,
		const JUserPtr<JDirectory>& directory,
		const std::wstring oridataPath,
		Graphic::J_GRAPHIC_RESOURCE_TYPE textureType)
		: JResourceObject::InitData(JTexture::StaticTypeInfo(), name, guid, flag, formatIndex, GetStaticResourceType(), directory),
		oridataPath(oridataPath), textureType(textureType)
	{}
	bool JTexture::InitData::IsValidData()const noexcept
	{ 
		return JResourceObject::InitData::IsValidData() && _waccess(oridataPath.c_str(), 00) != -1;
	}

	JTexture::LoadMetaData::LoadMetaData(const JUserPtr<JDirectory>& directory)
		:JResourceObject::InitData(JTexture::StaticTypeInfo(), GetDefaultFormatIndex(), GetStaticResourceType(), directory)
	{}
 
	Core::JIdentifierPrivate& JTexture::PrivateInterface()const noexcept
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
		static std::vector<std::wstring> format{ L".jpg",L".png",L".dds",L".tga",L".bmp", L".tif", L".tiff"};
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
	J_TEXTURE_RESOLUTION JTexture::GetTextureResolution()const noexcept
	{
		return impl->GetTextureResolution();
	}
	void JTexture::SetTextureType(const Graphic::J_GRAPHIC_RESOURCE_TYPE textureType)noexcept
	{
		impl->SetTextureType(textureType);
	} 
	void JTexture::SetTextureResolution(const J_TEXTURE_RESOLUTION resolutionType)noexcept
	{
		impl->SetTextureResolution(resolutionType);
	}
	void JTexture::DoActivate()noexcept
	{	
		JResourceObject::DoActivate();
		impl->StuffResource();
	}
	void JTexture::DoDeActivate()noexcept
	{ 
		impl->ClearResource();
		JResourceObject::DoDeActivate();
	}
	JTexture::JTexture(const InitData& initData)
		: JResourceObject(initData), impl(std::make_unique<JTextureImpl>(initData, this))
	{  }
	JTexture::~JTexture()
	{  
		impl.reset();
	}
 
	using CreateInstanceInterface = JTexturePrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JTexturePrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JTexturePrivate::AssetDataIOInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JTexture>(*static_cast<JTexture::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JResourceObjectPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JTexture* txt = static_cast<JTexture*>(createdPtr);
		txt->impl->RegisterThisPointer(txt);
		txt->impl->RegisterInterfacePointer();
		txt->impl->Initialize(static_cast<JTexture::InitData*>(initData));
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JTexture::InitData::StaticTypeInfo());	
		return isValidPtr && initData->IsValidData();
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		JTexture* txt = static_cast<JTexture*>(ptr);
		txt->impl->DeRegisterInterfacePointer();
		JResourceObjectPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JTexture::LoadData::StaticTypeInfo()))
			return nullptr;
 
		auto loadData = static_cast<JTexture::LoadData*>(data);
		auto pathData = loadData->pathData;
		JUserPtr<JDirectory> directory = loadData->directory;
		JTexture::LoadMetaData metadata(loadData->directory);

		if (LoadMetaData(pathData.engineMetaFileWPath, &metadata) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;
 
		JUserPtr<JTexture> newTex = nullptr;
		if (directory->HasFile(metadata.guid))
			newTex = Core::GetUserPtr<JTexture>(JTexture::StaticTypeInfo().TypeGuid(), metadata.guid);

		if (newTex == nullptr)
		{
			std::unique_ptr<JTexture::InitData> initData = std::make_unique<JTexture::InitData>(pathData.name,
				metadata.guid,
				metadata.flag,
				(uint8)metadata.formatIndex,
				directory,
				pathData.engineFileWPath,
				metadata.textureType);

			initData->resoultion = metadata.resoultion;
			initData->mipMapDesc = metadata.mipMapDesc;

			auto idenUser = tPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &tPrivate);
			newTex.ConnnectChild(idenUser); 
		}
		return newTex;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{ 
		//asset data는 임포트시 한번 저장되고 이후에 저장되지 않는다 (fixed)
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JTexture::LoadMetaData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JFileIOTool tool;
		if (!tool.Begin(path, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto loadMetaData = static_cast<JTexture::LoadMetaData*>(data);
		if (LoadCommonMetaData(tool, loadMetaData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
		 
		JObjectFileIOHelper::LoadEnumData(tool, loadMetaData->textureType, "TextureType"); 
		JObjectFileIOHelper::LoadEnumData(tool, loadMetaData->resoultion, "Resoultion");
		JObjectFileIOHelper::LoadEnumData(tool, loadMetaData->mipMapDesc.type, "MipmapType");
		JObjectFileIOHelper::LoadEnumData(tool, loadMetaData->mipMapDesc.kernelSize, "MipmapKernelSize");
		JObjectFileIOHelper::LoadAtomicData(tool, loadMetaData->mipMapDesc.sharpnessFactor, "MipmapSharpnessFactor");
		tool.Close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JTexture::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JTexture::StoreData*>(data);
		JUserPtr<JTexture> tex = Core::ConnectChildUserPtr<JTexture>(storeData->obj);

		JFileIOTool tool;
		if (!tool.Begin(tex->GetMetaFilePath(), JFileIOTool::TYPE::JSON))
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (StoreCommonMetaData(tool, storeData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	  
		JObjectFileIOHelper::StoreEnumData(tool, tex->GetTextureType(), "TextureType");
		JObjectFileIOHelper::StoreEnumData(tool, tex->GetTextureResolution(), "Resoultion");
		JObjectFileIOHelper::StoreEnumData(tool, tex->impl->mipMapGenerateDesc.type, "MipmapType");
		JObjectFileIOHelper::StoreEnumData(tool, tex->impl->mipMapGenerateDesc.kernelSize, "MipmapKernelSize");
		JObjectFileIOHelper::StoreAtomicData(tool, tex->impl->mipMapGenerateDesc.sharpnessFactor, "MipmapSharpnessFactor");
		tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
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