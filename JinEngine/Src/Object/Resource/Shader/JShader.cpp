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
		{SHADER_FUNCTION_ALPHA_CLIP, {"ALPHA_CLIP", "10"}},
		{SHADER_FUNCTION_WRITE_SHADOWMAP, {"WRITE_SHADOW_MAP", "11"}},
		{SHADER_FUNCTION_DEPTH_TEST_BOUNDING_OBJECT, {"BOUNDING_OBJECT_DEPTH_TEST", "12"}},
		{SHADER_FUNCTION_DEBUG, {"DEBUG", "13"}}
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
	
		using EqualGShaderT = Core::JStaticCallableType<bool, const J_GRAPHIC_SHADER_FUNCTION, const JShaderGraphicPsoCondition&, JShader*>;
		static std::unique_ptr<EqualGShaderT::Callable> equalGShader;

		using EqualCShaderT = Core::JStaticCallableType<bool, const J_COMPUTE_SHADER_FUNCTION, JShader*>;
		static std::unique_ptr<EqualCShaderT::Callable> equalCShader;
	}
	 
	static JShader* FindOverlapShader(const J_GRAPHIC_SHADER_FUNCTION newFunc, const JShaderGraphicPsoCondition& graphicPSO)
	{  
		uint count;
		std::vector<JResourceObject*>::const_iterator st = JResourceManager::Instance().GetResourceVectorHandle<JShader>(count);
		for (uint i = 0; i < count; ++i)
		{
			JShader* shader = static_cast<JShader*>(*(st + i));
			if ((*equalGShader)(nullptr, newFunc, graphicPSO, shader))
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
			if ((*equalCShader)(nullptr, newFunc, shader))
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
		const JShaderGraphicPsoCondition newGraphicPSO,
		const J_COMPUTE_SHADER_FUNCTION newCShaderFunctionFlag)
		:JResourceInitData(MakeName(newGShaderFunctionFlag, newGraphicPSO, newCShaderFunctionFlag),
			guid,
			Core::HasSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE) ? flag : Core::AddSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE),
			GetShaderDirectory(),
			JResourceObject::GetFormatIndex<JShader>(GetAvailableFormat()[0])),
		gShaderFunctionFlag(newGShaderFunctionFlag),
		cShaderFunctionFlag(newCShaderFunctionFlag),
		graphicPSO(newGraphicPSO)
	{
		if (cShaderFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
			gShaderFunctionFlag = SHADER_FUNCTION_NONE;
	}
	JShader::JShaderInitdata::JShaderInitdata(const J_OBJECT_FLAG flag,
		const J_GRAPHIC_SHADER_FUNCTION newGShaderFunctionFlag,
		const JShaderGraphicPsoCondition newGraphicPSO,
		const J_COMPUTE_SHADER_FUNCTION newCShaderFunctionFlag)
		: JResourceInitData(MakeName(newGShaderFunctionFlag, newGraphicPSO, newCShaderFunctionFlag),
			Core::MakeGuid(),
			Core::HasSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE) ? flag : Core::AddSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE),
			GetShaderDirectory(),
			JResourceObject::GetFormatIndex<JShader>(GetAvailableFormat()[0])),
		gShaderFunctionFlag(newGShaderFunctionFlag),
		cShaderFunctionFlag(newCShaderFunctionFlag),
		graphicPSO(newGraphicPSO)
	{
		if (cShaderFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
			gShaderFunctionFlag = SHADER_FUNCTION_NONE;
	}
	JShader::JShaderInitdata::JShaderInitdata(const J_GRAPHIC_SHADER_FUNCTION newGShaderFunctionFlag, 
		const JShaderGraphicPsoCondition newGraphicPSO,
		const J_COMPUTE_SHADER_FUNCTION newCShaderFunctionFlag)
		: JResourceInitData(MakeName(newGShaderFunctionFlag, newGraphicPSO, newCShaderFunctionFlag),
			Core::MakeGuid(),
			OBJECT_FLAG_UNEDITABLE,
			GetShaderDirectory(),
			JResourceObject::GetFormatIndex<JShader>(GetAvailableFormat()[0])),
		gShaderFunctionFlag(newGShaderFunctionFlag),
		cShaderFunctionFlag(newCShaderFunctionFlag),
		graphicPSO(newGraphicPSO)
	{
		if (cShaderFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
			gShaderFunctionFlag = SHADER_FUNCTION_NONE;
	}
	J_RESOURCE_TYPE JShader::JShaderInitdata::GetResourceType() const noexcept
	{
		return J_RESOURCE_TYPE::SHADER;
	}
	std::wstring JShader::JShaderInitdata::MakeName(const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
		const JShaderGraphicPsoCondition& graphicPSO,
		const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)noexcept
	{
		if (cFunctionFlag == J_COMPUTE_SHADER_FUNCTION::NONE)
			return JShaderType::ConvertToName(gFunctionFlag, graphicPSO.UniqueID());
		else
			return JShaderType::ConvertToName(cFunctionFlag);
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
		return cShaderData->pso.Get();
	}
	ID3D12PipelineState* JShader::GetGraphicPso(const J_SHADER_VERTEX_LAYOUT vertexLayout)const noexcept
	{
		return gShaderData[(int)vertexLayout]->pso.Get();
	}
	ID3D12PipelineState* JShader::GetGraphicExtraPso(const J_SHADER_VERTEX_LAYOUT vertexLayout, const J_GRAPHIC_EXTRA_PSO_TYPE type)const noexcept
	{ 
		return gShaderData[(int)vertexLayout]->extraPso[(int)type].Get();
	}
	ID3D12RootSignature* JShader::GetComputeRootSignature()const noexcept
	{
		return cShaderData->RootSignature;
	}
	JVector3<uint> JShader::GetComputeGroupDim()const noexcept
	{
		return IsComputeShader() ? cShaderData->dispatchInfo.groupDim : JVector3<uint>(0, 0, 0);
	}
	J_GRAPHIC_SHADER_FUNCTION JShader::GetShaderGFunctionFlag()const noexcept
	{
		return gFunctionFlag;
	}
	J_COMPUTE_SHADER_FUNCTION JShader::GetShdaerCFunctionFlag()const noexcept
	{
		return cFunctionFlag;
	}
	JShaderGraphicPsoCondition JShader::GetSubGraphicPso()const noexcept
	{
		return graphicPSO;
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
		for (int i = 0; i < SHADER_VERTEX_COUNT; ++i)
		{
			if (gShaderData[i] != nullptr)
			{
				gShaderData[i]->Clear();
				gShaderData[i].reset();
			}
		}
		if (cShaderData != nullptr)
		{
			cShaderData->Clear();
			cShaderData.reset();
		}
		SetValid(false);
	}
	bool JShader::Destroy(const bool isForced)
	{
		if (!JResourceObject::Destroy(isForced))
			return false;

		const bool hasUnDestroyable = HasFlag(J_OBJECT_FLAG::OBJECT_FLAG_UNDESTROYABLE);
		if (!hasUnDestroyable)
			DeleteRFile();		// delete shader file in memory
		return true;
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
	void JShader::SetComputeShaderFunctionFlag(const J_COMPUTE_SHADER_FUNCTION newFunctionFlag)
	{
		if (cFunctionFlag != newFunctionFlag || !HasShaderData())
		{
			cFunctionFlag = newFunctionFlag;
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
		for (uint i = 0; i < SHADER_VERTEX_COUNT; ++i)
			GetMacroVec(macroVec[i], (J_SHADER_VERTEX_LAYOUT)(SHADER_VERTEX_LAYOUT_STATIC + i), shader->GetShaderGFunctionFlag());

		std::wstring vertexShaderPath = JApplicationVariable::GetShaderPath() + L"\\VertexShader.hlsl";
		std::wstring pixelShaderPath = JApplicationVariable::GetShaderPath() + L"\\PixelShader.hlsl";

		for (uint i = 0; i < SHADER_VERTEX_COUNT; ++i)
		{
			shader->gShaderData[i] = std::make_unique<JGraphicShaderData>();
			shader->gShaderData[i]->vs = JD3DUtility::CompileShader(vertexShaderPath, &macroVec[i][0], "VS", "vs_5_1");
			if ((shader->gFunctionFlag & SHADER_FUNCTION_WRITE_SHADOWMAP) == 0 && (shader->gFunctionFlag & SHADER_FUNCTION_DEPTH_TEST_BOUNDING_OBJECT) == 0)
				shader->gShaderData[i]->ps = JD3DUtility::CompileShader(pixelShaderPath, &macroVec[i][0], "PS", "ps_5_1");
			GetInputLayout(shader->gShaderData[i]->inputLayout, (J_SHADER_VERTEX_LAYOUT)(SHADER_VERTEX_LAYOUT_STATIC + i));
			JGraphic::Instance().ResourceInterface()->StuffGraphicShaderPso(shader->gShaderData[i].get(), 
				(J_SHADER_VERTEX_LAYOUT)(SHADER_VERTEX_LAYOUT_STATIC + i), 
				shader->GetShaderGFunctionFlag(),
				shader->GetSubGraphicPso());

			for (uint j = 0; j < (uint)J_GRAPHIC_EXTRA_PSO_TYPE::COUNT; ++j)
			{
				JGraphic::Instance().ResourceInterface()->StuffGraphicShaderPso(shader->gShaderData[i].get(),
					(J_SHADER_VERTEX_LAYOUT)(SHADER_VERTEX_LAYOUT_STATIC + i),
					shader->GetShaderGFunctionFlag(),
					shader->GetSubGraphicPso(),
					(J_GRAPHIC_EXTRA_PSO_TYPE)j);
			}
		}
	}
	void JShader::CompileComputeShader(JShader* shader)
	{
		CSInitHelper initHelper;
		GetInitHelper(initHelper, shader->GetShdaerCFunctionFlag());

		//macroVec = std::vector<D3D_SHADER_MACRO>{ { NULL, NULL } };
		if (shader->cFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
		{
			JShaderType::CompileInfo compileInfo = JShaderType::ComputeShaderCompileInfo(shader->cFunctionFlag);
			std::wstring computeShaderPath = JApplicationVariable::GetShaderPath() + L"\\" + compileInfo.fileName;
			shader->cShaderData = std::make_unique<JComputeShaderData>();
			shader->cShaderData->cs = JD3DUtility::CompileShader(computeShaderPath, &initHelper.macro[0], compileInfo.functionName, "cs_5_1");
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

		auto InitHZBMaps = [](_Out_ CSInitHelper& initHelper, const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
		{
			std::vector<GpuInfo> gpuInfo = Core::JHardwareInfo::Instance().GetGpuInfo();
			Graphic::JGraphicInfo graphicInfo = JGraphic::Instance().GetGraphicInfo();

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

			StuffComputeShaderThreadDim(cFunctionFlag, initHelper.dispatchInfo.threadDim);
			OutComputeShaderCommonMacro(initHelper.macro, cFunctionFlag);
		};

		switch (cFunctionFlag)
		{
		case JinEngine::J_COMPUTE_SHADER_FUNCTION::HZB_COPY:
		{
			InitHZBMaps(initHelper, cFunctionFlag);
			break;
		}
		case JinEngine::J_COMPUTE_SHADER_FUNCTION::HZB_DOWN_SAMPLING:
		{
			InitHZBMaps(initHelper, cFunctionFlag);
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
			//graphicInfo.upObjCapacity always 2 squared
			uint queryCount = graphicInfo.upObjCapacity > 0 ? graphicInfo.upObjCapacity : 1;

			//수정필요 
			//thread per group factor가 하드코딩됨
			//이후 amd graphic info 추가와 동시에 수정할 예정
			uint warpFactor = gpuInfo[0].vendor == Core::J_GRAPHIC_VENDOR::AMD ? 64 : 32;
			if (queryCount < warpFactor)
			{
				initHelper.dispatchInfo.threadDim = JVector3<uint>(queryCount, 1, 1);
				initHelper.dispatchInfo.groupDim = JVector3<uint>(1, 1, 1);
				initHelper.dispatchInfo.taskOriCount = queryCount;
			}
			else
			{
				initHelper.dispatchInfo.threadDim = JVector3<uint>(warpFactor, 1, 1);
				initHelper.dispatchInfo.groupDim = JVector3<uint>(queryCount / warpFactor, 1, 1);
				initHelper.dispatchInfo.taskOriCount = queryCount;
			}

			StuffComputeShaderThreadDim(cFunctionFlag, initHelper.dispatchInfo.threadDim);
			hzbOcclusionQueryCount = std::to_string(queryCount);
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
			JFileIOHelper::StoreEnumData(stream, L"SubPsoPrimitiveCondition:", shader->graphicPSO.primitiveCondition);
			JFileIOHelper::StoreEnumData(stream, L"SubPsoDepthComparesionCondition:", shader->graphicPSO.depthCompareCondition);
			JFileIOHelper::StoreEnumData(stream, L"SubPsoCullModeCondition:", shader->graphicPSO.cullModeCondition);

			JFileIOHelper::StoreEnumData(stream, L"SubPsoPrimitive:", shader->graphicPSO.primitiveType);
			JFileIOHelper::StoreEnumData(stream, L"SubPsoDepthComparesion:", shader->graphicPSO.depthCompareFunc);
			JFileIOHelper::StoreAtomicData(stream, L"SubPsoCullMode:", shader->graphicPSO.isCullModeNone);
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
			JShaderGraphicPsoCondition psoCondition; 
			bool sCullMode;

			JFileIOHelper::LoadEnumData(stream, gFunctionFlag);
			JFileIOHelper::LoadEnumData(stream, cFunctionFlag);

			JFileIOHelper::LoadEnumData(stream, psoCondition.primitiveCondition);
			JFileIOHelper::LoadEnumData(stream, psoCondition.depthCompareCondition);
			JFileIOHelper::LoadEnumData(stream, psoCondition.cullModeCondition);
			JFileIOHelper::LoadEnumData(stream, psoCondition.primitiveType);
			JFileIOHelper::LoadEnumData(stream, psoCondition.depthCompareFunc);
			JFileIOHelper::LoadAtomicData(stream, psoCondition.isCullModeNone);
			stream.close();
			  
			JShader* newShader = nullptr;
			if (directory->HasFile(pathData.name))
				newShader = JResourceManager::Instance().GetResourceByPath<JShader>(pathData.engineFileWPath);

			if (newShader == nullptr && loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
			{
				std::unique_ptr<JShaderInitdata> initdata;
				initdata = std::make_unique<JShaderInitdata>(metadata.guid, metadata.flag, gFunctionFlag, psoCondition, cFunctionFlag);

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
				if (newShader->IsComputeShader())
					newShader->SetComputeShaderFunctionFlag((J_COMPUTE_SHADER_FUNCTION)cFunctionFlag);
				else
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
					newShader = FindOverlapShader(sInitdata->gShaderFunctionFlag, sInitdata->graphicPSO);
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
				oriS->graphicPSO,
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

		/*
		using EqualGShaderT = Core::JStaticCallableType<bool, const J_GRAPHIC_SHADER_FUNCTION, const JShaderGraphicPsoCondition&, JShader*>;
		using EqualCShaderT = Core::JStaticCallableType<bool, const J_COMPUTE_SHADER_FUNCTION, JShader*>;
		*/
		auto equalGShaderLam = [](const J_GRAPHIC_SHADER_FUNCTION newFunc, const JShaderGraphicPsoCondition& graphicPSO, JShader* shader)
		{
			return !shader->IsComputeShader() && shader->GetShaderGFunctionFlag() == newFunc && shader->graphicPSO.Equal(graphicPSO);
		};
		auto equalCShaderLam = [](const J_COMPUTE_SHADER_FUNCTION newFunc, JShader* shader)
		{
			return shader->IsComputeShader() && shader->GetShdaerCFunctionFlag() == newFunc;
		};

		equalGShader = std::make_unique<EqualGShaderT::Callable>(equalGShaderLam);
		equalCShader = std::make_unique<EqualCShaderT::Callable>(equalCShaderLam);
	}
	/*
	* ComputeShdaer Symbol
	{J_COMPUTE_SHADER_FUNCTION::HZB_DOWN_SAMPLING, {"MAPCOUNT", "DIMX", "DIMY", "DIMZ"}},
	{J_COMPUTE_SHADER_FUNCTION::HZB_OCCLUSION, {"MAPCOUNT", "DIMX", "DIMY", "DIMZ"}},
	*/
	JShader::JShader(const JShaderInitdata& initdata)
		: JShaderInterface(initdata),
		gFunctionFlag(initdata.gShaderFunctionFlag),
		cFunctionFlag(initdata.cShaderFunctionFlag),
		graphicPSO(initdata.graphicPSO)
	{}
	JShader::~JShader() {}
}