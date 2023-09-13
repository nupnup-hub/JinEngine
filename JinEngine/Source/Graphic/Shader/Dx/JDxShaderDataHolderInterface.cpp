#include"JDxShaderDataHolderInterface.h"

namespace JinEngine::Graphic
{
	std::vector<D3D_SHADER_MACRO> JDxShaderDataUtil::ToD3d12Macro(const std::vector<JMacroSet>& set)noexcept
	{
		const uint setCount = (uint)set.size();
		const uint macroCount = setCount + 1;
		std::vector<D3D_SHADER_MACRO> macro(macroCount);
		for (uint i = 0; i < setCount; ++i)
			macro[i] = { set[i].name.c_str(),  set[i].value.c_str() };
		macro[setCount] = { NULL, NULL };
		return macro;
	}
}