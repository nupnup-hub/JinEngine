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


#include"JDxShaderDataHolderInterface.h"
#include"../../../Core/Exception/JExceptionMacro.h"
#include"../../../Core/Utility/JCommonUtility.h"
#include"../../../Core/Log/JLogMacro.h"
#include"../../../Develop/Debug/JDevelopDebug.h"
#include <D3Dcompiler.h>   

namespace JinEngine::Graphic
{
    namespace Private
    {
        std::vector<DxcDefine> ToDxMacro(const std::vector<JMacroSet>& set)noexcept
        {
            const uint setCount = (uint)set.size();
            const uint macroCount = setCount;
            std::vector<DxcDefine> macro(macroCount);
            for (uint i = 0; i < setCount; ++i)
                macro[i] = { set[i].name.c_str(),  set[i].value.c_str() };
            //macro[setCount] = { NULL, NULL };
            return macro;
        }
        std::vector<D3D_SHADER_MACRO> ToD3dMacro(const std::vector<JMacroSet>& set)noexcept
        {
            const uint setCount = (uint)set.size();
            const uint macroCount = setCount;
            std::vector<D3D_SHADER_MACRO> macro(macroCount);
            for (uint i = 0; i < setCount; ++i)
                macro[i] = {JCUtil::WstrToU8Str(set[i].name).c_str(),  JCUtil::WstrToU8Str(set[i].value).c_str() };
            //macro[setCount] = { NULL, NULL };
            return macro;
        }
        static Microsoft::WRL::ComPtr<ID3DBlob> CompileShaderUseOldApi(const std::wstring& filename,
            const std::vector<JMacroSet>& macroSet,
            const std::string& entrypoint,
            const std::string& target)
        {
            UINT compileFlags = 0;
#if defined(_DEBUG)  
            compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
            Microsoft::WRL::ComPtr<ID3DBlob> byteCode = nullptr;
            Microsoft::WRL::ComPtr<ID3DBlob> errors;

            std::vector<D3D_SHADER_MACRO> macro = ToD3dMacro(macroSet);
            ThrowIfFailedHr(D3DCompileFromFile(filename.c_str(), macro.data(), D3D_COMPILE_STANDARD_FILE_INCLUDE,
                entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors));

            if (errors != nullptr)
                OutputDebugStringA((char*)errors->GetBufferPointer());

            return byteCode;
        }
        //https://github.com/microsoft/DirectXShaderCompiler/wiki/Using-dxc.exe-and-dxcompiler.dll
        static Microsoft::WRL::ComPtr<IDxcBlob> CompileShaderUseNewApi(const std::wstring& filename,
            const std::vector<JMacroSet>& macroSet,
            const std::wstring& entrypoint,
            const std::wstring& target)
        {
            Microsoft::WRL::ComPtr<IDxcUtils> pUtils;
            Microsoft::WRL::ComPtr<IDxcCompiler3> pCompiler;
            DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
            DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

            Microsoft::WRL::ComPtr<IDxcLibrary> library;
            DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));

            uint32_t codePage = CP_UTF8;
            Microsoft::WRL::ComPtr<IDxcBlobEncoding> sourceBlob;
            library->CreateBlobFromFile(filename.c_str(), &codePage, &sourceBlob);

            Microsoft::WRL::ComPtr<IDxcIncludeHandler> pIncludeHandler;
            pUtils->CreateDefaultIncludeHandler(&pIncludeHandler);
            
            //Microsoft::WRL::ComPtr<IDxcBlobEncoding> pSource = nullptr;
           // pUtils->LoadFile(filename.c_str(), nullptr, &pSource);

            DxcBuffer source;
            source.Ptr = sourceBlob->GetBufferPointer();
            source.Size = sourceBlob->GetBufferSize();
            source.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.
             
            std::vector<DxcDefine> macro = ToDxMacro(macroSet);  
            Microsoft::WRL::ComPtr<IDxcCompilerArgs> pArgs;
            ThrowIfFailedHr(pUtils->BuildArguments(filename.c_str(),
                entrypoint.c_str(),
                target.c_str(),
                nullptr, 0,
                macro.data(),
                macro.size(),
                &pArgs));
              
            Microsoft::WRL::ComPtr<IDxcOperationResult> pResult;
            HRESULT hr = pCompiler->Compile(&source, pArgs->GetArguments(), pArgs->GetCount(), pIncludeHandler.Get(), IID_PPV_ARGS(&pResult));
            if (SUCCEEDED(hr))
                pResult->GetStatus(&hr); 
            if (FAILED(hr))
            {
                if (pResult)
                {
                    Microsoft::WRL::ComPtr<IDxcBlobEncoding> errorsBlob;
                    hr = pResult->GetErrorBuffer(&errorsBlob);
                    if (SUCCEEDED(hr) && errorsBlob)
                    {
                        std::string err = (const char*)errorsBlob->GetBufferPointer();
                        if (Develop::JDevelopDebug::IsActivate())
                        { 
                            if (!Develop::JDevelopDebug::HasLogHandler("ShaderCompileError"))
                                Develop::JDevelopDebug::CreatePublicLogHandler("ShaderCompileError");

                            Develop::JDevelopDebug::PushLog("Shader compile error");
                            Develop::JDevelopDebug::PushLog("ShaderCompileError", L"File: " + filename);
                            Develop::JDevelopDebug::PushLog("ShaderCompileError", L"Entry: " + entrypoint);
                            Develop::JDevelopDebug::PushLog("ShaderCompileError", L"Target: " + target);
                            Develop::JDevelopDebug::PushLog("ShaderCompileError", "Contents \n" + err);
                            Develop::JDevelopDebug::Write();
                        }
                        else
                            J_LOG_PRINT_OUT("Shader compilation failed with errors", err); 
                    }
                } 
            }
            Microsoft::WRL::ComPtr<IDxcBlob> code;
            pResult->GetResult(&code);   
            return code;
        }
    }
    Microsoft::WRL::ComPtr<IDxcBlob> JDxShaderDataUtil::CompileShader(const std::wstring& filename,
        const std::wstring& entrypoint,
        const std::wstring& target)
    {
        return Private::CompileShaderUseNewApi(filename, std::vector<JMacroSet>{}, entrypoint, target);
    }
    Microsoft::WRL::ComPtr<IDxcBlob> JDxShaderDataUtil::CompileShader(const std::wstring& filename,
        const std::vector<JMacroSet>& macroSet,
        const std::wstring& entrypoint,
        const std::wstring& target)
    {
       // auto underLine = target.find_first_of(L"_");
        return Private::CompileShaderUseNewApi(filename, macroSet, entrypoint, target);
        //auto versionNumber = JCUtil::WstringToInt(target.substr(underLine + 1, 1));
        //if (versionNumber >= 6)
        //    return Private::CompileShaderUseNewApi(filename, macroSet, entrypoint, target);
        //else
        //    return Private::CompileShaderUseOldApi(filename, macroSet, JCUtil::WstrToU8Str(entrypoint), JCUtil::WstrToU8Str(target));
    }
    Microsoft::WRL::ComPtr<IDxcBlob> JDxShaderDataUtil::CompileShader(const JCompileInfo& info,
        const std::vector<JMacroSet>& macroSet,
        const std::wstring& target)
    {
        return CompileShader(info.filePath, macroSet, info.functionName, target);
    }
}