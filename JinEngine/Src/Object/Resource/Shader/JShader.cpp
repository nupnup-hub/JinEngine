#include"JShader.h" 
#include"../JResourceManager.h"
#include"../JResourceObjectFactory.h"
#include"../../Directory/JDirectory.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Platform/JHardwareInfo.h"
#include"../../../Graphic/JGraphic.h"
#include"../../../Application/JApplicationVariable.h"
#include"../../../Utility/JD3DUtility.h"
#include"../../../Utility/JCommonUtility.h"
#include<fstream>

namespace JinEngine
{
	std::unordered_map<J_GRAPHIC_SHADER_FUNCTION, const D3D_SHADER_MACRO> JShader::shaderFuncMacroMap
	{
		{SHADER_FUNCTION_NONE, {NULL, NULL}},
		{SHADER_FUNCTION_ALBEDO_MAP, {"ALBEDO_MAP", "1"}},
		{SHADER_FUNCTION_ALBEDO_MAP_ONLY, {"ALBEDO_MAP_ONLY", "2"}},
		{SHADER_FUNCTION_NORMAL_MAP, {"NORMAL_MAP", "3"}},
		{SHADER_FUNCTION_HEIGHT_MAP, {"HEIGHT_MAP", "4"}},
		{SHADER_FUNCTION_ROUGHNESS_MAP,{ "ROUGHNESS_MAP", "5"}},
		{SHADER_FUNCTION_AMBIENT_OCCLUSION_MAP, {"AMBIENT_OCCUSION_MAP", "6"}},
		{SHADER_FUNCTION_SHADOW, {"SHADOW", "7"}},
		{SHADER_FUNCTION_LIGHT, {"LIGHT", "8"}},
		{SHADER_FUNCTION_SKY, {"SKY", "9"}},
		{SHADER_FUNCTION_NONCULLING, {"NONCULLING", "10"}},
		{SHADER_FUNCTION_ALPHA_CLIP, {"ALPHA_CLIP", "11"}},
		{SHADER_FUNCTION_WRITE_SHADOWMAP, {"WRITE_SHADOW_MAP", "12"}},
		{SHADER_FUNCTION_DEPTH_TEST_BOUNDING_OBJECT, {"BOUNDING_OBJECT_DEPTH_TEST", "13"}},
		{SHADER_FUNCTION_DEBUG, {"DEBUG", "14"}},
	};

	//std::unordered_map<J_COMPUTE_SHADER_FUNCTION, std::unique_ptr<JShader::CSInitHelperCallable>> JShader::computeShaderHelperMap;
	std::unordered_map<J_SHADER_VERTEX_LAYOUT, const D3D_SHADER_MACRO> JShader::vertexLayoutMacroMap
	{
		{SHADER_VERTEX_LAYOUT_STATIC, {"STATIC", "1"}},
		{SHADER_VERTEX_LAYOUT_SKINNED, {"SKINNED", "2"}},
	};
	std::unordered_map<J_SHADER_VERTEX_LAYOUT, std::vector<D3D12_INPUT_ELEMENT_DESC>> JShader::inputLayout
	{
		{SHADER_VERTEX_LAYOUT_STATIC,
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		}},
		{SHADER_VERTEX_LAYOUT_SKINNED,
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		}},
	};

	namespace
	{
		//Graphic Shader Macro Symbol
		static const std::string gShdaer2DTextureSymbol = "TEXTURE_2D_COUNT";
		static const std::string gShdaerCubeMapSymbol = "CUBE_MAP_COUNT";
		static const std::string gShdaerShadowMapSymbol = "SHADOW_MAP_COUNT";

		static std::string gTexture2DCount;
		static std::string gTextureCubeCount;
		static std::string gTextureShadowCount;
	}

	namespace
	{
		//Compute Shader Macro Symbol
		static const std::string threadDimXSymbol = "DIMX";
		static const std::string threadDimYSymbol = "DIMY";
		static const std::string threadDimZSymbol = "DIMZ";

		static const std::string hzbSamplingCountSymbol = "DOWN_SAMPLING_COUNT";
		static const std::string hzbOcclusionQueryCountSymbol = "OCCLUSION_QUERY_COUNT";

		static std::string hzbSamplingCount;
		static std::string hzbOcclusionQueryCount;

		static std::unordered_map <J_COMPUTE_SHADER_FUNCTION, std::vector<std::string>> computeShaderThreadDim;
	}

	static JShader* FindOverlapShader(const J_GRAPHIC_SHADER_FUNCTION newFunc)
	{
		uint count;
		std::vector<JResourceObject*>::const_iterator st = JResourceManager::Instance().GetResourceVectorHandle<JShader>(count);
		for (uint i = 0; i < count; ++i)
		{
			JShader* shader = static_cast<JShader*>(*(st + i));
			if (newFunc == shader->GetShaderFunctionFlag())
				return shader;
		}
		return nullptr;
	}
	static JShader* FindOverlapShader(const J_COMPUTE_SHADER_FUNCTION newFunc)
	{
		uint count;
		std::vector<JResourceObject*>::const_iterator st = JResourceManager::Instance().GetResourceVectorHandle<JShader>(count);
		for (uint i = 0; i < count; ++i)
		{
			JShader* shader = static_cast<JShader*>(*(st + i));
			if (newFunc == shader->GetShdaerComputeFunctionFlag())
				return shader;
		}
		return nullptr;
	}
	static JDirectory* GetShaderDirectory()
	{
		return JResourceManager::Instance().GetDirectory(JApplicationVariable::GetProjectShaderMetafilePath());
	}

	JShader::JShaderInitdata::JShaderInitdata(const size_t guid,
		const J_OBJECT_FLAG flag,
		const J_GRAPHIC_SHADER_FUNCTION newGShaderFunctionFlag,
		const J_COMPUTE_SHADER_FUNCTION newCShaderFunctionFlag)
		:JResourceInitData(MakeName(newGShaderFunctionFlag, newCShaderFunctionFlag),
			guid,
			Core::HasSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE) ? flag : Core::AddSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE),
			GetShaderDirectory(),
			JResourceObject::GetFormatIndex<JShader>(GetAvailableFormat()[0])),
		gShaderFunctionFlag(newGShaderFunctionFlag),
		cShaderFunctionFlag(newCShaderFunctionFlag)
	{
		if (cShaderFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
			gShaderFunctionFlag = SHADER_FUNCTION_NONE;
	}
	JShader::JShaderInitdata::JShaderInitdata(const J_OBJECT_FLAG flag,
		const J_GRAPHIC_SHADER_FUNCTION newGShaderFunctionFlag,
		const J_COMPUTE_SHADER_FUNCTION newCShaderFunctionFlag)
		: JResourceInitData(MakeName(newGShaderFunctionFlag, newCShaderFunctionFlag),
			Core::MakeGuid(),
			Core::HasSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE) ? flag : Core::AddSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE),
			GetShaderDirectory(),
			JResourceObject::GetFormatIndex<JShader>(GetAvailableFormat()[0])),
		gShaderFunctionFlag(newGShaderFunctionFlag),
		cShaderFunctionFlag(newCShaderFunctionFlag)
	{
		if (cShaderFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
			gShaderFunctionFlag = SHADER_FUNCTION_NONE;
	}
	JShader::JShaderInitdata::JShaderInitdata(const J_GRAPHIC_SHADER_FUNCTION newGShaderFunctionFlag, const J_COMPUTE_SHADER_FUNCTION newCShaderFunctionFlag)
		: JResourceInitData(MakeName(newGShaderFunctionFlag, newCShaderFunctionFlag),
			Core::MakeGuid(),
			OBJECT_FLAG_UNEDITABLE,
			GetShaderDirectory(),
			JResourceObject::GetFormatIndex<JShader>(GetAvailableFormat()[0])),
		gShaderFunctionFlag(newGShaderFunctionFlag),
		cShaderFunctionFlag(newCShaderFunctionFlag)
	{
		if (cShaderFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
			gShaderFunctionFlag = SHADER_FUNCTION_NONE;
	}
	J_RESOURCE_TYPE JShader::JShaderInitdata::GetResourceType() const noexcept
	{
		return J_RESOURCE_TYPE::SHADER;
	}
	std::wstring JShader::JShaderInitdata::MakeName(const J_GRAPHIC_SHADER_FUNCTION gShaderFunctionFlag, const J_COMPUTE_SHADER_FUNCTION cShaderFunctionFlag)const noexcept
	{
		if (cShaderFunctionFlag == J_COMPUTE_SHADER_FUNCTION::NONE)
			return JShaderType::ConvertToName(gShaderFunctionFlag);
		else
			return JShaderType::ConvertToName(cShaderFunctionFlag);
	}

	J_RESOURCE_TYPE JShader::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	}
	std::wstring JShader::GetFormat()const noexcept
	{
		return GetAvailableFormat()[0];
	}
	std::vector<std::wstring> JShader::GetAvailableFormat()noexcept
	{
		static std::vector<std::wstring> format{ L".shader" };
		return format;
	}
	ID3D12PipelineState* JShader::GetComputePso()const noexcept
	{
		return cShaderData->Pso.Get();
	}
	ID3D12PipelineState* JShader::GetGraphicPso(const J_SHADER_VERTEX_LAYOUT vertexLayout)const noexcept
	{
		return gShaderData[(int)vertexLayout]->Pso.Get();
	}
	ID3D12RootSignature* JShader::GetComputeRootSignature()const noexcept
	{
		return cShaderData->RootSignature;
	}
	J_GRAPHIC_SHADER_FUNCTION JShader::GetShaderFunctionFlag()const noexcept
	{
		return gFunctionFlag;
	}
	J_COMPUTE_SHADER_FUNCTION JShader::GetShdaerComputeFunctionFlag()const noexcept
	{
		return cFunctionFlag;
	}
	bool JShader::IsComputeShader()const noexcept
	{
		return cFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE;
	}
	JShaderCompileInterface* JShader::CompileInterface()noexcept
	{
		return this;
	}
	void JShader::DoCopy(JObject* ori)
	{
		JShader* oriS = static_cast<JShader*>(ori);
		CopyRFile(*oriS);
		gFunctionFlag = oriS->gFunctionFlag;
		CompileShdaer(this);
	}
	void JShader::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
		CompileShdaer(this);
		SetValid(true);
	}
	void JShader::DoDeActivate()noexcept
	{
		JResourceObject::DoDeActivate();
		gShaderData[0].reset();
		gShaderData[1].reset();
		cShaderData.reset();
		SetValid(false);
	}
	void JShader::SetGraphicShaderFunctionFlag(const J_GRAPHIC_SHADER_FUNCTION newFunctionFlag)
	{
		if (gFunctionFlag != newFunctionFlag || !HasShaderData())
		{
			gFunctionFlag = newFunctionFlag;
			if (IsActivated())
				CompileShdaer(this);
		}
	}
	void JShader::RecompileGraphicShader()
	{
		if (IsActivated())
		{
			for (uint i = 0; i < SHADER_VERTEX_COUNT; ++i)
				gShaderData[i].reset();
			CompileShdaer(this);
		}
	}
	void JShader::CompileShdaer(JShader* shader)
	{
		if (shader == nullptr)
			return;

		if (shader->cFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
			CompileComputeShader(shader);
		else
			CompileGraphicShader(shader);
	}
	void JShader::CompileGraphicShader(JShader* shader)
	{
		std::vector<D3D_SHADER_MACRO> macroVec[SHADER_VERTEX_COUNT];
		for (uint j = 0; j < SHADER_VERTEX_COUNT; ++j)
			GetMacroVec(macroVec[j], (J_SHADER_VERTEX_LAYOUT)(SHADER_VERTEX_LAYOUT_STATIC + j), shader->GetShaderFunctionFlag());

		std::wstring vertexShaderPath = JApplicationVariable::GetShaderPath() + L"\\VertexShader.hlsl";
		std::wstring pixelShaderPath = JApplicationVariable::GetShaderPath() + L"\\PixelShader.hlsl";

		for (uint j = 0; j < SHADER_VERTEX_COUNT; ++j)
		{
			shader->gShaderData[j] = std::make_unique<JGraphicShaderData>();
			shader->gShaderData[j]->Vs = JD3DUtility::CompileShader(vertexShaderPath, &macroVec[j][0], "VS", "vs_5_1");
			shader->gShaderData[j]->Ps = JD3DUtility::CompileShader(pixelShaderPath, &macroVec[j][0], "PS", "ps_5_1");
			GetInputLayout(shader->gShaderData[j]->InputLayout, (J_SHADER_VERTEX_LAYOUT)(SHADER_VERTEX_LAYOUT_STATIC + j));
			JGraphic::Instance().ResourceInterface()->StuffGraphicShaderPso(shader->gShaderData[j].get(), (J_SHADER_VERTEX_LAYOUT)(SHADER_VERTEX_LAYOUT_STATIC + j), shader->GetShaderFunctionFlag());
		}
	}
	void JShader::CompileComputeShader(JShader* shader)
	{
		CSInitHelper initHelper; 
		GetInitHelper(initHelper, shader->GetShdaerComputeFunctionFlag());

		//macroVec = std::vector<D3D_SHADER_MACRO>{ { NULL, NULL } };
		if (shader->cFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
		{
			JShaderType::CompileInfo compileInfo = JShaderType::ComputeShaderCompileInfo(shader->cFunctionFlag);
			std::wstring computeShaderPath = JApplicationVariable::GetShaderPath() + L"\\" + compileInfo.fileName;
			shader->cShaderData = std::make_unique< JComputeShaderData>();
			shader->cShaderData->Cs = JD3DUtility::CompileShader(computeShaderPath, &initHelper.macro[0], compileInfo.functionName, "cs_5_1");
			shader->cShaderData->dispatchInfo = initHelper.dispatchInfo;
			JGraphic::Instance().ResourceInterface()->StuffComputeShaderPso(shader->cShaderData.get(), shader->cFunctionFlag);
		}
	}
	void JShader::GetMacroVec(_Out_ std::vector<D3D_SHADER_MACRO>& outMacro, const J_SHADER_VERTEX_LAYOUT vertexLayoutFlag, const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag)noexcept
	{
		outMacro.push_back(vertexLayoutMacroMap.find(vertexLayoutFlag)->second);
		for (const auto& data : shaderFuncMacroMap)
		{
			if ((data.first & gFunctionFlag) > 0)
				outMacro.push_back(data.second);
		}

		Graphic::JGraphicInfo info = JGraphic::Instance().GetGraphicInfo();
		gTexture2DCount = std::to_string(info.binding2DTextureCapacity);
		gTextureCubeCount = std::to_string(info.bindingCubeMapCapacity);
		gTextureShadowCount = std::to_string(info.initBindTShadowCapacity);

		outMacro.push_back({ gShdaer2DTextureSymbol.c_str(), gTexture2DCount.c_str() });
		outMacro.push_back({ gShdaerCubeMapSymbol.c_str(), gTextureCubeCount.c_str() });
		outMacro.push_back({ gShdaerShadowMapSymbol.c_str(), gTextureShadowCount.c_str() });

		outMacro.push_back(shaderFuncMacroMap.find(SHADER_FUNCTION_NONE)->second);
	}
	void JShader::GetInitHelper(_Out_ CSInitHelper& initHelper, const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)noexcept
	{
		auto calThreadDim = [](const uint ori, const uint range, const uint devideFactor, uint& devideCount) -> uint
		{
			devideCount = 0;
			uint result = ori;
			while (result > range)
			{
				result /= devideFactor;
				++devideCount;
			}
			if (result == 0)
				return 1;
			else
				return result;
		};

		using GpuInfo = Core::JHardwareInfoImpl::GpuInfo;

		switch (cFunctionFlag)
		{
		case JinEngine::J_COMPUTE_SHADER_FUNCTION::HZB_DOWN_SAMPLING:
		{
			std::vector<GpuInfo> gpuInfo = Core::JHardwareInfo::Instance().GetGpuInfo();
			uint totalSmCount = 0;
			uint totalBlockPerSmCount = 0;
			uint totalThreadPerBlockCount = 0;
			for (const auto& data : gpuInfo)
			{
				totalSmCount += data.multiProcessorCount;
				totalBlockPerSmCount += data.maxBlocksPerMultiProcessor;
				totalThreadPerBlockCount += data.maxThreadsPerBlock;
			}

			Graphic::JGraphicInfo graphicInfo = JGraphic::Instance().GetGraphicInfo();
			uint depthMapPixelCount = graphicInfo.occlusionWidth * graphicInfo.occlusionHeight;

			//수정필요 
			//thread per group factor가 하드코딩됨
			//이후 amd graphic info 추가와 동시에 수정할 예정
			uint warpFactor = gpuInfo[0].vendor == Core::J_GRAPHIC_VENDOR::AMD ? 64 : 32;
			CalculateThreadDim(initHelper.dispatchInfo, depthMapPixelCount, totalSmCount, warpFactor, 1024); 
		
			StuffComputeShaderThreadDim(cFunctionFlag, initHelper.dispatchInfo.threadDim);
			hzbSamplingCount = std::to_string(graphicInfo.occlusionMapCapacity);
			initHelper.macro.push_back({ hzbSamplingCountSymbol.c_str(), hzbSamplingCount.c_str() });
			OutComputeShaderCommonMacro(initHelper.macro, cFunctionFlag);
			break;
		}
		case JinEngine::J_COMPUTE_SHADER_FUNCTION::HZB_OCCLUSION:
		{
			std::vector<GpuInfo> gpuInfo = Core::JHardwareInfo::Instance().GetGpuInfo();
			uint totalSmCount = 0;
			uint totalBlockPerSmCount = 0;
			uint totalThreadPerBlockCount = 0;
			for (const auto& data : gpuInfo)
			{
				totalSmCount += data.multiProcessorCount;
				totalBlockPerSmCount += data.maxBlocksPerMultiProcessor;
				totalThreadPerBlockCount += data.maxThreadsPerBlock;
			}

			Graphic::JGraphicInfo graphicInfo = JGraphic::Instance().GetGraphicInfo();
			uint queryCount = graphicInfo.upObjCapacity > 0 ? graphicInfo.upObjCapacity : 1;
			//수정필요 
			//thread per group factor가 하드코딩됨
			//이후 amd graphic info 추가와 동시에 수정할 예정
			uint warpFactor = gpuInfo[0].vendor == Core::J_GRAPHIC_VENDOR::AMD ? 64 : 32;
			CalculateThreadDim(initHelper.dispatchInfo, queryCount, totalSmCount, warpFactor, 1024);

			StuffComputeShaderThreadDim(cFunctionFlag, initHelper.dispatchInfo.threadDim);
			hzbSamplingCount = std::to_string(graphicInfo.occlusionMapCapacity);
			hzbOcclusionQueryCount = std::to_string(queryCount);
			initHelper.macro.push_back({ hzbSamplingCountSymbol.c_str(), hzbSamplingCount.c_str() });
			initHelper.macro.push_back({ hzbOcclusionQueryCountSymbol.c_str(), hzbOcclusionQueryCount.c_str() });
			OutComputeShaderCommonMacro(initHelper.macro, cFunctionFlag);
			break;
		}
		default:
			break;
		}
		initHelper.macro.push_back({ NULL, NULL });
	}
	void JShader::GetInputLayout(_Out_ std::vector<D3D12_INPUT_ELEMENT_DESC>& outInputLayout, const J_SHADER_VERTEX_LAYOUT vertexLayoutFlag)noexcept
	{
		outInputLayout = inputLayout.find(vertexLayoutFlag)->second;
	}
	void JShader::CalculateThreadDim(JComputeShaderData::DispatchInfo& dInfo, const uint taskCount, const uint smCount, const uint warpFactor, const uint threadMaxDim)
	{
		if (taskCount < smCount)
		{
			dInfo.groupDim = JVector3<uint>(taskCount, 1, 1);
			dInfo.threadDim = JVector3<uint>(1, 1, 1);
			dInfo.threadCount = taskCount;
			dInfo.taskOriCount = taskCount;
		}
		else
		{
			const uint oneCycleMax = smCount * threadMaxDim;
			uint threadCount = oneCycleMax < taskCount ? oneCycleMax : taskCount; 
			threadCount += (threadCount % smCount == 0 ? 0 : (smCount - (threadCount % smCount)));

			uint threadPerGroup = threadCount / smCount;
			threadPerGroup += (threadPerGroup % warpFactor == 0 ? 0 : (warpFactor - (threadPerGroup % warpFactor)));

			dInfo.groupDim = JVector3<uint>(smCount, 1, 1);
			dInfo.threadDim = JVector3<uint>(threadPerGroup, 1, 1);
			dInfo.threadCount = threadCount;
			dInfo.taskOriCount = taskCount;
		}
	}
	void JShader::StuffComputeShaderThreadDim(const J_COMPUTE_SHADER_FUNCTION cFunctionFlag, const JVector3<uint> dim)
	{
		auto data = computeShaderThreadDim.find(cFunctionFlag);
		if (data == computeShaderThreadDim.end())
		{
			computeShaderThreadDim.emplace(cFunctionFlag, std::vector<std::string>{"", "", ""});
			data = computeShaderThreadDim.find(cFunctionFlag);
		}

		data->second[0] = std::to_string(dim[0]);
		data->second[1] = std::to_string(dim[1]);
		data->second[2] = std::to_string(dim[2]);
	}
	void JShader::OutComputeShaderCommonMacro(_Out_ std::vector<D3D_SHADER_MACRO>& outMacro, const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
	{
		auto dimValue = computeShaderThreadDim.find(cFunctionFlag);
		outMacro.push_back({ threadDimXSymbol.c_str(), dimValue->second[0].c_str() });
		outMacro.push_back({ threadDimYSymbol.c_str(), dimValue->second[1].c_str() });
		outMacro.push_back({ threadDimZSymbol.c_str(), dimValue->second[2].c_str() });
	}
	bool JShader::HasShaderData()const noexcept
	{
		bool hasData = true;
		for (uint i = 0; i < (uint)SHADER_VERTEX_COUNT; ++i)
			hasData = hasData && (gShaderData[i] != nullptr);
		return hasData;
	}
	Core::J_FILE_IO_RESULT JShader::CallStoreResource()
	{
		return StoreObject(this);
	}
	Core::J_FILE_IO_RESULT JShader::StoreObject(JShader* shader)
	{
		if (shader == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (((int)shader->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		std::wofstream stream;
		stream.open(shader->GetMetafilePath(), std::ios::out | std::ios::binary);
		Core::J_FILE_IO_RESULT res = StoreMetadata(stream, shader);
		stream.close();

		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		stream.open(shader->GetPath(), std::ios::out | std::ios::binary);
		if (stream.is_open())
		{
			JFileIOHelper::StoreEnumData(stream, L"ShaderFuncFlag:", shader->gFunctionFlag);
			JFileIOHelper::StoreEnumData(stream, L"ComputeShaderFuncFlag:", shader->cFunctionFlag);
			stream.close();
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JShader* JShader::LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;

		std::wifstream stream;
		stream.open(pathData.engineMetaFileWPath, std::ios::in | std::ios::binary);
		JResourceMetaData metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);
		stream.close();

		stream.open(pathData.engineFileWPath, std::ios::in | std::ios::binary);
		if (stream.is_open())
		{
			J_GRAPHIC_SHADER_FUNCTION gFunctionFlag;
			J_COMPUTE_SHADER_FUNCTION cFunctionFlag;
			JFileIOHelper::LoadEnumData(stream, gFunctionFlag);
			JFileIOHelper::LoadEnumData(stream, cFunctionFlag);
			stream.close();

			JShader* newShader = nullptr;
			if (directory->HasFile(pathData.name))
				newShader = JResourceManager::Instance().GetResourceByPath<JShader>(pathData.engineFileWPath);

			if (newShader == nullptr && loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
			{
				std::unique_ptr<JShaderInitdata> initdata;
				initdata = std::make_unique<JShaderInitdata>(metadata.guid, metadata.flag, gFunctionFlag, cFunctionFlag);

				if (initdata->IsValidLoadData())
				{
					Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JShader>(*initdata);
					newShader = ownerPtr.Get();
					if (!AddInstance(std::move(ownerPtr)))
						return nullptr;
				}
			}

			if (newShader == nullptr)
				return newShader;

			if (newShader->IsValid())
				return newShader;
			else
			{
				newShader->SetGraphicShaderFunctionFlag((J_GRAPHIC_SHADER_FUNCTION)gFunctionFlag);
				return newShader;
			}
		}
		else
			return nullptr;
	}
	void JShader::RegisterJFunc()
	{
		auto defaultC = [](Core::JOwnerPtr<JResourceInitData> initdata) ->JResourceObject*
		{
			if (initdata.IsValid() && initdata->GetResourceType() == J_RESOURCE_TYPE::SHADER && initdata->IsValidCreateData())
			{
				JShaderInitdata* sInitdata = static_cast<JShaderInitdata*>(initdata.Get());
				JShader* newShader = nullptr;
				if (sInitdata->cShaderFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
					newShader = FindOverlapShader(sInitdata->cShaderFunctionFlag);
				else
					newShader = FindOverlapShader(sInitdata->gShaderFunctionFlag);
				if (newShader != nullptr)
					return newShader;

				Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JShader>(*sInitdata);
				newShader = ownerPtr.Get();
				if (AddInstance(std::move(ownerPtr)))
				{
					StoreObject(newShader);
					return newShader;
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
			if (ori->GetResourceType() != J_RESOURCE_TYPE::SHADER)
				return nullptr;

			JShader* oriS = static_cast<JShader*>(ori);
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JShader>(InitData(Core::MakeGuid(),
				ori->GetFlag(),
				oriS->gFunctionFlag,
				oriS->cFunctionFlag));

			JShader* newShader = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				newShader->Copy(ori);
				return newShader;
			}
			else
				return nullptr;
		};
		JRFI<JShader>::Register(defaultC, loadC, copyC);

		auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex<JShader>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JShader::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JShader::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, true, false };
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{});
	}
	/*
	* ComputeShdaer Symbol
	{J_COMPUTE_SHADER_FUNCTION::HZB_DOWN_SAMPLING, {"MAPCOUNT", "DIMX", "DIMY", "DIMZ"}},
	{J_COMPUTE_SHADER_FUNCTION::HZB_OCCLUSION, {"MAPCOUNT", "DIMX", "DIMY", "DIMZ"}},
	*/
	JShader::JShader(const JShaderInitdata& initdata)
		: JShaderInterface(initdata), gFunctionFlag(initdata.gShaderFunctionFlag), cFunctionFlag(initdata.cShaderFunctionFlag)
	{}
	JShader::~JShader() {}
}