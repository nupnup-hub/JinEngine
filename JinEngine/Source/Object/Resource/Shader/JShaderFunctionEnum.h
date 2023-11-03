#pragma once
#include"../../../Core/Geometry/Mesh/JMeshType.h"
#include"../../../Application/JApplicationEngine.h"
#include<string>

namespace JinEngine
{
	enum J_SHADER_VERTEX_LAYOUT
	{
		SHADER_VERTEX_LAYOUT_STATIC = 0,
		SHADER_VERTEX_LAYOUT_SKINNED,
		SHADER_VERTEX_COUNT,
	};

	//Vs Ps function
	enum J_GRAPHIC_SHADER_FUNCTION
	{
		SHADER_FUNCTION_NONE = 0,
		SHADER_FUNCTION_ALBEDO_MAP = 1 << 0,
		SHADER_FUNCTION_ALBEDO_MAP_ONLY = 1 << 1,
		SHADER_FUNCTION_NORMAL_MAP = 1 << 2,
		SHADER_FUNCTION_HEIGHT_MAP = 1 << 3,
		SHADER_FUNCTION_ROUGHNESS_MAP = 1 << 4,
		SHADER_FUNCTION_AMBIENT_OCCLUSION_MAP = 1 << 5,
		SHADER_FUNCTION_SHADOW = 1 << 6,
		SHADER_FUNCTION_LIGHT = 1 << 7,
		SHADER_FUNCTION_SKY = 1 << 8, 
		SHADER_FUNCTION_ALPHA_CLIP = 1 << 9, 
		SHADER_FUNCTION_DEBUG = 1 << 10
	};

	enum class J_GRAPHIC_SHADER_EXTRA_FUNCTION
	{
		NONE = -1,
		STENCIL_WRITE_ALWAYS = 0,
		COUNT,
	};

	//Cs Function
	enum class J_COMPUTE_SHADER_FUNCTION
	{
		NONE = 0,
		COUNT
	};

	class JShaderType
	{
	public:
		struct CompileInfo
		{
		public:
			std::wstring filePath;
			std::string functionName;
		public:
			CompileInfo(const std::wstring& fileName, const std::string& functionName)
				:filePath(JApplicationEngine::ShaderPath() + L"\\" + fileName), functionName(functionName) {}
		};
	public:
		static std::wstring ConvertToName(const J_GRAPHIC_SHADER_FUNCTION funcFlag, const size_t subPsoID)
		{
			return L"Gs" + std::to_wstring(funcFlag) + L"_" + std::to_wstring(subPsoID);
		}
		static std::wstring ConvertToName(const J_COMPUTE_SHADER_FUNCTION funcFlag)
		{
			return L"Cs" + std::to_wstring((int)funcFlag);
		}
		static J_SHADER_VERTEX_LAYOUT ConvertToVertexLayout(const Core::J_MESHGEOMETRY_TYPE type)
		{
			return type == Core::J_MESHGEOMETRY_TYPE::STATIC ? SHADER_VERTEX_LAYOUT_STATIC : SHADER_VERTEX_LAYOUT_SKINNED;
		}
	};
}