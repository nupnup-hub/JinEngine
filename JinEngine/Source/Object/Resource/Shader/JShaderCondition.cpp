#include"JShaderCondition.h"
#include<string>

namespace JinEngine
{
	bool JShaderCondition::operator ==(const JShaderCondition& rhs)const noexcept
	{
		return Equal(rhs);
	}
	bool JShaderCondition::operator !=(const JShaderCondition& rhs)const noexcept
	{
		return !Equal(rhs);
	}
	bool JShaderCondition::Equal(const JShaderCondition& pso)const noexcept
	{
		return primitiveCondition == pso.primitiveCondition &&
			depthCompareCondition == pso.depthCompareCondition &&
			cullModeCondition == pso.cullModeCondition &&
			primitiveType == pso.primitiveType &&
			depthCompareFunc == pso.depthCompareFunc &&
			isCullModeNone == pso.isCullModeNone;
	}
	size_t JShaderCondition::UniqueID()const noexcept
	{
		return std::hash <std::wstring>{}(L"PrimitiveC:" + std::to_wstring((int)primitiveCondition) +
			L"DepthComparesionC:" + std::to_wstring((int)depthCompareCondition) +
			L"CullModeC:" + std::to_wstring((int)cullModeCondition) +
			L"Primitive:" + std::to_wstring((int)primitiveType) +
			L"DepthComparesion:" + std::to_wstring((int)depthCompareFunc) +
			L"CullMode:" + std::to_wstring(isCullModeNone));
	}
}