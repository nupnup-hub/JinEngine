/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#include"JShader.h" 
#include"JShaderPrivate.h" 
#include"../JResourceObjectHint.h"
#include"../JResourceManager.h" 
#include"../../Directory/JDirectory.h"
#include"../../Directory/JFile.h"
#include"../../JObjectFileIOHelper.h"
#include"../../../Core/Guid/JGuidCreator.h"
#include"../../../Core/Reflection/JTypeImplBase.h" 
#include"../../../Core/Platform/JHardwareInfo.h" 
#include"../../../Core/Utility/JCommonUtility.h"
#include"../../../Graphic/JGraphic.h"
#include"../../../Graphic/JGraphicPrivate.h"
#include"../../../Graphic/Shader/JShaderDataHolder.h"
#include"../../../Graphic/Shader/JShaderType.h"
#include"../../../Application/Engine/JApplicationEngine.h"
#include"../../../Application/Project/JApplicationProject.h"
#include<fstream> 

namespace JinEngine
{ 
	namespace
	{ 
		static JShaderPrivate sPrivate;
	}
 
	namespace
	{
		static JUserPtr<JDirectory> GetShaderDirectory()
		{
			return _JResourceManager::Instance().GetDirectory(JApplicationProject::ShaderMetafilePath());
		}
		/*
		static JShader* FindOverlapShader(const J_GRAPHIC_SHADER_FUNCTION newFunc, const JGraphicShaderCondition& condition)
		{
			uint count;
			std::vector<JResourceObject*>::const_iterator st = _JResourceManager::Instance().Instance().GetResourceVectorHandle<JShader>(count);
			for (uint i = 0; i < count; ++i)
			{
				JShader* shader = static_cast<JShader*>(*(st + i));
				if ((*equalGShader)(nullptr, newFunc, condition, shader))
					return shader;
			}
			return nullptr;
		}
		static JShader* FindOverlapShader(const J_COMPUTE_SHADER_FUNCTION newFunc)
		{
			uint count;
			std::vector<JResourceObject*>::const_iterator st = _JResourceManager::Instance().Instance().GetResourceVectorHandle<JShader>(count);
			for (uint i = 0; i < count; ++i)
			{
				JShader* shader = static_cast<JShader*>(*(st + i));
				if ((*equalCShader)(nullptr, newFunc, shader))
					return shader;
			}
			return nullptr;
		}
		*/
		static JUserPtr<JShader> FindOverlapShader(const std::wstring& name)
		{
			JUserPtr<JFile> file = GetShaderDirectory()->GetDirectoryFileByName(name);
			return file != nullptr ? Core::ConvertChildUserPtr<JShader>(file->TryGetResourceUser()) : nullptr;
		}
		static std::wstring MakeName(const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
			const JGraphicShaderCondition& condition,
			const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)noexcept
		{
			if (cFunctionFlag == J_COMPUTE_SHADER_FUNCTION::NONE)
				return JShaderType::ConvertToName(gFunctionFlag, condition.UniqueID());
			else
				return JShaderType::ConvertToName(cFunctionFlag);
		}
	}
 
	class JShader::JShaderImpl : public Core::JTypeImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JShaderImpl)
	public:
		JWeakPtr<JShader> thisPointer = nullptr;
	public: 
		//graphic forward
		JOwnerPtr<Graphic::JShaderDataHolder> gFShaderData[(uint)J_GRAPHIC_SHADER_TYPE::COUNT][(uint)J_GRAPHIC_SHADER_VERTEX_LAYOUT::COUNT];
		//graphic deferred geometry
		JOwnerPtr<Graphic::JShaderDataHolder> gDGShaderData[(uint)J_GRAPHIC_SHADER_TYPE::COUNT][(uint)J_GRAPHIC_SHADER_VERTEX_LAYOUT::COUNT];
	public:
		//compute
		JOwnerPtr<Graphic::JShaderDataHolder> cShaderData = nullptr;
	public:
		J_GRAPHIC_SHADER_FUNCTION gFunctionFlag = SHADER_FUNCTION_NONE;
		J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE;
	public:
		JGraphicShaderCondition condition;
	public:
		JShaderImpl(const InitData& initData, JShader* thisShaderRaw)
			:gFunctionFlag(initData.gFunctionFlag),
			cFunctionFlag(initData.cFunctionFlag),
			condition(initData.condition)
		{}
		~JShaderImpl()
		{}
	public:
		bool IsComputeShader()const noexcept
		{
			return cFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE;
		}
	public:
		void SetGraphicShaderFunctionFlag(const J_GRAPHIC_SHADER_FUNCTION newFunctionFlag)
		{
			if (gFunctionFlag != newFunctionFlag || !HasGraphicShaderData())
			{
				gFunctionFlag = newFunctionFlag;
				if (thisPointer->IsActivated())
					CompileShdaer(); 
			}
		}
		void SetComputeShaderFunctionFlag(const J_COMPUTE_SHADER_FUNCTION newFunctionFlag)
		{
			if (cFunctionFlag != newFunctionFlag || !HasComputeShaderData())
			{
				cFunctionFlag = newFunctionFlag;
				if (thisPointer->IsActivated())
					CompileShdaer();
			}
		}
	public:
		bool HasGraphicShaderData()const noexcept
		{
			bool hasData = true;
			for (uint i = 0; i < (uint)J_GRAPHIC_SHADER_VERTEX_LAYOUT::COUNT; ++i)
				hasData = hasData && (gFShaderData[i] != nullptr);
			return hasData;
		}
		bool HasComputeShaderData()const noexcept
		{ 
			return cShaderData != nullptr;
		}
	public:
		void RecompileGraphicShader()
		{
			if (thisPointer->IsActivated())
			{
				for (uint i = 0; i < (uint)J_GRAPHIC_SHADER_TYPE::COUNT; ++i)
				{
					for (uint j = 0; j < (uint)J_GRAPHIC_SHADER_VERTEX_LAYOUT::COUNT; ++j)
					{
						gFShaderData[i][j] = nullptr;
						gDGShaderData[i][j] = nullptr;
					}
				}
				CompileShdaer();
			}
		}
		void RecompileComputeShader()
		{
			if (thisPointer->IsActivated())
			{
				cShaderData.Clear();
				CompileShdaer();
			}
		}
		void CompileShdaer()
		{
			if (thisPointer == nullptr)
				return;

			if (cFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
				CompileComputeShader();
			else
				CompileGraphicShader();
		}
		void CompileGraphicShader()
		{
			JGraphicShaderInitData initHelper;
			StuffInitHelper(initHelper, gFunctionFlag, condition);

			using GResourceInterface = Graphic::JGraphicPrivate::ResourceInterface;
			for (uint i = 0; i < (uint)J_GRAPHIC_SHADER_TYPE::COUNT; ++i)
			{
				for (uint j = 0; j < (uint)J_GRAPHIC_SHADER_VERTEX_LAYOUT::COUNT; ++j)
				{
					initHelper.shaderType = (J_GRAPHIC_SHADER_TYPE)i;
					initHelper.layoutType = (J_GRAPHIC_SHADER_VERTEX_LAYOUT)j;
					initHelper.processType = J_GRAPHIC_RENDERING_PROCESS::FORWARD;
					gFShaderData[i][j] = GResourceInterface::StuffGraphicShaderPso(initHelper);
				}
			}
			if (!JGraphic::Instance().GetGraphicOption().rendering.allowDeferred)
				return;
			    
			for (uint i = 0; i < (uint)J_GRAPHIC_SHADER_TYPE::COUNT; ++i)
			{
				for (uint j = 0; j < (uint)J_GRAPHIC_SHADER_VERTEX_LAYOUT::COUNT; ++j)
				{
					initHelper.shaderType = (J_GRAPHIC_SHADER_TYPE)i;
					initHelper.layoutType = (J_GRAPHIC_SHADER_VERTEX_LAYOUT)j;
					initHelper.processType = J_GRAPHIC_RENDERING_PROCESS::DEFERRED_GEOMETRY;
					gDGShaderData[i][j] = GResourceInterface::StuffGraphicShaderPso(initHelper);
				}
			}
		}
		void CompileComputeShader()
		{
			JComputeShaderInitData initHelper;
			StuffInitHelper(initHelper, cFunctionFlag);
			//macroVec = std::vector<D3D_SHADER_MACRO>{ { NULL, NULL } };
			if (cFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
			{
				using GResourceInterface = Graphic::JGraphicPrivate::ResourceInterface;
				cShaderData = GResourceInterface::StuffComputeShaderPso(initHelper);
			}
		}
		static void StuffInitHelper(_Out_ JGraphicShaderInitData& initHelper,
			const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag, 
			const JGraphicShaderCondition& cond)noexcept
		{
			//앞으로 graphic shader에 macro는 JGraphicShaderDataHandler에 하위클래스에서 stuff 하도록한다. --2023/12/28--
			initHelper.gFunctionFlag = gFunctionFlag;
			initHelper.condition = cond;
		} 
		static void StuffInitHelper(_Out_ JComputeShaderInitData& initHelper, J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
		{
			//앞으로 hzb에 대한 shader option control은 JShader객체를 통해서가아닌
			//Graphic Option을 통해서 이루어지게 한다.
			//그러므로 JHZBOccCulling 하위 class들이 Shader data를 소유하고 graphic option변경에 따라 graphic이 이들을 호출해
			//shader를 새 option에 따라 재컴파일한다.
			/*
			auto calThreadDim = [](const uint ori, const uint length, const uint devideFactor, uint& devideCount) -> uint
			{
				devideCount = 0;
				uint result = ori;
				while (result > length)
				{
					result /= devideFactor;
					++devideCount;
				}
				if (result == 0)
					return 1;
				else
					return result;
			};
			using GpuInfo = Core::JHardwareInfo::GpuInfo;		
			auto InitHZBMaps = [](_Out_ JComputeShaderInitData& initHelper, const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
			{
				std::vector<GpuInfo> gpuInfo = Core::JHardwareInfo::GetGpuInfo();
				JGraphicInfo graphicInfo = JGraphic::Instance().GetGraphicInfo();

				//수정필요
				//thread per group factor가 하드코딩됨
				//이후 amd graphic info 추가와 동시에 수정할 예정
				uint warpFactor = gpuInfo[0].vendor == Core::J_GRAPHIC_VENDOR::AMD ? 64 : 32;
				uint groupDimX = (uint)std::ceil((float)graphicInfo.occlusionWidth / float(gpuInfo[0].maxThreadsDim.x));
				uint groupDimY = graphicInfo.occlusionHeight;

				//textuer size is always 2 squared
				uint threadDimX = graphicInfo.occlusionWidth;
				uint threadDimY = (uint)std::ceil((float)graphicInfo.occlusionHeight / float(gpuInfo[0].maxGridDim.y));

				initHelper.dispatchInfo.threadDim = JVector3<uint>(threadDimX, threadDimY, 1);
				initHelper.dispatchInfo.groupDim = JVector3<uint>(groupDimX, groupDimY, 1);
				initHelper.dispatchInfo.taskOriCount = graphicInfo.occlusionWidth * graphicInfo.occlusionHeight;
				StuffComputeShaderCommonMacro(initHelper, cFunctionFlag);
			};

			*/

			switch (cFunctionFlag)
			{
			default:
				break;
			}
			//initHelper.macro.push_back({ NULL, NULL });
			initHelper.cFunctionFlag = cFunctionFlag;
		}
	public:
		void ClearShaderData()
		{
			for (uint i = 0; i < (uint)J_GRAPHIC_SHADER_TYPE::COUNT; ++i)
			{
				for (uint j = 0; j < (uint)J_GRAPHIC_SHADER_VERTEX_LAYOUT::COUNT; ++j)
				{
					if (gFShaderData[i][j] != nullptr)
					{
						gFShaderData[i][j]->Clear();
						gFShaderData[i][j].Clear();
					}
					if (gDGShaderData[i][j] != nullptr)
					{
						gDGShaderData[i][j]->Clear();
						gDGShaderData[i][j].Clear();
					}
				}
			}
			 
			if (cShaderData != nullptr)
			{
				cShaderData->Clear();
				cShaderData.Clear();
			}
		}
	public:
		bool ReadAssetData()
		{
			return true;
		}
		bool WriteAssetData()
		{
			return true;
		}
	public: 
		void Initialize()
		{
			if (IsComputeShader())
				SetComputeShaderFunctionFlag((J_COMPUTE_SHADER_FUNCTION)cFunctionFlag);
			else
				SetGraphicShaderFunctionFlag((J_GRAPHIC_SHADER_FUNCTION)gFunctionFlag);
		}
		void RegisterThisPointer(JShader* shader)
		{
			thisPointer = Core::GetWeakPtr(shader);
		}
		static void RegisterTypeData()
		{
			auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex(GetStaticResourceType(),format); };

			static GetRTypeInfoCallable getTypeInfoCallable{ &JShader::StaticTypeInfo };
			static GetAvailableFormatCallable getAvailableFormatCallable{ &JShader::GetAvailableFormat };
			static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

			static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, true, false, true, true, false, false };
			static RTypeCommonFunc rTypeCFunc{ getTypeInfoCallable, getAvailableFormatCallable, getFormatIndexCallable };

			RegisterRTypeInfo(rTypeHint, rTypeCFunc, RTypePrivateFunc{});
			Core::JIdentifier::RegisterPrivateInterface(JShader::StaticTypeInfo(), sPrivate);

			IMPL_REALLOC_BIND(JShader::JShaderImpl, thisPointer)
		}
	};

	JShader::InitData::InitData(const J_OBJECT_FLAG flag, 
		const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
		const JGraphicShaderCondition condition,
		const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
		:JResourceObject::InitData(JShader::StaticTypeInfo(), GetDefaultFormatIndex(), GetStaticResourceType(), GetShaderDirectory()),
		gFunctionFlag(gFunctionFlag), 
		condition(condition), 
		cFunctionFlag(cFunctionFlag)
	{
		InitData::flag = Core::HasSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE) ? flag : Core::AddSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE);
		if (cFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
			InitData::gFunctionFlag = SHADER_FUNCTION_NONE;
		name = MakeName(InitData::gFunctionFlag, condition, cFunctionFlag);
		name = directory->MakeUniqueFileName(name, RTypeCommonCall::GetFormat(rType, formatIndex), guid);
	}
	JShader::InitData::InitData(const std::wstring& name,
		const size_t& guid,
		const J_OBJECT_FLAG flag,
		const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
		const JGraphicShaderCondition condition,
		const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
		: JResourceObject::InitData(JShader::StaticTypeInfo(), name, guid, flag, GetDefaultFormatIndex(), GetStaticResourceType(), GetShaderDirectory()),
		gFunctionFlag(gFunctionFlag),
		condition(condition),
		cFunctionFlag(cFunctionFlag)
	{
		InitData::flag = Core::HasSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE) ? flag : Core::AddSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE);
		if (cFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
			InitData::gFunctionFlag = SHADER_FUNCTION_NONE;
		InitData::name = MakeName(InitData::gFunctionFlag, condition, cFunctionFlag);
		InitData::name = directory->MakeUniqueFileName(name, RTypeCommonCall::GetFormat(rType, formatIndex), guid);
	}

	Core::JIdentifierPrivate& JShader::PrivateInterface()const noexcept
	{
		return sPrivate;
	}
	J_RESOURCE_TYPE JShader::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	}
	std::wstring JShader::GetFormat()const noexcept
	{
		return GetAvailableFormat()[GetFormatIndex()];
	}
	std::vector<std::wstring> JShader::GetAvailableFormat()noexcept
	{
		static std::vector<std::wstring> format{ L".shader" };
		return format;
	}
	JUserPtr<Graphic::JShaderDataHolder> JShader::GetGraphicData(const J_GRAPHIC_RENDERING_PROCESS processType, const J_GRAPHIC_SHADER_TYPE type, const J_GRAPHIC_SHADER_VERTEX_LAYOUT vertexLayout)const noexcept
	{
		if (processType == J_GRAPHIC_RENDERING_PROCESS::DEFERRED_GEOMETRY)
			return impl->gDGShaderData[(uint)type][(uint)vertexLayout];
		else
			return impl->gFShaderData[(uint)type][(uint)vertexLayout];
	}
	JUserPtr<Graphic::JShaderDataHolder> JShader::GetGraphicForwardData(const J_GRAPHIC_SHADER_TYPE type, const J_GRAPHIC_SHADER_VERTEX_LAYOUT vertexLayout)const noexcept
	{
		return impl->gFShaderData[(uint)type][(uint)vertexLayout];
	}
	JUserPtr<Graphic::JShaderDataHolder> JShader::GetGraphicDeferredData(const J_GRAPHIC_SHADER_TYPE type, const J_GRAPHIC_SHADER_VERTEX_LAYOUT vertexLayout)const noexcept
	{
		return impl->gDGShaderData[(uint)type][(uint)vertexLayout];
	} 
	JUserPtr<Graphic::JShaderDataHolder> JShader::GetComputeData()const noexcept
	{
		return impl->cShaderData;
	} 
	J_GRAPHIC_SHADER_FUNCTION JShader::GetShaderGFunctionFlag()const noexcept
	{
		return impl->gFunctionFlag;
	}
	J_COMPUTE_SHADER_FUNCTION JShader::GetShdaerCFunctionFlag()const noexcept
	{
		return impl->cFunctionFlag;
	}
	JGraphicShaderCondition JShader::GetShaderCondition()const noexcept
	{
		return impl->condition;
	} 
	bool JShader::IsComputeShader()const noexcept
	{
		return impl->IsComputeShader();
	}
	JUserPtr<JShader> JShader::FindShader(const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
		const JGraphicShaderCondition condition,
		const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
	{
		return FindOverlapShader(MakeName(gFunctionFlag, condition, cFunctionFlag));
	}
	void JShader::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
		impl->CompileShdaer();
		SetValid(true);
	}
	void JShader::DoDeActivate()noexcept
	{
		impl->ClearShaderData();
		SetValid(false);
		JResourceObject::DoDeActivate();
	}
	JShader::JShader(const InitData& initData)
		: JResourceObject(initData), impl(std::make_unique<JShaderImpl>(initData, this))
	{}
	JShader::~JShader()
	{
		impl.reset();
	}
	 
	using CreateInstanceInterface = JShaderPrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JShaderPrivate::AssetDataIOInterface;
	using CompileInterface = JShaderPrivate::CompileInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JShader>(*static_cast<JShader::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JResourceObjectPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JShader* shader = static_cast<JShader*>(createdPtr);
		shader->impl->RegisterThisPointer(shader);
		shader->impl->Initialize();
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JShader::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JShader::LoadData::StaticTypeInfo()))
			return nullptr;
 
		auto loadData = static_cast<JShader::LoadData*>(data);
		auto pathData = loadData->pathData;
		JUserPtr<JDirectory> directory = loadData->directory;

		auto initData = std::make_unique<JShader::InitData>();	//for load metadata
		if (LoadMetaData(pathData.metaFilePath, initData.get()) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;

		JUserPtr<JShader> newShdaer = nullptr;
		if (directory->HasFile(initData->guid))
			newShdaer = Core::GetUserPtr<JShader>(JShader::StaticTypeInfo().TypeGuid(), initData->guid);

		if (newShdaer == nullptr)
		{
			initData->name = MakeName(initData->gFunctionFlag, initData->condition, initData->cFunctionFlag);
			auto idenUser = sPrivate.GetCreateInstanceInterface().BeginCreate(std::move(initData), &sPrivate);
			newShdaer.ConnnectChild(idenUser);
		}
		newShdaer->impl->ReadAssetData();
		return newShdaer;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JShader::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JShader::StoreData*>(data);
		if (!storeData->HasCorrectType(JShader::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JUserPtr<JShader> shader;
		shader.ConnnectChild(storeData->obj);
		return shader->impl->WriteAssetData() ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JShader::InitData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JFileIOTool tool;
		if (!tool.Begin(path, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto loadMetaData = static_cast<JShader::InitData*>(data);
		if (LoadCommonMetaData(tool, loadMetaData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
 
		JObjectFileIOHelper::LoadEnumData(tool, loadMetaData->gFunctionFlag, "ShaderFuncFlag:");
		JObjectFileIOHelper::LoadEnumData(tool, loadMetaData->cFunctionFlag, "ComputeShaderFuncFlag:");

		JObjectFileIOHelper::LoadEnumData(tool, loadMetaData->condition.primitiveCondition, "SubPsoPrimitiveCondition:");
		JObjectFileIOHelper::LoadEnumData(tool, loadMetaData->condition.depthCompareCondition, "SubPsoDepthComparesionCondition:");
		JObjectFileIOHelper::LoadEnumData(tool, loadMetaData->condition.cullModeCondition, "SubPsoCullModeCondition:");
		JObjectFileIOHelper::LoadEnumData(tool, loadMetaData->condition.primitiveType, "SubPsoPrimitive:");
		JObjectFileIOHelper::LoadEnumData(tool, loadMetaData->condition.depthCompareFunc, "SubPsoDepthComparesion:");
		JObjectFileIOHelper::LoadAtomicData(tool, loadMetaData->condition.isCullModeNone, "SubPsoCullMode:");
		tool.Close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JShader::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JShader::StoreData*>(data);
		JUserPtr<JShader> shader;
		shader.ConnnectChild(storeData->obj);

		JFileIOTool tool;
		if (!tool.Begin(shader->GetMetaFilePath(), JFileIOTool::TYPE::JSON))
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (StoreCommonMetaData(tool, storeData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JObjectFileIOHelper::StoreEnumData(tool, shader->impl->gFunctionFlag, "ShaderFuncFlag:");
		JObjectFileIOHelper::StoreEnumData(tool, shader->impl->cFunctionFlag, "ComputeShaderFuncFlag:");

		JObjectFileIOHelper::StoreEnumData(tool, shader->impl->condition.primitiveCondition, "SubPsoPrimitiveCondition:");
		JObjectFileIOHelper::StoreEnumData(tool, shader->impl->condition.depthCompareCondition, "SubPsoDepthComparesionCondition:");
		JObjectFileIOHelper::StoreEnumData(tool, shader->impl->condition.cullModeCondition, "SubPsoCullModeCondition:");
		JObjectFileIOHelper::StoreEnumData(tool, shader->impl->condition.primitiveType, "SubPsoPrimitive:");
		JObjectFileIOHelper::StoreEnumData(tool, shader->impl->condition.depthCompareFunc, "SubPsoDepthComparesion:");
		JObjectFileIOHelper::StoreAtomicData(tool, shader->impl->condition.isCullModeNone, "SubPsoCullMode:");
		
		tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	void CompileInterface::RecompileGraphicShader(JShader* shader)noexcept
	{
		shader->impl->RecompileGraphicShader();
	}
	void CompileInterface::RecompileComputeShader(JShader* shader)noexcept
	{
		shader->impl->RecompileComputeShader();
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JShaderPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	JResourceObjectPrivate::AssetDataIOInterface& JShaderPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}
}