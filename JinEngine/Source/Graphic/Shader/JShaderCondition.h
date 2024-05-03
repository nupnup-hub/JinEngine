#pragma once 
#include"../../Core/JCoreEssential.h"
#include<string>

namespace JinEngine
{
	enum class J_SHADER_APPLIY_CONDITION
	{
		NOT = 0,
		APPLY,
	};
	enum class J_SHADER_PRIMITIVE_TYPE
	{
		TRIANGLE = 0,	//triangle
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
	struct JGraphicShaderCondition
	{
	public:
		J_SHADER_APPLIY_CONDITION primitiveCondition = J_SHADER_APPLIY_CONDITION::NOT;
		J_SHADER_APPLIY_CONDITION depthCompareCondition = J_SHADER_APPLIY_CONDITION::NOT;
		J_SHADER_APPLIY_CONDITION cullModeCondition = J_SHADER_APPLIY_CONDITION::NOT;
	public:
		J_SHADER_PRIMITIVE_TYPE primitiveType = J_SHADER_PRIMITIVE_TYPE::TRIANGLE;
		J_SHADER_DEPTH_COMPARISON_FUNC depthCompareFunc = J_SHADER_DEPTH_COMPARISON_FUNC::DEFAULT;
		bool isCullModeNone = false;
	public:
		bool operator ==(const JGraphicShaderCondition& rhs)const noexcept;
		bool operator !=(const JGraphicShaderCondition& rhs)const noexcept;
	public:
		bool Equal(const JGraphicShaderCondition& pso)const noexcept;
		size_t UniqueID()const noexcept;
	};
}