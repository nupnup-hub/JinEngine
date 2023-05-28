#include"JShader.h" 
#include"JShaderPrivate.h"
#include"../JResourceObjectHint.h"
#include"../JResourceManager.h" 
#include"../../Directory/JDirectory.h"
#include"../../Directory/JFile.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Platform/JHardwareInfo.h"
#include"../../../Graphic/JGraphic.h"
#include"../../../Graphic/JGraphicPrivate.h"
#include"../../../Application/JApplicationEngine.h"
#include"../../../Application/JApplicationProject.h"
#include"../../../Utility/JD3DUtility.h"
#include"../../../Utility/JCommonUtility.h"
#include<fstream>
#include<d3d12.h>

namespace JinEngine
{
	namespace
	{
//Graphic Shader Macro Symbol  
#define TEXTURE_2D_COUNT_SYMBOL "TEXTURE_2D_COUNT"
#define TEXTURE_CUBE_COUNT_SYMBOL "CUBE_MAP_COUNT"
#define SHADOW_MAP_COUNT_SYMBOL "SHADOW_MAP_COUNT"

//Compute Shader Macro Symbol ";
#define THREAD_DIM_X_SYMBOL "DIMX"
#define THREAD_DIM_Y_SYMBOL "DIMY"
#define THREAD_DIM_Z_SYMBOL "DIMZ"

#define HZB_SAMPLING_COUNT_SYMBOL "DOWN_SAMPLING_COUNT"
#define HZB_OCC_QUERY_COUNT_SYMBOL "OCCLUSION_QUERY_COUNT" 
	}
 
	struct GSInitHelper
	{
	public:
		std::vector<D3D_SHADER_MACRO> macro[SHADER_VERTEX_COUNT];
		std::vector<std::string> macroStrVec[SHADER_VERTEX_COUNT];
	};
	struct CSInitHelper
	{
	public:
		std::vector<D3D_SHADER_MACRO> macro;
		std::vector<std::string> macroStrVec;
		JComputeShaderData::DispatchInfo dispatchInfo;
	};

	namespace
	{
		static const std::unordered_map<J_GRAPHIC_SHADER_FUNCTION, const D3D_SHADER_MACRO> shaderFuncMacroMap
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
		static const std::unordered_map<J_SHADER_VERTEX_LAYOUT, const D3D_SHADER_MACRO> vertexLayoutMacroMap
		{
			{SHADER_VERTEX_LAYOUT_STATIC, {"STATIC", "1"}},
			{SHADER_VERTEX_LAYOUT_SKINNED, {"SKINNED", "2"}},
		};
		static const std::unordered_map<J_SHADER_VERTEX_LAYOUT, std::vector<D3D12_INPUT_ELEMENT_DESC>> inputLayout
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
	}

	namespace
	{
		using CSInitHelperCallable = Core::JStaticCallable<CSInitHelper>;
		using EqualGShaderT = Core::JStaticCallableType<bool, const J_GRAPHIC_SHADER_FUNCTION, const JShaderGraphicPsoCondition&, JShader*>;
		using EqualCShaderT = Core::JStaticCallableType<bool, const J_COMPUTE_SHADER_FUNCTION, JShader*>;
		using GResourceInterface = Graphic::JGraphicPrivate::ResourceInterface;

		static std::unique_ptr<EqualGShaderT::Callable> equalGShader = nullptr;
		static std::unique_ptr<EqualCShaderT::Callable> equalCShader = nullptr;
		static std::unordered_map <J_COMPUTE_SHADER_FUNCTION, std::vector<std::string>> computeShaderThreadDim;

		static JShaderPrivate sPrivate;
	}
 
	namespace
	{
		static JUserPtr<JDirectory> GetShaderDirectory()
		{
			return _JResourceManager::Instance().GetDirectory(JApplicationProject::ShaderMetafilePath());
		}
		/*
		static JShader* FindOverlapShader(const J_GRAPHIC_SHADER_FUNCTION newFunc, const JShaderGraphicPsoCondition& graphicPSOCond)
		{
			uint count;
			std::vector<JResourceObject*>::const_iterator st = _JResourceManager::Instance().Instance().GetResourceVectorHandle<JShader>(count);
			for (uint i = 0; i < count; ++i)
			{
				JShader* shader = static_cast<JShader*>(*(st + i));
				if ((*equalGShader)(nullptr, newFunc, graphicPSOCond, shader))
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
			JUserPtr<JFile> file = GetShaderDirectory()->GetDirectoryFile(name);
			return file != nullptr ? Core::ConvertChildUserPtr<JShader>(file->TryGetResourceUser()) : nullptr;
		}
		static std::wstring MakeName(const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
			const JShaderGraphicPsoCondition& graphicPSOCond,
			const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)noexcept
		{
			if (cFunctionFlag == J_COMPUTE_SHADER_FUNCTION::NONE)
				return JShaderType::ConvertToName(gFunctionFlag, graphicPSOCond.UniqueID());
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
		std::unique_ptr<JGraphicShaderData>gShaderData[SHADER_VERTEX_COUNT]{ nullptr, nullptr };
		std::unique_ptr<JComputeShaderData> cShaderData = nullptr;
		J_GRAPHIC_SHADER_FUNCTION gFunctionFlag = SHADER_FUNCTION_NONE;
		J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE;
	public:
		JShaderGraphicPsoCondition graphicPSOCond;
	public:
		JShaderImpl(const InitData& initData, JShader* thisShaderRaw)
			:gFunctionFlag(initData.gFunctionFlag),
			cFunctionFlag(initData.cFunctionFlag),
			graphicPSOCond(initData.graphicPSOCond)
		{ }
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
			if (gFunctionFlag != newFunctionFlag || !HasShaderData())
			{
				gFunctionFlag = newFunctionFlag;
				if (thisPointer->IsActivated())
					CompileShdaer();
			}
		}
		void SetComputeShaderFunctionFlag(const J_COMPUTE_SHADER_FUNCTION newFunctionFlag)
		{
			if (cFunctionFlag != newFunctionFlag || !HasShaderData())
			{
				cFunctionFlag = newFunctionFlag;
				if (thisPointer->IsActivated())
					CompileShdaer();
			}
		}
	public:
		bool HasShaderData()const noexcept
		{
			bool hasData = true;
			for (uint i = 0; i < (uint)SHADER_VERTEX_COUNT; ++i)
				hasData = hasData && (gShaderData[i] != nullptr);
			return hasData;
		}
	public:
		void RecompileGraphicShader()
		{
			if (thisPointer->IsActivated())
			{
				for (uint i = 0; i < SHADER_VERTEX_COUNT; ++i)
					gShaderData[i].reset();
				CompileShdaer();
			}
		}
		void RecompileComputeShader()
		{
			if (thisPointer->IsActivated())
			{
				cShaderData.reset();
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
			GSInitHelper initHelper;
			StuffInitHelper(initHelper, thisPointer->GetShaderGFunctionFlag());

			std::wstring vertexShaderPath = JApplicationEngine::ShaderPath() + L"\\VertexShader.hlsl";
			std::wstring pixelShaderPath = JApplicationEngine::ShaderPath() + L"\\PixelShader.hlsl";

			 
			for (uint i = 0; i < SHADER_VERTEX_COUNT; ++i)
			{ 
				gShaderData[i] = std::make_unique<JGraphicShaderData>();
				gShaderData[i]->vs = JD3DUtility::CompileShader(vertexShaderPath, &initHelper.macro[i][0], "VS", "vs_5_1");
				if ((gFunctionFlag & SHADER_FUNCTION_WRITE_SHADOWMAP) == 0 && (gFunctionFlag & SHADER_FUNCTION_DEPTH_TEST_BOUNDING_OBJECT) == 0)
					gShaderData[i]->ps = JD3DUtility::CompileShader(pixelShaderPath, &initHelper.macro[i][0], "PS", "ps_5_1");
				StuffInputLayout(gShaderData[i]->inputLayout, (J_SHADER_VERTEX_LAYOUT)(SHADER_VERTEX_LAYOUT_STATIC + i));
				
				GResourceInterface::StuffGraphicShaderPso(gShaderData[i].get(),
					(J_SHADER_VERTEX_LAYOUT)(SHADER_VERTEX_LAYOUT_STATIC + i),
					thisPointer->GetShaderGFunctionFlag(),
					thisPointer->GetSubGraphicPso());

				for (uint j = 0; j < (uint)J_GRAPHIC_EXTRA_PSO_TYPE::COUNT; ++j)
				{
					GResourceInterface::StuffGraphicShaderPso(gShaderData[i].get(),
						(J_SHADER_VERTEX_LAYOUT)(SHADER_VERTEX_LAYOUT_STATIC + i),
						thisPointer->GetShaderGFunctionFlag(),
						thisPointer->GetSubGraphicPso(),
						(J_GRAPHIC_EXTRA_PSO_TYPE)j);
				}
			}
		}
		void CompileComputeShader()
		{
			CSInitHelper initHelper;
			StuffInitHelper(initHelper, thisPointer->GetShdaerCFunctionFlag());
			 
			//macroVec = std::vector<D3D_SHADER_MACRO>{ { NULL, NULL } };
			if (cFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
			{
				JShaderType::CompileInfo compileInfo = JShaderType::ComputeShaderCompileInfo(cFunctionFlag);
				std::wstring computeShaderPath = JApplicationEngine::ShaderPath() + L"\\" + compileInfo.fileName;
				cShaderData = std::make_unique<JComputeShaderData>();
				cShaderData->cs = JD3DUtility::CompileShader(computeShaderPath, &initHelper.macro[0], compileInfo.functionName, "cs_5_1");
				cShaderData->dispatchInfo = initHelper.dispatchInfo;
				GResourceInterface::StuffComputeShaderPso(cShaderData.get(), cFunctionFlag);
			}
		}
		static void StuffInitHelper(_Out_ GSInitHelper& initHelper, const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag)noexcept
		{
			Graphic::JGraphicInfo info = JGraphic::Instance().GetGraphicInfo();
			for (uint i = 0; i < (uint)J_SHADER_VERTEX_LAYOUT::SHADER_VERTEX_COUNT; ++i)
			{
				J_SHADER_VERTEX_LAYOUT layount = (J_SHADER_VERTEX_LAYOUT)i;
				initHelper.macro[i].push_back(vertexLayoutMacroMap.find(layount)->second);
				for (const auto& data : shaderFuncMacroMap)
				{
					if ((data.first & gFunctionFlag) > 0)
						initHelper.macro[i].push_back(data.second);
				}

				initHelper.macroStrVec[i].push_back(std::to_string(info.binding2DTextureCapacity));
				initHelper.macroStrVec[i].push_back(std::to_string(info.bindingCubeMapCapacity));
				initHelper.macroStrVec[i].push_back(std::to_string(info.initBindTShadowCapacity));

				initHelper.macro[i].push_back({ TEXTURE_2D_COUNT_SYMBOL, initHelper.macroStrVec[i][0].c_str() });
				initHelper.macro[i].push_back({ TEXTURE_CUBE_COUNT_SYMBOL, initHelper.macroStrVec[i][1].c_str() });
				initHelper.macro[i].push_back({ SHADOW_MAP_COUNT_SYMBOL, initHelper.macroStrVec[i][2].c_str() });
				initHelper.macro[i].push_back(shaderFuncMacroMap.find(SHADER_FUNCTION_NONE)->second);
			}
		}
		static void StuffInitHelper(_Out_ CSInitHelper& initHelper, const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)noexcept
		{
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

			auto InitHZBMaps = [](_Out_ CSInitHelper& initHelper, const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
			{
				std::vector<GpuInfo> gpuInfo = Core::JHardwareInfo::GetGpuInfo();
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
				StuffComputeShaderCommonMacro(initHelper, cFunctionFlag);
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
				std::vector<GpuInfo> gpuInfo = Core::JHardwareInfo::GetGpuInfo();
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

				initHelper.macroStrVec.push_back(std::to_string(queryCount));
				initHelper.macro.push_back({ HZB_OCC_QUERY_COUNT_SYMBOL, initHelper.macroStrVec[0].c_str() });
				StuffComputeShaderCommonMacro(initHelper, cFunctionFlag);
				break;
			}
			default:
				break;
			}
			initHelper.macro.push_back({ NULL, NULL });
		}
		static void StuffInputLayout(_Out_ std::vector<D3D12_INPUT_ELEMENT_DESC>& outInputLayout, const J_SHADER_VERTEX_LAYOUT vertexLayoutFlag)noexcept
		{
			outInputLayout = inputLayout.find(vertexLayoutFlag)->second;
		}
		static void StuffComputeShaderCommonMacro(_Out_ CSInitHelper& initHelper, const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
		{
			int offset = initHelper.macroStrVec.size();
			initHelper.macroStrVec.push_back(std::to_string(initHelper.dispatchInfo.threadDim.x));
			initHelper.macroStrVec.push_back(std::to_string(initHelper.dispatchInfo.threadDim.y));
			initHelper.macroStrVec.push_back(std::to_string(initHelper.dispatchInfo.threadDim.z));

			initHelper.macro.push_back({ THREAD_DIM_X_SYMBOL, initHelper.macroStrVec[offset].c_str() });
			initHelper.macro.push_back({ THREAD_DIM_Y_SYMBOL, initHelper.macroStrVec[offset + 1].c_str() });
			initHelper.macro.push_back({ THREAD_DIM_Z_SYMBOL, initHelper.macroStrVec[offset + 2].c_str() });
		}
	public:
		void ClearShaderData()
		{
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

			/*
			using EqualGShaderT = Core::JStaticCallableType<bool, const J_GRAPHIC_SHADER_FUNCTION, const JShaderGraphicPsoCondition&, JShader*>;
			using EqualCShaderT = Core::JStaticCallableType<bool, const J_COMPUTE_SHADER_FUNCTION, JShader*>;
			*/
			auto equalGShaderLam = [](const J_GRAPHIC_SHADER_FUNCTION newFunc, const JShaderGraphicPsoCondition& graphicPSOCond, JShader* shader)
			{
				return !shader->IsComputeShader() && shader->GetShaderGFunctionFlag() == newFunc && shader->impl->graphicPSOCond == graphicPSOCond;
			};
			auto equalCShaderLam = [](const J_COMPUTE_SHADER_FUNCTION newFunc, JShader* shader)
			{
				return shader->IsComputeShader() && shader->GetShdaerCFunctionFlag() == newFunc;
			};

			equalGShader = std::make_unique<EqualGShaderT::Callable>(equalGShaderLam);
			equalCShader = std::make_unique<EqualCShaderT::Callable>(equalCShaderLam);
			Core::JIdentifier::RegisterPrivateInterface(JShader::StaticTypeInfo(), sPrivate);

			IMPL_REALLOC_BIND(JShader::JShaderImpl, thisPointer)
		}
	};

	JShader::InitData::InitData(const J_OBJECT_FLAG flag, 
		const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
		const JShaderGraphicPsoCondition graphicPSOCond,
		const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
		:JResourceObject::InitData(JShader::StaticTypeInfo(), GetDefaultFormatIndex(), GetStaticResourceType(), GetShaderDirectory()),
		gFunctionFlag(gFunctionFlag), 
		graphicPSOCond(graphicPSOCond), 
		cFunctionFlag(cFunctionFlag)
	{
		InitData::flag = Core::HasSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE) ? flag : Core::AddSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE);
		if (cFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
			InitData::gFunctionFlag = SHADER_FUNCTION_NONE;
		name = MakeName(InitData::gFunctionFlag, graphicPSOCond, cFunctionFlag);
		name = directory->MakeUniqueFileName(name, RTypeCommonCall::GetFormat(rType, formatIndex), guid);
	}
	JShader::InitData::InitData(const std::wstring& name,
		const size_t& guid,
		const J_OBJECT_FLAG flag,
		const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
		const JShaderGraphicPsoCondition graphicPSOCond,
		const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
		: JResourceObject::InitData(JShader::StaticTypeInfo(), name, guid, flag, GetDefaultFormatIndex(), GetStaticResourceType(), GetShaderDirectory()),
		gFunctionFlag(gFunctionFlag),
		graphicPSOCond(graphicPSOCond),
		cFunctionFlag(cFunctionFlag)
	{
		InitData::flag = Core::HasSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE) ? flag : Core::AddSQValueEnum(flag, OBJECT_FLAG_UNEDITABLE);
		if (cFunctionFlag != J_COMPUTE_SHADER_FUNCTION::NONE)
			InitData::gFunctionFlag = SHADER_FUNCTION_NONE;
		InitData::name = MakeName(InitData::gFunctionFlag, graphicPSOCond, cFunctionFlag);
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
	ID3D12PipelineState* JShader::GetComputePso()const noexcept
	{
		return impl->cShaderData->pso.Get();
	}
	ID3D12PipelineState* JShader::GetGraphicPso(const J_SHADER_VERTEX_LAYOUT vertexLayout)const noexcept
	{
		return impl->gShaderData[(int)vertexLayout]->pso.Get();
	}
	ID3D12PipelineState* JShader::GetGraphicExtraPso(const J_SHADER_VERTEX_LAYOUT vertexLayout, const J_GRAPHIC_EXTRA_PSO_TYPE type)const noexcept
	{ 
		return impl->gShaderData[(int)vertexLayout]->extraPso[(int)type].Get();
	}
	ID3D12RootSignature* JShader::GetComputeRootSignature()const noexcept
	{
		return impl->cShaderData->RootSignature;
	}
	JVector3<uint> JShader::GetComputeGroupDim()const noexcept
	{
		return IsComputeShader() ? impl->cShaderData->dispatchInfo.groupDim : JVector3<uint>(0, 0, 0);
	}
	J_GRAPHIC_SHADER_FUNCTION JShader::GetShaderGFunctionFlag()const noexcept
	{
		return impl->gFunctionFlag;
	}
	J_COMPUTE_SHADER_FUNCTION JShader::GetShdaerCFunctionFlag()const noexcept
	{
		return impl->cFunctionFlag;
	}
	JShaderGraphicPsoCondition JShader::GetSubGraphicPso()const noexcept
	{
		return impl->graphicPSOCond;
	} 
	bool JShader::IsComputeShader()const noexcept
	{
		return impl->IsComputeShader();
	}
	JUserPtr<JShader> JShader::FindShader(const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
		const JShaderGraphicPsoCondition graphicPSOCond,
		const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)
	{
		return FindOverlapShader(MakeName(gFunctionFlag, graphicPSOCond, cFunctionFlag));
	}
	void JShader::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
		impl->CompileShdaer();
		SetValid(true);
	}
	void JShader::DoDeActivate()noexcept
	{
		JResourceObject::DoDeActivate();
		impl->ClearShaderData();
		SetValid(false);
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
		if (LoadMetaData(pathData.engineMetaFileWPath, initData.get()) != Core::J_FILE_IO_RESULT::SUCCESS)
			return nullptr;

		JUserPtr<JShader> newShdaer = nullptr;
		if (directory->HasFile(initData->guid))
			newShdaer = Core::GetUserPtr<JShader>(JShader::StaticTypeInfo().TypeGuid(), initData->guid);

		if (newShdaer == nullptr)
		{
			initData->name = MakeName(initData->gFunctionFlag, initData->graphicPSOCond, initData->cFunctionFlag);
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

		std::wifstream stream;
		stream.open(path, std::ios::in | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		auto loadMetaData = static_cast<JShader::InitData*>(data);
		if (LoadCommonMetaData(stream, loadMetaData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
 
		JFileIOHelper::LoadEnumData(stream, loadMetaData->gFunctionFlag);
		JFileIOHelper::LoadEnumData(stream, loadMetaData->cFunctionFlag);

		JFileIOHelper::LoadEnumData(stream, loadMetaData->graphicPSOCond.primitiveCondition);
		JFileIOHelper::LoadEnumData(stream, loadMetaData->graphicPSOCond.depthCompareCondition);
		JFileIOHelper::LoadEnumData(stream, loadMetaData->graphicPSOCond.cullModeCondition);
		JFileIOHelper::LoadEnumData(stream, loadMetaData->graphicPSOCond.primitiveType);
		JFileIOHelper::LoadEnumData(stream, loadMetaData->graphicPSOCond.depthCompareFunc);
		JFileIOHelper::LoadAtomicData(stream, loadMetaData->graphicPSOCond.isCullModeNone);

		stream.close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreMetaData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JShader::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JShader::StoreData*>(data);
		JUserPtr<JShader> shader;
		shader.ConnnectChild(storeData->obj);

		std::wofstream stream;
		stream.open(shader->GetMetaFilePath(), std::ios::out | std::ios::binary);
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (StoreCommonMetaData(stream, storeData) != Core::J_FILE_IO_RESULT::SUCCESS)
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		JFileIOHelper::StoreEnumData(stream, L"ShaderFuncFlag:", shader->impl->gFunctionFlag);
		JFileIOHelper::StoreEnumData(stream, L"ComputeShaderFuncFlag:", shader->impl->cFunctionFlag);

		JFileIOHelper::StoreEnumData(stream, L"SubPsoPrimitiveCondition:", shader->impl->graphicPSOCond.primitiveCondition);
		JFileIOHelper::StoreEnumData(stream, L"SubPsoDepthComparesionCondition:", shader->impl->graphicPSOCond.depthCompareCondition);
		JFileIOHelper::StoreEnumData(stream, L"SubPsoCullModeCondition:", shader->impl->graphicPSOCond.cullModeCondition);
		JFileIOHelper::StoreEnumData(stream, L"SubPsoPrimitive:", shader->impl->graphicPSOCond.primitiveType);
		JFileIOHelper::StoreEnumData(stream, L"SubPsoDepthComparesion:", shader->impl->graphicPSOCond.depthCompareFunc);
		JFileIOHelper::StoreAtomicData(stream, L"SubPsoCullMode:", shader->impl->graphicPSOCond.isCullModeNone);

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