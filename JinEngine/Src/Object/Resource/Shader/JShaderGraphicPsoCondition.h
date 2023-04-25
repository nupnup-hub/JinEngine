#pragma once
#include<d3d12.h> 

namespace JinEngine
{ 
	//shader pso 적용 여부
	enum class J_SHADER_PSO_APPLIY_CONDITION
	{ 
		SAME_AS = 0,
		APPLY_J_PSO,
	};
	enum class J_SHADER_PRIMITIVE_TYPE
	{
		DEFAULT = 0,
		LINE
	};
	enum class J_SHADER_DEPTH_COMPARISON_FUNC
	{ 
		DEFAULT = 0,
		NEVER = 1,
		LESS = 2,
		EQUAL = 3,
		LESS_EQUAL = 4,
		GREATER = 5,
		NOT_EQUAL = 6,
		GREATER_EQUAL = 7,
		ALWAYS = 8
	};

	struct JShaderGraphicPsoCondition
	{
	public:
		J_SHADER_PSO_APPLIY_CONDITION primitiveCondition = J_SHADER_PSO_APPLIY_CONDITION::SAME_AS;
		J_SHADER_PSO_APPLIY_CONDITION depthCompareCondition = J_SHADER_PSO_APPLIY_CONDITION::SAME_AS;
		J_SHADER_PSO_APPLIY_CONDITION cullModeCondition = J_SHADER_PSO_APPLIY_CONDITION::SAME_AS;
	public:
		J_SHADER_PRIMITIVE_TYPE primitiveType = J_SHADER_PRIMITIVE_TYPE::DEFAULT;
		J_SHADER_DEPTH_COMPARISON_FUNC depthCompareFunc = J_SHADER_DEPTH_COMPARISON_FUNC::DEFAULT;
		bool isCullModeNone = false;
	public:
		bool operator ==(const JShaderGraphicPsoCondition& rhs)const noexcept;
		bool operator !=(const JShaderGraphicPsoCondition& rhs)const noexcept;
	public: 
		bool Equal(const JShaderGraphicPsoCondition& pso)const noexcept;
		size_t UniqueID()const noexcept;
	public:
		D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertD3d12PrimitiveType()const noexcept;
		D3D12_COMPARISON_FUNC ConvertD3d12Comparesion()const noexcept; 
		D3D12_CULL_MODE ConvertD3d12CullMode()const noexcept;
	};
	

}