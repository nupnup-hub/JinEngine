#pragma once
#include<wrl/client.h>   

struct ID3D12RootSignature;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;

namespace JinEngine
{
	namespace Graphic
	{
		class JOutline
		{ 
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature; 
		};
	}
}