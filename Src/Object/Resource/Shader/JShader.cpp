#include"JShader.h" 
#include"JShaderData.h" 
#include"../JResourceManager.h"
#include"../JResourceObjectFactory.h"
#include"../../Directory/JDirectory.h"
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
	std::wstring JShader::GetFormat()const noexcept
	{
		return GetAvailableFormat()[0];
	}
	std::vector<std::wstring> JShader::GetAvailableFormat()noexcept
	{
		static std::vector<std::wstring> format{ L".shader" };
		return format;
	}
	bool JShader::Copy(JObject* ori)
	{
		if (ori->HasFlag(OBJECT_FLAG_UNCOPYABLE) || ori->GetGuid() == GetGuid())
			return false;

		if (typeInfo.IsA(ori->GetTypeInfo()))
		{
			JShader* oriS = static_cast<JShader*>(ori);
			CopyRFile(*oriS, *this);
			functionFlag = oriS->functionFlag;
			CompileShdaer(this);
			return true;
		}
		else
			return false;
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
		if (functionFlag != newFunctionFlag || !HasShaderData())
		{
			functionFlag = newFunctionFlag;
			CompileShdaer(this);
		}
	}
	void JShader::CompileShdaer(JShader* shader)
	{
		if (shader == nullptr)
			return;

		std::vector<D3D_SHADER_MACRO> macroVec[SHADER_VERTEX_COUNT];
		for (uint j = 0; j < SHADER_VERTEX_COUNT; ++j)
			GetMacroVec(macroVec[j], (J_SHADER_VERTEX_LAYOUT)(SHADER_VERTEX_LAYOUT_STATIC + j), shader->GetShaderFunctionFlag());

		std::wstring vertexShaderPath = JApplicationVariable::GetShaderPath() + L"VertexJShader.hlsl";
		std::wstring pixelShaderPath = JApplicationVariable::GetShaderPath() + L"PixelJShader.hlsl";
		for (uint j = 0; j < SHADER_VERTEX_COUNT; ++j)
		{
			shader->shaderData[j] = std::make_unique<JShaderData>();
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
	bool JShader::HasShaderData()const noexcept
	{
		bool hasData = true;
		for (uint i = 0; i < (uint)SHADER_VERTEX_COUNT; ++i)
			hasData = hasData && (shaderData[i] != nullptr);
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

		stream.open(pathData.wstrPath, std::ios::in | std::ios::binary);
		if (stream.is_open())
		{
			int functionFlag;
			stream >> functionFlag;
			stream.close();

			JShader* newShader = nullptr;
			if (directory->HasFile(pathData.fullName))
				newShader = JResourceManager::Instance().GetResourceByPath<JShader>(pathData.wstrPath);

			if (newShader == nullptr)
			{
				if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
				{
					Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JShader>(pathData.name,
						metadata.guid,
						metadata.flag,
						directory,
						GetFormatIndex<JShader>(pathData.format));
					newShader = ownerPtr.Get();
					AddInstance(std::move(ownerPtr));
				}
				else
				{
					Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JShader>(pathData.name,
						Core::MakeGuid(),
						OBJECT_FLAG_UNEDITABLE,
						directory,
						GetFormatIndex<JShader>(pathData.format));
					newShader = ownerPtr.Get();
					AddInstance(std::move(ownerPtr));
				}
			}

			if (newShader->IsValid())
				return newShader;
			else
			{
				newShader->SetShaderFunctionFlag((J_SHADER_FUNCTION)functionFlag);
				return newShader;
			}
		}
		else
			return nullptr;
	}
	void JShader::RegisterJFunc()
	{
		auto defaultC = [](JDirectory* directory) ->JResourceObject*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JShader>(directory->MakeUniqueFileName(GetDefaultName<JShader>()),
				Core::MakeGuid(),
				OBJECT_FLAG_UNEDITABLE,
				directory,
				JResourceObject::GetDefaultFormatIndex());
			JResourceObject* ret = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			return ret;
		};
		auto initC = [](const std::wstring& name, const size_t guid, const J_OBJECT_FLAG objFlag, JDirectory* directory, const uint8 formatIndex)-> JResourceObject*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JShader>(name, guid, objFlag, directory, formatIndex);
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
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JShader>(directory->MakeUniqueFileName(ori->GetName()),
				Core::MakeGuid(),
				ori->GetFlag(),
				directory,
				GetFormatIndex<JShader>(ori->GetFormat()));

			JShader* newShader = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			newShader->Copy(ori);
			return newShader;
		};
		JRFI<JShader>::Register(defaultC, initC, loadC, copyC, &JShader::SetShaderFunctionFlag);

		auto getFormatIndexLam = [](const std::wstring& format) {return JResourceObject::GetFormatIndex<JShader>(format); };

		static GetTypeNameCallable getTypeNameCallable{ &JShader::TypeName };
		static GetAvailableFormatCallable getAvailableFormatCallable{ &JShader::GetAvailableFormat };
		static GetFormatIndexCallable getFormatIndexCallable{ getFormatIndexLam };

		static RTypeHint rTypeHint{ GetStaticResourceType(), std::vector<J_RESOURCE_TYPE>{}, true, false, false };
		static RTypeCommonFunc rTypeCFunc{ getTypeNameCallable, getAvailableFormatCallable, getFormatIndexCallable };

		RegisterTypeInfo(rTypeHint, rTypeCFunc, RTypeInterfaceFunc{});
	}
	JShader::JShader(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG objFlag, JDirectory* directory, uint8 formatIndex)
		: JResourceObject(name, guid, Core::AddTwoSquareValueEnum(objFlag, OBJECT_FLAG_UNCOPYABLE), directory, formatIndex)
	{

	}
	JShader::~JShader() {}
}