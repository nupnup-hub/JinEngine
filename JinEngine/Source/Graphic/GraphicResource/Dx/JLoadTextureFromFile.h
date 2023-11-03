#pragma once 
#include<d3d12.h> 
#include<assert.h>
#include <wrl.h>
#include<string>
using Microsoft::WRL::ComPtr;

namespace JinEngine
{
	bool LoadTextureFromFile(const std::wstring& path,
		const std::wstring& format,
		ID3D12Device* device,
		D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle,
		ComPtr<ID3D12Resource>& out_tex_resource,
		ComPtr<ID3D12Resource>& uploadBuffer,
		const int maxSize);
}