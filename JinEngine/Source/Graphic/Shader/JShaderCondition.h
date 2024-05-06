/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once 
#include"../../Core/JCoreEssential.h" 

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