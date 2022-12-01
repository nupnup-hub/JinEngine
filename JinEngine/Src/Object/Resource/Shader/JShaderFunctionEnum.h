#pragma once
#include"../Mesh/JMeshType.h"
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
		SHADER_FUNCTION_NONCULLING = 1 << 9,
		SHADER_FUNCTION_ALPHA_CLIP = 1 << 10, 
		SHADER_FUNCTION_WRITE_SHADOWMAP = 1 << 11,
		SHADER_FUNCTION_DEPTH_TEST_BOUNDING_OBJECT = 1 << 12,
		SHADER_FUNCTION_DEBUG =  1 << 13, 
	}; 

	//Cs Function
	enum class J_COMPUTE_SHADER_FUNCTION
	{
		NONE = 0,
		HZB_COPY,
		HZB_DOWN_SAMPLING,
		HZB_OCCLUSION,
		COUNT,
	};

	class JShaderType
	{
	public:
		struct CompileInfo
		{
		public:
			std::wstring fileName;
			std::string functionName;
		public:
			CompileInfo(const std::wstring& fileName, const std::string& functionName)
				:fileName(fileName), functionName(functionName)
			{}
		};
	public:
		static std::wstring ConvertToName(const J_GRAPHIC_SHADER_FUNCTION funcFlag)
		{
			return L"Gs" + std::to_wstring(funcFlag);
		}
		static std::wstring ConvertToName(const J_COMPUTE_SHADER_FUNCTION funcFlag)
		{
			return L"Cs" + std::to_wstring((int)funcFlag);
		}
		static CompileInfo ComputeShaderCompileInfo(const J_COMPUTE_SHADER_FUNCTION funcFlag)
		{
			switch (funcFlag)
			{
			case JinEngine::J_COMPUTE_SHADER_FUNCTION::HZB_COPY:
				return CompileInfo(L"Hierarchical z-buffer.hlsl", "HZBCopyDepthMap");
			case JinEngine::J_COMPUTE_SHADER_FUNCTION::HZB_DOWN_SAMPLING:
				return CompileInfo(L"Hierarchical z-buffer.hlsl", "HZBDownSampling");
			case JinEngine::J_COMPUTE_SHADER_FUNCTION::HZB_OCCLUSION:
				return CompileInfo(L"Hierarchical z-buffer.hlsl", "HZBOcclusion");
			default:
				return CompileInfo(L"Error", "Error");
			}
		}
		static J_SHADER_VERTEX_LAYOUT ConvertToVertexLayout(const J_MESHGEOMETRY_TYPE type)
		{
			return type == J_MESHGEOMETRY_TYPE::STATIC ? SHADER_VERTEX_LAYOUT_STATIC : SHADER_VERTEX_LAYOUT_SKINNED;
		}
	};
}