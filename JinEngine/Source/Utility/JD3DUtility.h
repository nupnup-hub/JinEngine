#pragma once
//***************************************************************************************
// d3dUtil.h by Frank Luna (C) 2015 All Rights Reserved.
//
// General helper code.
//***************************************************************************************
#pragma once 
#include <wrl.h> 
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h> 
#include <DirectXCollision.h>
#include <string>
#include <memory>   
#include <DirectXMath.h> 
#include"../Core/JDataType.h"
#include "../../ThirdParty/DirectX/Tk/Src/d3dx12.h" 
 
using Microsoft::WRL::ComPtr;

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif

namespace JinEngine
{  
	class JD3DUtility
	{
	public:
		static bool IsKeyDown(const int vkeyCode);	 
		static uint CalcConstantBufferByteSize(const uint byteSize);
		static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);
		static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* device,
			ID3D12GraphicsCommandList* cmdList,
			const void* initData,
			uint64 byteSize,
			Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);
		static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::wstring& filename,
			const D3D_SHADER_MACRO* defines,
			const std::string& entrypoint,
			const std::string& target);
	};
}