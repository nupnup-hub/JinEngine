#include"JDxShaderDataHolderInterface.h"
#include"../../../Core/Exception/JExceptionMacro.h"
#include <D3Dcompiler.h>
 
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
    Microsoft::WRL::ComPtr<ID3DBlob> JDxShaderDataUtil::CompileShader(
        const std::wstring& filename,
        const D3D_SHADER_MACRO* defines,
        const std::string& entrypoint,
        const std::string& target)
    {
        UINT compileFlags = 0;
#if defined(_DEBUG)  
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        Microsoft::WRL::ComPtr<ID3DBlob> byteCode = nullptr;
        Microsoft::WRL::ComPtr<ID3DBlob> errors;

        ThrowIfFailedHr(D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors));

        if (errors != nullptr)
            OutputDebugStringA((char*)errors->GetBufferPointer());

        return byteCode;
    }
}