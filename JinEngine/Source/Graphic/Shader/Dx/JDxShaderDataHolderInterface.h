#pragma once
#include"../JShaderDataHolder.h"
#include <wrl.h> 
#include <d3d12.h>
#include <dxc/dxcapi.h>
namespace JinEngine
{
	namespace Graphic
	{
		class JDxShaderDataUtil
		{
		public: 
			static Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filename,
				const std::wstring& entrypoint,
				const std::wstring& target);
			static Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filename,
				const std::vector<JMacroSet>& macroSet,
				const std::wstring& entrypoint,
				const std::wstring& target);
		};
	}
}