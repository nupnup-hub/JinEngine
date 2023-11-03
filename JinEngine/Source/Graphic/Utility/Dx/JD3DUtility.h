#pragma once
//***************************************************************************************
// d3dUtil.h by Frank Luna (C) 2015 All Rights Reserved.
//
// General helper code.
//***************************************************************************************
#pragma once 
#include <wrl.h>  
#include <string>
#include <memory>    
#include"../../../Core/JCoreEssential.h" 
#include"../../../Object/Component/RenderItem/JRenderItemPrimitive.h"
#include "../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"   

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif

namespace JinEngine
{  
	class JD3DUtility
	{
	public: 	 
		static uint CalcConstantBufferByteSize(const uint byteSize)noexcept;
		static D3D12_PRIMITIVE_TOPOLOGY ConvertRenderPrimitive(const J_RENDER_PRIMITIVE renderPrimitive)noexcept;
		static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename); 
		static void ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
	};
}