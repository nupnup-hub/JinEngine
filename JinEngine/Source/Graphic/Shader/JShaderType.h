#pragma once
#include"JShaderCondition.h"
#include"../../Core/JCoreEssential.h"
#include"../../Core/Math/JVector.h"
#include"../../Core/Platform/JHardwareInfo.h" 
#include"../../Core/Geometry/Mesh/JMeshType.h"
#include"../../Application/Engine/JApplicationEngine.h" 

namespace JinEngine
{
	enum class J_SHADER_TYPE
	{
		GRAPHIC,
		COMPUTE,
		RAY_TRACE_COMPUTE,
		COUNT
	};
	enum class J_GRAPHIC_SHADER_TYPE
	{
		STANDARD,
		PREVIEW,
		COUNT
	};
	enum class J_GRAPHIC_SHADER_VERTEX_LAYOUT
	{
		STATIC = 0,
		SKINNED,
		COUNT,
	};
	enum class J_GRAPHIC_RENDERING_PROCESS
	{
		FORWARD = 0,
		DEFERRED_GEOMETRY,
		DEFERRED_SHADING,
		COUNT
	};
	enum J_GRAPHIC_SHADER_FUNCTION
	{
		SHADER_FUNCTION_NONE = 0,
		SHADER_FUNCTION_ALBEDO_MAP = 1 << 0,
		SHADER_FUNCTION_ALBEDO_MAP_ONLY = 1 << 1,
		SHADER_FUNCTION_SPECULAR_MAP = 1 << 2,
		SHADER_FUNCTION_NORMAL_MAP = 1 << 3,
		SHADER_FUNCTION_HEIGHT_MAP = 1 << 4,
		SHADER_FUNCTION_METALLIC_MAP = 1 << 5,
		SHADER_FUNCTION_ROUGHNESS_MAP = 1 << 6,
		SHADER_FUNCTION_AMBIENT_OCCLUSION_MAP = 1 << 7,
		SHADER_FUNCTION_SHADOW = 1 << 8,
		SHADER_FUNCTION_LIGHT = 1 << 9,
		SHADER_FUNCTION_SKY = 1 << 10,
		SHADER_FUNCTION_ALPHA_CLIP = 1 << 11, 
		SHADER_FUNCTION_DEBUG = 1 << 12,
	};
	enum J_GRAPHIC_SHADER_PRIVATE_FUNCTION
	{
		SHADER_FUNCTION_PRIVATE_NONE = 0, 
		SHADER_FUNCTION_PRIVATE_LIGHT_CULLING = 1 << 0,
		SHADER_FUNCTION_PRIVATE_GLOBAL_ILLUMINATION = 1 << 1,
	};
	//material가 아닌 다른객체에 대한 종속성
	enum class J_GRAPHIC_SHADER_EXTRA_FUNCTION
	{
		NONE = 0,
		STENCIL_WRITE_ALWAYS = 1 << 0,		//outline  
		COUNT = (1 << 1),
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
		static std::wstring ConvertToName(const J_GRAPHIC_SHADER_FUNCTION funcFlag, const size_t subPsoID);
		static std::wstring ConvertToName(const J_COMPUTE_SHADER_FUNCTION funcFlag);
		static J_GRAPHIC_SHADER_VERTEX_LAYOUT ConvertToVertexLayout(const Core::J_MESHGEOMETRY_TYPE type);
	};
	struct JMacroSet
	{
	public:
		std::wstring name;
		std::wstring value;
	};
	struct JCompileInfo
	{
	public:
		std::wstring filePath;
		std::wstring functionName;
	public:
		JCompileInfo() = default;
		JCompileInfo(const std::wstring& fileName, const std::wstring& functionName);
	public:
		bool IsValid()const noexcept;
	};
	struct JDispatchInfo
	{
	public:
		JVector3<uint> groupDim = JVector3<uint>::Zero();
		JVector3<uint> threadDim = JVector3<uint>::Zero();
		//threadDim과 groupDim이 미리 정의되는 경우에만 유효 task에 따라서 threadDim만 미리 정의되고
		//groupDim은 동적으로 계산하는 경우가있음.. ex)light cluster, display size에 따른 task
		uint taskOriCount = 0;
	public:
		uint GetTotalThreadCount()const noexcept;
	};
	struct JGraphicShaderInitData
	{ 
	public:
		std::vector<JMacroSet> macro[(uint)J_GRAPHIC_SHADER_VERTEX_LAYOUT::COUNT];
		J_GRAPHIC_SHADER_TYPE shaderType;
		J_GRAPHIC_SHADER_FUNCTION gFunctionFlag;
		J_GRAPHIC_SHADER_VERTEX_LAYOUT layoutType;
		J_GRAPHIC_RENDERING_PROCESS processType;
		JGraphicShaderCondition condition; 
	public:
		J_GRAPHIC_SHADER_PRIVATE_FUNCTION privateFlag;
	};
	struct JComputeShaderInitData
	{
	public:
		std::vector<JMacroSet> macro;
		J_COMPUTE_SHADER_FUNCTION cFunctionFlag;
		JDispatchInfo dispatchInfo;
	public:
		//caution! 
		//dispatchInfo.threadDim에 유효한 값이 할당되어 있어야한다. 
		void PushThreadDimensionMacro();
		void CalThreadAndGroupDim(const Core::JHardwareInfo::GpuInfo& info, const uint width, const uint height);
	};
}