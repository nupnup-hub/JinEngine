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


#include"JShaderCondition.h" 

namespace JinEngine
{
	bool JGraphicShaderCondition::operator ==(const JGraphicShaderCondition& rhs)const noexcept
	{
		return Equal(rhs);
	}
	bool JGraphicShaderCondition::operator !=(const JGraphicShaderCondition& rhs)const noexcept
	{
		return !Equal(rhs);
	}
	bool JGraphicShaderCondition::Equal(const JGraphicShaderCondition& pso)const noexcept
	{
		return primitiveCondition == pso.primitiveCondition &&
			depthCompareCondition == pso.depthCompareCondition &&
			cullModeCondition == pso.cullModeCondition &&
			primitiveType == pso.primitiveType &&
			depthCompareFunc == pso.depthCompareFunc &&
			isCullModeNone == pso.isCullModeNone;
	}
	size_t JGraphicShaderCondition::UniqueID()const noexcept
	{
		return std::hash <std::wstring>{}(L"PrimitiveC:" + std::to_wstring((int)primitiveCondition) +
			L"DepthComparesionC:" + std::to_wstring((int)depthCompareCondition) +
			L"CullModeC:" + std::to_wstring((int)cullModeCondition) +
			L"Primitive:" + std::to_wstring((int)primitiveType) +
			L"DepthComparesion:" + std::to_wstring((int)depthCompareFunc) +
			L"CullMode:" + std::to_wstring(isCullModeNone));
	}
}