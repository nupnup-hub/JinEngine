#include"JShader.h" 
#include"JShaderData.h" 
#include"../JResourceManager.h"
#include"../JResourceObjectFactory.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Graphic/JGraphic.h"
#include"../../../Application/JApplicationVariable.h"
#include"../../../Utility/JD3DUtility.h"
#include"../../../Utility/JCommonUtility.h"
#include<fstream>

namespace JinEngine
{
	std::unordered_map<J_SHADER_FUNCTION, const D3D_SHADER_MACRO> JShader::shaderFuncMacroMap
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
		{SHADER_FUNCTION_SHADOW_MAP, {"SHADOW_MAP", "12"}},
		{SHADER_FUNCTION_DEBUG, {"DEBUG", "13"}},
	};
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
	ID3D12PipelineState* JShader::GetPso(const J_SHADER_VERTEX_LAYOUT vertexLayout)const noexcept
	{
		return shaderData[(int)vertexLayout]->Pso.Get();
	}
	J_SHADER_FUNCTION JShader::GetShaderFunctionFlag()const noexcept
	{
		return functionFlag;
	}
	J_RESOURCE_TYPE JShader::GetResourceType()const noexcept
	{
		return GetStaticResourceType();
	} 
	std::string JShader::GetFormat()const noexcept
	{
		return GetAvailableFormat()[0];
	}
	std::vector<std::string> JShader::GetAvailableFormat()noexcept
	{
		static std::vector<std::string> format{ ".shader" };
		return format;
	}
	J_SHADER_FUNCTION JShader::AddShaderFunctionFlag(const J_SHADER_FUNCTION ori, const J_SHADER_FUNCTION addValue)noexcept
	{
		return (J_SHADER_FUNCTION)(ori | (addValue ^ (addValue & ori)));
	}
	J_SHADER_FUNCTION JShader::MinusShaderFunctionFlag(const J_SHADER_FUNCTION ori, const J_SHADER_FUNCTION minusValue)noexcept
	{
		return (J_SHADER_FUNCTION)(ori ^ (minusValue & ori));
	}
	void JShader::DoActivate()noexcept
	{
		JResourceObject::DoActivate();
	}
	void JShader::DoDeActivate()noexcept
	{
		JResourceObject::DoDeActivate();
	}
	void JShader::SetShaderFunctionFlag(const J_SHADER_FUNCTION newFunctionFlag)
	{
		functionFlag = newFunctionFlag;
	}
	void JShader::CompileShdaer(JShader* shader)
	{
		if (shader == nullptr)
			return;

		std::vector<D3D_SHADER_MACRO> macroVec[SHADER_VERTEX_COUNT];
		for (uint j = 0; j < SHADER_VERTEX_COUNT; ++j)
			GetMacroVec(macroVec[j], (J_SHADER_VERTEX_LAYOUT)(SHADER_VERTEX_LAYOUT_STATIC + j), shader->GetShaderFunctionFlag());

		std::wstring vertexShaderPath = JCommonUtility::U8StringToWstring(JApplicationVariable::GetShaderPath()) + L"VertexJShader.hlsl";
		std::wstring pixelShaderPath = JCommonUtility::U8StringToWstring(JApplicationVariable::GetShaderPath()) + L"PixelJShader.hlsl";
		for (uint j = 0; j < SHADER_VERTEX_COUNT; ++j)
		{
			shader->shaderData[j]->Vs = JD3DUtility::CompileShader(vertexShaderPath, &macroVec[j][0], "VS", "vs_5_1");
			shader->shaderData[j]->Ps = JD3DUtility::CompileShader(pixelShaderPath, &macroVec[j][0], "PS", "ps_5_1");
			shader->shaderData[j]->InputLayout.clear();
			GetInputLayout(shader->shaderData[j]->InputLayout, (J_SHADER_VERTEX_LAYOUT)(SHADER_VERTEX_LAYOUT_STATIC + j));
			JGraphic::Instance().ResourceInterface()->StuffShaderPso(shader->shaderData[j].get(), (J_SHADER_VERTEX_LAYOUT)(SHADER_VERTEX_LAYOUT_STATIC + j), shader->GetShaderFunctionFlag());
		}
	}
	void JShader::GetMacroVec(_Out_ std::vector<D3D_SHADER_MACRO>& outMacro, const J_SHADER_VERTEX_LAYOUT vertexLayoutFlag, const J_SHADER_FUNCTION functionFlag)noexcept
	{
		outMacro.push_back(vertexLayoutMacroMap.find(vertexLayoutFlag)->second);
		for (const auto& data : shaderFuncMacroMap)
		{
			if ((data.first & functionFlag) > 0)
				outMacro.push_back(data.second);
		}
		outMacro.push_back(shaderFuncMacroMap.find(SHADER_FUNCTION_NONE)->second);
	}
	void JShader::GetInputLayout(_Out_ std::vector<D3D12_INPUT_ELEMENT_DESC>& outInputLayout, const J_SHADER_VERTEX_LAYOUT vertexLayoutFlag)noexcept
	{
		outInputLayout = inputLayout.find(vertexLayoutFlag)->second;
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

		stream.open(shader->GetWPath(), std::ios::out | std::ios::binary);
		if (stream.is_open())
		{
			stream << (int)shader->functionFlag;
			stream.close();
			return Core::J_FILE_IO_RESULT::SUCCESS;
		}
		else
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
	}
	JShader* JShader::LoadObject(JDirectory* directory, const JResourcePathData& pathData)
	{
		if (directory == nullptr)
			return nullptr;
		 
		if (!JResourceObject::IsResourceFormat<JShader>(pathData.format))
			return nullptr;

		std::wifstream stream;
		stream.open(ConvertMetafilePath(pathData.wstrPath), std::ios::in | std::ios::binary);
		ObjectMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);
		stream.close();

		JShader* newShader;
		if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
			newShader = new JShader(pathData.name, metadata.guid, metadata.flag, directory, GetFormatIndex<JShader>(pathData.format));
		else
			newShader = new JShader(pathData.name, Core::MakeGuid(), OBJECT_FLAG_UNEDITABLE, directory, GetFormatIndex<JShader>(pathData.format));

		stream.open(pathData.wstrPath, std::ios::in | std::ios::binary);
		if (stream.is_open())
		{
			int functionFlag;
			stream >> functionFlag;
			newShader->functionFlag = (J_SHADER_FUNCTION)functionFlag;
			CompileShdaer(newShader);
			stream.close();
			return newShader;
		}
		else
			return nullptr;
	}
	void JShader::RegisterFunc()
	{
		auto defaultC = [](JDirectory* owner) ->JResourceObject*
		{
			return new JShader(owner->MakeUniqueFileName(GetDefaultName<JShader>()),
				Core::MakeGuid(),
				OBJECT_FLAG_UNEDITABLE,
				owner,
				JResourceObject::GetDefaultFormatIndex());
		};
		auto initC = [](const std::string& name, const size_t guid, const JOBJECT_FLAG objFlag, JDirectory* directory, const uint8 formatIndex)-> JResourceObject*
		{
			return  new JShader(name, guid, objFlag, directory, formatIndex);
		};
		auto loadC = [](JDirectory* directory, const JResourcePathData& pathData)-> JResourceObject*
		{
			return LoadObject(directory, pathData);
		};
		auto copyC = [](JResourceObject* ori)->JResourceObject*
		{
			return static_cast<JShader*>(ori)->CopyResource();
		};
		JRFI<JShader>::Register(defaultC, initC, loadC, copyC, &JShader::SetShaderFunctionFlag);

		auto getFormatIndexLam = [](const std::string& format) {return JResourceObject::GetFormatIndex<JShader>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JShader::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JShader::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

		RegisterTypeInfo(RTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, true },
			RTypeUtil{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable });
	}
	JShader::JShader(const std::string& name, const size_t guid, const JOBJECT_FLAG objFlag, JDirectory* directory, uint8 formatIndex)
		: JResourceObject(name, guid, objFlag, directory, formatIndex)
	{}
	JShader::~JShader(){}
}

/*
*
* 	void JShader::SetFunctionFlag(const J_SHADER_FUNCTION functionFlag)
	{
		JShader::functionFlag = functionFlag;
		for (uint i = 0; i < vertexLayoutCount; ++i)
			shaderData[i] = std::make_unique<JShaderData>();
		CompileShdaer(this);
	}
void JShader::SetShaderFunctionFlag(const J_SHADER_FUNCTION newFunction)noexcept
	{
		const J_SHADER_FUNCTION value = (J_SHADER_FUNCTION)(newFunction ^ (newFunction & functionFlag));
		if (value != SHADER_FUNCTION_NONE)
		{
			functionFlag = (J_SHADER_FUNCTION)(functionFlag | value);
			ShaderFileCreator::ReCompileShdaer(this);
		}
	}
	void JShader::OffShaderFunctionFlag(const J_SHADER_FUNCTION oldFunction)noexcept
	{
		const J_SHADER_FUNCTION value = (J_SHADER_FUNCTION)(oldFunction & functionFlag);
		if (value != SHADER_FUNCTION_NONE)
		{
			functionFlag = (J_SHADER_FUNCTION)(functionFlag ^ value);
			ShaderFileCreator::ReCompileShdaer(this);
		}
	}
	void JShader::SetShaderVertexLayoutFlag(const J_SHADER_VERTEX_LAYOUT newLayout)noexcept
	{
		vertexLayoutFlag = newLayout;
		ShaderFileCreator::ReCompileShdaer(this);
	}
*/