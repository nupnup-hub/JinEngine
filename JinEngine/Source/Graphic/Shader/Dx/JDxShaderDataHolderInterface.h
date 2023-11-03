#pragma once
#include"../JShaderDataHolder.h"
#include <wrl.h> 
#include <d3d12.h>
namespace JinEngine
{
	namespace Graphic
	{
		class JDxShaderDataUtil
		{
		public:
			static std::vector<D3D_SHADER_MACRO> ToD3d12Macro(const std::vector<JMacroSet>& set)noexcept;
			static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::wstring& filename,
				const D3D_SHADER_MACRO* defines,
				const std::string& entrypoint,
				const std::string& target);
		};
	}
}