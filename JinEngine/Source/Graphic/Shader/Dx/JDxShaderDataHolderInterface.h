#pragma once
#include"../JShaderDataHolder.h"
namespace JinEngine
{
	namespace Graphic
	{
		class JDxShaderDataUtil
		{
		public:
			static std::vector<D3D_SHADER_MACRO> ToD3d12Macro(const std::vector<JMacroSet>& set)noexcept;
		};
	}
}