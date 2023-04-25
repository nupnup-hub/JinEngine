#include"JShaderGraphicPsoCondition.h"
#include<string>

namespace JinEngine
{
	bool JShaderGraphicPsoCondition::operator ==(const JShaderGraphicPsoCondition& rhs)const noexcept
	{
		return Equal(rhs);
	}
	bool JShaderGraphicPsoCondition::operator !=(const JShaderGraphicPsoCondition& rhs)const noexcept
	{
		return !Equal(rhs);
	}
	bool JShaderGraphicPsoCondition::Equal(const JShaderGraphicPsoCondition& pso)const noexcept
	{
		return primitiveCondition == pso.primitiveCondition &&
			depthCompareCondition == pso.depthCompareCondition &&
			cullModeCondition == pso.cullModeCondition &&
			primitiveType == pso.primitiveType &&
			depthCompareFunc == pso.depthCompareFunc &&
			isCullModeNone == pso.isCullModeNone;
	}
	size_t JShaderGraphicPsoCondition::UniqueID()const noexcept
	{
		return std::hash <std::wstring>{}(L"PrimitiveC:" + std::to_wstring((int)primitiveCondition) +
			L"DepthComparesionC:" + std::to_wstring((int)depthCompareCondition) +
			L"CullModeC:" + std::to_wstring((int)cullModeCondition) +
			L"Primitive:" + std::to_wstring((int)primitiveType) +
			L"DepthComparesion:" + std::to_wstring((int)depthCompareFunc) +
			L"CullMode:" + std::to_wstring(isCullModeNone));
	}
	D3D12_PRIMITIVE_TOPOLOGY_TYPE JShaderGraphicPsoCondition::ConvertD3d12PrimitiveType()const noexcept
	{
		switch (primitiveType)
		{
		case JinEngine::J_SHADER_PRIMITIVE_TYPE::DEFAULT:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	// defualt value
		case JinEngine::J_SHADER_PRIMITIVE_TYPE::LINE:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		default:
			break;
		}
	}
	D3D12_COMPARISON_FUNC JShaderGraphicPsoCondition::ConvertD3d12Comparesion()const noexcept
	{
		switch (depthCompareFunc)
		{
		case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::DEFAULT:
			return D3D12_COMPARISON_FUNC_LESS;	// defualt value
		case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::NEVER:
			return D3D12_COMPARISON_FUNC_NEVER;
		case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::LESS:
			return D3D12_COMPARISON_FUNC_LESS;
		case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::EQUAL:
			return D3D12_COMPARISON_FUNC_EQUAL;
		case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::LESS_EQUAL:
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::GREATER:
			return D3D12_COMPARISON_FUNC_GREATER;
		case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::NOT_EQUAL:
			return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::GREATER_EQUAL:
			return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case JinEngine::J_SHADER_DEPTH_COMPARISON_FUNC::ALWAYS:
			return D3D12_COMPARISON_FUNC_ALWAYS;
		default:
			break;
		}
	} 
	D3D12_CULL_MODE JShaderGraphicPsoCondition::ConvertD3d12CullMode()const noexcept
	{
		if (isCullModeNone)
			return D3D12_CULL_MODE_NONE;
		else
			return D3D12_CULL_MODE_BACK;	//default value
	}
}